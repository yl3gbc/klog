#ifndef KLOGNG_CONTESTDB_H
#define KLOGNG_CONTESTDB_H

#include <QObject>
#include <QString>

// Sacensibu dati DuckDB baze (~/.klogng/contests.duckdb).
// Sacensibu laika QSO raksta seit; pec sacensibam parnes uz zurnalu.
class ContestDB : public QObject
{
    Q_OBJECT
public:
    explicit ContestDB(const QString &dbPath, QObject *parent = nullptr);
    ~ContestDB();

    bool isOpen() const { return con != nullptr; }
    QString lastError() const { return err; }
    bool addQso(const QString &contestId, int profileId, const QString &date,
                const QString &time, const QString &band, const QString &mode,
                const QString &call, const QString &rstSent, const QString &rstRcvd,
                const QString &exchSent, const QString &exchRcvd, int round);

private:
    bool createSchema();
    void *db = nullptr;
    void *con = nullptr;
    QString err;
};

#endif
