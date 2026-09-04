#ifndef KLOGNG_HAMQTH_H
#define KLOGNG_HAMQTH_H

#include <QObject>
#include <QString>
#include <QNetworkAccessManager>

// HamQTH XML API: DOK, vards, QTH, lokators.
// Sesija tiek kesota; pieteiksanas ar LIELAJIEM burtiem.
class HamQTH : public QObject
{
    Q_OBJECT
public:
    explicit HamQTH(QObject *parent = nullptr);
    void setCredentials(const QString &user, const QString &pass);
    bool isReady() const { return !user.isEmpty() && !pass.isEmpty(); }

    // Pieprasa datus par zimi; atbilde nak ka signals
    void lookup(const QString &call);

signals:
    void dataReady(const QString &call, const QString &dok,
                   const QString &name, const QString &qth, const QString &grid);

private slots:
    void onLogin();
    void onLookup();

private:
    void doLookup(const QString &call);
    QNetworkAccessManager net;
    QString user, pass, sid;
    QStringList pending;
};

#endif
