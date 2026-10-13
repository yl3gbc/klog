#include "contestpanel.h"
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>

ContestPanel::ContestPanel(QWidget *parent) : QWidget(parent)
{
    roundLabel = new QLabel(tr("No contest"), this);
    roundLabel->setAlignment(Qt::AlignCenter);
    roundLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));

    scoreLabel = new QLabel(QString(), this);
    scoreLabel->setAlignment(Qt::AlignCenter);

    squaresLabel = new QLabel(this);
    squaresLabel->setWordWrap(true);
    squaresLabel->setAlignment(Qt::AlignTop | Qt::AlignLeft);
    squaresLabel->hide();

    grid = new QGridLayout;
    grid->setSpacing(2);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(roundLabel);
    lay->addLayout(grid, 1);
    lay->addWidget(squaresLabel, 1);
    matrixWidget = new QWidget(this);
    new QGridLayout(matrixWidget);
    matrixWidget->hide();
    lay->addWidget(matrixWidget, 1);
    lay->addWidget(scoreLabel);
}

void ContestPanel::loadDistricts(const QString &path)
{
    zoneMode = false;
    if (matrixWidget) matrixWidget->hide();
    order.clear();
    qDeleteAll(cells);
    cells.clear();

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);
    while (!in.atEnd())
    {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        const QString code = line.section(QLatin1Char(';'), 0, 0).trimmed().toUpper();
        const QString name = line.section(QLatin1Char(';'), 1, 1).trimmed();
        if (code.isEmpty()) continue;
        order << code;
        QLabel *c = new QLabel(code, this);
        c->setAlignment(Qt::AlignCenter);
        c->setToolTip(name);
        c->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        c->setMinimumWidth(34);
        cells.insert(code, c);
    }
    f.close();

    int row = 0, col = 0;
    for (const QString &code : order)
    {
        grid->addWidget(cells.value(code), row, col);
        if (++col >= 8) { col = 0; ++row; }
    }
    setWorkedInRound(QSet<QString>());
}

void ContestPanel::setRound(int n, const QString &timeLeft)
{
    if (!roundLabel) return;
    if (n <= 0)
        roundLabel->setText(tr("Outside contest hours"));
    else
        roundLabel->setText(tr("Round %1 — %2 left").arg(n).arg(timeLeft));
}

void ContestPanel::setWorkedInRound(const QSet<QString> &codes)
{
    for (auto it = cells.cbegin(); it != cells.cend(); ++it)
    {
        const bool done = codes.contains(it.key());
        it.value()->setStyleSheet(done
            ? QStringLiteral("background:#2e7d32; color:white;")
            : QStringLiteral("background:#e0e0e0; color:#555;"));
    }
}

void ContestPanel::setScore(int qsos, int mults, long long cwssb, long long ft8)
{
    if (!scoreLabel) return;
    scoreLabel->setText(tr("Round: %1 QSO, %2 mult — CW/SSB: %3, FT8: %4")
                            .arg(qsos).arg(mults).arg(cwssb).arg(ft8));
}


void ContestPanel::setFt8Mode(bool on)
{
    if (zoneMode) return;
    for (QLabel *c : cells) c->setVisible(!on);
    if (squaresLabel) squaresLabel->setVisible(on);
    if (on) setWorkedSquares(QStringList());
}

void ContestPanel::setWorkedSquares(const QStringList &squares)
{
    if (!squaresLabel) return;
    QStringList u = squares;
    u.removeDuplicates();
    u.sort();
    squaresLabel->setText(tr("Squares worked: %1\n%2")
                              .arg(u.size())
                              .arg(u.isEmpty() ? QStringLiteral("-") : u.join(QStringLiteral(", "))));
}


static const char *ZONE_OFF = "background:#e0e0e0; color:#555;";
static const char *ZONE_ON  = "background:#2e7d32; color:white;";

void ContestPanel::setZoneMatrix(const QStringList &bands)
{
    zoneMode = true;
    for (QLabel *c : cells) c->hide();

    QGridLayout *g = qobject_cast<QGridLayout *>(matrixWidget->layout());
    while (QLayoutItem *it = g->takeAt(0))
    {
        if (it->widget()) delete it->widget();
        delete it;
    }
    zoneCells.clear();
    countryCells.clear();
    g->setSpacing(1);

    for (int z = 1; z <= 40; ++z)
    {
        QLabel *h = new QLabel(QString::number(z), matrixWidget);
        h->setAlignment(Qt::AlignCenter);
        h->setStyleSheet(QStringLiteral("font-size:9px;"));
        g->addWidget(h, 0, z);
    }
    QLabel *ch = new QLabel(tr("Countries"), matrixWidget);
    ch->setAlignment(Qt::AlignCenter);
    g->addWidget(ch, 0, 41);

    int row = 1;
    for (const QString &b : bands)
    {
        QLabel *bl = new QLabel(b, matrixWidget);
        bl->setStyleSheet(QStringLiteral("font-weight:bold;"));
        g->addWidget(bl, row, 0);
        QList<QLabel *> list;
        for (int z = 1; z <= 40; ++z)
        {
            QLabel *c = new QLabel(matrixWidget);
            c->setMinimumSize(18, 16);
            c->setStyleSheet(QString::fromLatin1(ZONE_OFF));
            c->setToolTip(tr("%1, zone %2").arg(b).arg(z));
            g->addWidget(c, row, z);
            list << c;
        }
        zoneCells.insert(b, list);
        QLabel *n = new QLabel(QStringLiteral("0"), matrixWidget);
        n->setAlignment(Qt::AlignCenter);
        g->addWidget(n, row, 41);
        countryCells.insert(b, n);
        ++row;
    }
    matrixWidget->show();
}

void ContestPanel::setWorkedZones(const QString &band, const QSet<int> &zones)
{
    const QList<QLabel *> list = zoneCells.value(band);
    for (int i = 0; i < list.size(); ++i)
        list.at(i)->setStyleSheet(QString::fromLatin1(zones.contains(i + 1) ? ZONE_ON : ZONE_OFF));
}

void ContestPanel::setCountryCount(const QString &band, int count)
{
    if (QLabel *l = countryCells.value(band, nullptr))
        l->setText(QString::number(count));
}
