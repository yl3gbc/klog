#include "graylinewidget.h"
#include <QPainter>
#include <QtMath>
#include <QDateTime>
#include <QDir>
#include "world.h"
#include "dataproxy_sqlite.h"

GrayLineWidget::GrayLineWidget(QWidget *parent) : QWidget(parent)
{
    setFixedSize(425, 205);
    worldMap.load(QStringLiteral(":/img/YL3GBC.png"));
    if (worldMap.isNull())
        worldMap.load(QDir::homePath() + QStringLiteral("/klogng/src/img/YL3GBC.png"));
    connect(&timer, &QTimer::timeout, this, QOverload<>::of(&QWidget::update));
    timer.start(60000);   // terminators parzimejas reizi minute
}

void GrayLineWidget::setMyLocator(const QString &grid)
{
    myGrid = grid.trimmed().toUpper();
    myPos = gridToXY(myGrid);
    update();
}

// Ja korespondents lokatoru nedod, nemam DXCC entitates koordinatas
void GrayLineWidget::setDxCallsign(const QString &call)
{
    if (!dxGrid.isEmpty()) return;          // lokators ir - to nemainam
    if (!world || call.trimmed().length() < 3) { dxPos = QPointF(-1, -1); update(); return; }
    const int enti = world->getQRZARRLId(call.trimmed().toUpper());
    if (enti <= 0) { dxPos = QPointF(-1, -1); update(); return; }
    const double lon = world->getLongitude(enti);
    const double lat = world->getLatitude(enti);
    dxPos = (qFuzzyIsNull(lon) && qFuzzyIsNull(lat)) ? QPointF(-1, -1) : lonLatToXY(lon, lat);
    update();
}

void GrayLineWidget::setDxLocator(const QString &grid)
{
    dxGrid = grid.trimmed().toUpper();
    dxPos = (dxGrid.length() >= 4) ? gridToXY(dxGrid) : QPointF(-1, -1);
    update();
}

// Maidenhead -> ekrana koordinatas
QPointF GrayLineWidget::gridToXY(const QString &grid) const
{
    if (grid.length() < 4) return QPointF(-1, -1);
    const QString g = grid.toUpper();
    double lon = (g.at(0).toLatin1() - 'A') * 20.0 - 180.0;
    double lat = (g.at(1).toLatin1() - 'A') * 10.0 - 90.0;
    lon += (g.at(2).toLatin1() - '0') * 2.0;
    lat += (g.at(3).toLatin1() - '0') * 1.0;
    if (grid.length() >= 6)
    {
        lon += (g.at(4).toLatin1() - 'A') * (2.0 / 24.0) + (1.0 / 24.0);
        lat += (g.at(5).toLatin1() - 'A') * (1.0 / 24.0) + (0.5 / 24.0);
    }
    else { lon += 1.0; lat += 0.5; }
    return lonLatToXY(lon, lat);
}

// Karte aizpilda visu logu; ja logs ir augstaks neka 2:1, polaros apgabalus
// apgriez (tur tapat maz notiek), proporcijas paliek pareizas.
QRect GrayLineWidget::mapRect() const
{
    int w = width();
    int h = w / 2;
    if (h < height()) { h = height(); w = h * 2; }
    return QRect((width() - w) / 2, (height() - h) / 2, w, h);
}

QPointF GrayLineWidget::lonLatToXY(double lon, double lat) const
{
    const QRect r = mapRect();
    const double x = r.left() + (lon + 180.0) / 360.0 * r.width();
    const double y = r.top() + (90.0 - lat) / 180.0 * r.height();
    return QPointF(x, y);
}

