#include "contestdb.h"
#include <QFileInfo>
#include <QDir>

#ifdef KLOGNG_HAS_DUCKDB
#include "duckdb.h"
#endif

ContestDB::ContestDB(const QString &dbPath, QObject *parent)
    : QObject(parent)
{
#ifdef KLOGNG_HAS_DUCKDB
    QDir().mkpath(QFileInfo(dbPath).absolutePath());
    duckdb_database *d = new duckdb_database;
    duckdb_connection *c = new duckdb_connection;
    if (duckdb_open(dbPath.toUtf8().constData(), d) != DuckDBSuccess)
    {
        err = QStringLiteral("duckdb_open failed: %1").arg(dbPath);
        delete d; delete c;
        return;
    }
    if (duckdb_connect(*d, c) != DuckDBSuccess)
    {
        err = QStringLiteral("duckdb_connect failed");
        duckdb_close(d);
        delete d; delete c;
        return;
    }
    db = d;
    con = c;
    createSchema();
#else
    Q_UNUSED(dbPath);
    err = QStringLiteral("KLogNG built without DuckDB support");
#endif
}

ContestDB::~ContestDB()
{
#ifdef KLOGNG_HAS_DUCKDB
    if (con) { duckdb_disconnect(static_cast<duckdb_connection *>(con)); delete static_cast<duckdb_connection *>(con); }
    if (db)  { duckdb_close(static_cast<duckdb_database *>(db));        delete static_cast<duckdb_database *>(db); }
#endif
}

bool ContestDB::createSchema()
{
#ifdef KLOGNG_HAS_DUCKDB
    if (!con) return false;
    const char *sql =
        "CREATE SEQUENCE IF NOT EXISTS seq_contest_qso START 1;"
        "CREATE TABLE IF NOT EXISTS contest_qso ("
        " id BIGINT PRIMARY KEY DEFAULT nextval('seq_contest_qso'),"
        " contest_id VARCHAR NOT NULL,"
        " profile_id INTEGER,"
        " qso_date DATE NOT NULL,"
        " time_on TIME NOT NULL,"
        " band VARCHAR,"
        " mode VARCHAR,"
        " freq DOUBLE,"
        " call VARCHAR NOT NULL,"
        " rst_sent VARCHAR,"
        " rst_rcvd VARCHAR,"
        " exch_sent VARCHAR,"
        " exch_rcvd VARCHAR,"
        " serial_sent INTEGER,"
        " serial_rcvd INTEGER,"
        " round INTEGER,"
        " points INTEGER DEFAULT 0,"
        " is_mult BOOLEAN DEFAULT FALSE,"
        " is_dupe BOOLEAN DEFAULT FALSE,"
        " exported BOOLEAN DEFAULT FALSE);";
    duckdb_state st = duckdb_query(*static_cast<duckdb_connection *>(con), sql, nullptr);
    if (st != DuckDBSuccess)
    {
        err = QStringLiteral("createSchema failed");
        return false;
    }
    return true;
#else
    return false;
#endif
}
