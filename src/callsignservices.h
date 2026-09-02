#ifndef KLOGNG_CALLSIGNSERVICES_H
#define KLOGNG_CALLSIGNSERVICES_H

#include <QObject>
#include <QSet>
#include <QString>
#include <QNetworkAccessManager>
#include <QTimer>

// Parbauda, kuros servisos dotais izsaukuma signals ir registrets.
// Dati: ~/.klogng/data/lotw.csv (ARRL) un eqsl.txt (eQSL AG saraksts).
class CallsignServices : public QObject
{
    Q_OBJECT
public:
    explicit CallsignServices(const QString &dataDir, QObject *parent = nullptr);

    bool usesLoTW(const QString &call) const;
    bool useseQSL(const QString &call) const;
    void reload();
    void updateIfStale();      // lejupielade, ja faili vecaki par 24h


    static QString baseCall(const QString &call);

private slots:
    void onReply();

private:
    void fetch(const QString &url, const QString &fileName);
    QNetworkAccessManager net;
    QTimer daily;
    QString dir;
    QSet<QString> lotw;
    QSet<QString> eqsl;
};

#endif
