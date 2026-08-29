#ifndef KLOGNG_SOLARINDICATOR_H
#define KLOGNG_SOLARINDICATOR_H

#include <QLabel>
#include <QNetworkAccessManager>
#include <QTimer>

// Radito soLaros indeksus (SFI / A / K) statusa josla.
// Dati no hamqsl.com XML, atjauno reizi studa.
class SolarIndicator : public QLabel
{
    Q_OBJECT
public:
    explicit SolarIndicator(QWidget *parent = nullptr);

public slots:
    void refresh();

private slots:
    void onReply();

private:
    QNetworkAccessManager net;
    QTimer timer;
};

#endif
