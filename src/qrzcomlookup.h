#ifndef KLOGNG_QRZCOMLOOKUP_H
#define KLOGNG_QRZCOMLOOKUP_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QNetworkAccessManager>

// QRZ.com XML API (xml.qrz.com/xml/1.34): vards, QTH, lokators.
// Strada ari bez abonementa.
class QRZComLookup : public QObject
{
    Q_OBJECT
public:
    explicit QRZComLookup(QObject *parent = nullptr);
    void setCredentials(const QString &user, const QString &pass);
    bool isReady() const { return !user.isEmpty() && !pass.isEmpty(); }
    void lookup(const QString &call);

signals:
    void dataReady(const QString &call, const QString &name,
                   const QString &qth, const QString &grid);

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
