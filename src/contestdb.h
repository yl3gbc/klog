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

private:
    bool createSchema();
    void *db = nullptr;
    void *con = nullptr;
    QString err;
};

#endif
