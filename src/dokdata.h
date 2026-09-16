#ifndef KLOGNG_DOKDATA_H
#define KLOGNG_DOKDATA_H

#include <QObject>
#include <QHash>
#include <QDate>
#include <QString>
#include <QNetworkAccessManager>

// DARC DOK dati no https://www.df2et.de/cqrlog/doks.tar.gz
//   dok.csv  - vardnica: DOK;distrikts;nosaukums
//   sdok.csv - specialie: DOK;nosaukums;zime;no;lidz;atsauce
// Vienai zimei var but vairaki periodi ar dazadiem DOK.
class DOKData : public QObject
{
    Q_OBJECT
public:
    explicit DOKData(const QString &dataDir, QObject *parent = nullptr);

    // Specialais DOK sai zimei sai datuma; tukss, ja nav
    QString specialDOK(const QString &call, const QDate &date) const;
    // DOK nosaukums no vardnicas (piem. Y17 -> Berlin)
    QString dokName(const QString &dok) const;
    void reload();
    void updateIfStale();

private slots:
    void onReply();

private:
    struct Period { QString dok; QDate from, to; };
    QString dir;
    QHash<QString, QList<Period>> special;
    QHash<QString, QString> names;
    QNetworkAccessManager net;
};

#endif
