#ifndef KLOGNG_GRAYLINEWIDGET_H
#define KLOGNG_GRAYLINEWIDGET_H

#include <QWidget>
#include <QTimer>
#include <QPixmap>
#include <QVector>
#include <QPointF>

class DataProxy_SQLite;

// Grayline karte: diena/nakts terminators, musu pozicija,
// lielaa loka linijas uz nostradatajiem QSO.
class GrayLineWidget : public QWidget
{
    Q_OBJECT
public:
    explicit GrayLineWidget(QWidget *parent = nullptr);

    void setMyLocator(const QString &grid);

    void setDataProxy(DataProxy_SQLite *dp) { dataProxy = dp; }

public slots:
    void refreshQSOs();
    void setDxLocator(const QString &grid);

protected:
    void paintEvent(QPaintEvent *) override;
    void resizeEvent(QResizeEvent *) override;

private:
    QPointF gridToXY(const QString &grid) const;
    QPointF lonLatToXY(double lon, double lat) const;
    QRect mapRect() const;
    void drawTerminator(QPainter &p);
    void drawWorld(QPainter &p);
    void drawSun(QPainter &p);

    DataProxy_SQLite *dataProxy = nullptr;
    QString myGrid;
    QPointF myPos;
    QVector<QPointF> qsoPoints;
    QString dxGrid;
    QPointF dxPos = QPointF(-1, -1);
    QTimer timer;
    QPixmap worldMap;
    QPixmap scaled;
};

#endif