// Saules deklinacija un terminators konkretajam laikam
void GrayLineWidget::drawTerminator(QPainter &p)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const int doy = now.date().dayOfYear();
    const double hour = now.time().hour() + now.time().minute() / 60.0;
    const double decl = 23.44 * qSin(qDegreesToRadians(360.0 / 365.24 * (doy - 81)));
    const double sunLon = 180.0 - hour * 15.0;

    const QRect r = mapRect();
    const double sd = qSin(qDegreesToRadians(decl));
    const double cd = qCos(qDegreesToRadians(decl));

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(10, 15, 50, 120));

    for (int px = r.left(); px <= r.right(); ++px)
    {
        const double lon = (px - r.left()) / (double)r.width() * 360.0 - 180.0;
        const double cha = qCos(qDegreesToRadians(lon - sunLon));
        int yStart = -1;
        for (int py = r.top(); py <= r.bottom(); ++py)
        {
            const double lat = 90.0 - (py - r.top()) / (double)r.height() * 180.0;
            const double alt = qSin(qDegreesToRadians(lat)) * sd
                             + qCos(qDegreesToRadians(lat)) * cd * cha;
            const bool night = (alt < 0.0);
            if (night && yStart < 0) yStart = py;
            if ((!night || py == r.bottom()) && yStart >= 0)
            {
                p.drawRect(px, yStart, 1, py - yStart + 1);
                yStart = -1;
            }
        }
    }
}

void GrayLineWidget::drawWorld(QPainter &p)
{
    if (!worldMap.isNull())
    {
        if (scaled.size() != mapRect().size())
            scaled = worldMap.scaled(mapRect().size(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        p.fillRect(rect(), QColor(0x20, 0x28, 0x38));
        p.drawPixmap(mapRect().topLeft(), scaled);
    }
    else
        p.fillRect(rect(), QColor(0xdd, 0xee, 0xff));

    p.setPen(QPen(QColor(255, 255, 255, 40), 1));
    for (int lon = -180; lon <= 180; lon += 30)
    {
        const double x = (lon + 180.0) / 360.0 * width();
        p.drawLine(QPointF(x, 0), QPointF(x, height()));
    }
    for (int lat = -60; lat <= 60; lat += 30)
    {
        const double y = (90.0 - lat) / 180.0 * height();
        p.drawLine(QPointF(0, y), QPointF(width(), y));
    }
}

void GrayLineWidget::drawSun(QPainter &p)
{
    const QDateTime now = QDateTime::currentDateTimeUtc();
    const int doy = now.date().dayOfYear();
    const double hour = now.time().hour() + now.time().minute() / 60.0;
    const double decl = 23.44 * qSin(qDegreesToRadians(360.0 / 365.24 * (doy - 81)));
    const double sunLon = 180.0 - hour * 15.0;
    const QPointF sp = lonLatToXY(sunLon, decl);

    p.setPen(Qt::NoPen);
    p.setBrush(QColor(255, 235, 120, 90));
    p.drawEllipse(sp, 11, 11);
    p.setBrush(QColor(255, 210, 40));
    p.setPen(QPen(QColor(200, 150, 0), 1));
    p.drawEllipse(sp, 5, 5);
}

void GrayLineWidget::resizeEvent(QResizeEvent *)
{
    if (!myGrid.isEmpty()) myPos = gridToXY(myGrid);
    update();
}

void GrayLineWidget::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    drawWorld(p);
    drawTerminator(p);
    drawSun(p);

    // QSO linijas
    if (myPos.x() >= 0)
    {
        p.setPen(QPen(QColor(0xff, 0x88, 0x00, 150), 1));
        for (const QPointF &q : qsoPoints)
            if (q.x() >= 0) p.drawLine(myPos, q);

        p.setBrush(QColor(0x33, 0x77, 0xcc));
        p.setPen(QPen(Qt::white, 1));
        for (const QPointF &q : qsoPoints)
            if (q.x() >= 0) p.drawEllipse(q, 2.5, 2.5);

        // tekosais QSO: linija uz korespondentu
        if (dxPos.x() >= 0)
        {
            p.setPen(QPen(QColor(255, 60, 60, 220), 2));
            p.drawLine(myPos, dxPos);
            p.setBrush(QColor(255, 240, 80));
            p.setPen(QPen(QColor(120, 60, 0), 1.5));
            p.drawEllipse(dxPos, 4, 4);
        }

        // musu pozicija
        p.setBrush(QColor(0xcc, 0x22, 0x22));
        p.setPen(QPen(Qt::white, 1.5));
        p.drawEllipse(myPos, 4, 4);
    }
}

void GrayLineWidget::refreshQSOs()
{
    qsoPoints.clear();
    update();
}
