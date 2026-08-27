#include "profilemanager.h"
#include <QSqlDatabase>
#include <QSqlError>
#include <QVariant>
#include <QObject>
#include <QDebug>

bool ProfileManager::exec(QSqlQuery &q, const char *ctx) const
{
    if (q.exec())
        return true;
    qWarning() << "ProfileManager" << ctx << "SQL:" << q.lastError().text();
    return false;
}

bool ProfileManager::ensureSchemaAndMigrate(QString *errorOut)
{
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        if (errorOut) *errorOut = QStringLiteral("Datubaze nav atverta");
        return false;
    }
    QSqlQuery q(db);

    q.prepare(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS profiles ("
        " profile_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " callsign VARCHAR(15) NOT NULL UNIQUE,"
        " operator_name VARCHAR, gridsquare VARCHAR(12), qth VARCHAR,"
        " cq_zone INTEGER, itu_zone INTEGER, dxcc INTEGER,"
        " default_rig VARCHAR, default_antenna VARCHAR, default_tx_pwr REAL,"
        " qsl_via VARCHAR, comment VARCHAR,"
        " created_at TEXT DEFAULT (datetime('now')))"));
    if (!exec(q, "createProfiles")) return false;

    q.prepare(QStringLiteral(
        "CREATE TABLE IF NOT EXISTS profile_variants ("
        " variant_id INTEGER PRIMARY KEY AUTOINCREMENT,"
        " profile_id INTEGER NOT NULL REFERENCES profiles(profile_id),"
        " station_callsign VARCHAR(20) NOT NULL,"
        " kind VARCHAR(8) NOT NULL DEFAULT 'BASE'"
        "   CHECK (kind IN ('BASE','P','M','MM','AM','CONTEST','OTHER')),"
        " gridsquare VARCHAR(12),"
        " dxcc_counts INTEGER NOT NULL DEFAULT 1,"
        " note VARCHAR,"
        " UNIQUE (profile_id, station_callsign))"));
    if (!exec(q, "createVariants")) return false;

    bool hasProfileCol = false;
    q.prepare(QStringLiteral("PRAGMA table_info(log)"));
    if (exec(q, "tableInfo"))
        while (q.next())
            if (q.value(1).toString() == QLatin1String("profile_id"))
                hasProfileCol = true;

    if (!hasProfileCol)
    {
        db.transaction();
        const QStringList steps = {
            QStringLiteral("ALTER TABLE log ADD COLUMN profile_id INTEGER REFERENCES profiles(profile_id)"),
            QStringLiteral("ALTER TABLE log ADD COLUMN variant_id INTEGER REFERENCES profile_variants(variant_id)"),
            QStringLiteral(
              "INSERT OR IGNORE INTO profiles (callsign, operator_name, comment) "
              "SELECT DISTINCT CASE WHEN instr(upper(trim(stationcall)),'/')>0 "
              " THEN substr(upper(trim(stationcall)),1,instr(upper(trim(stationcall)),'/')-1) "
              " ELSE upper(trim(stationcall)) END, operators, comment FROM logs"),
            QStringLiteral(
              "INSERT OR IGNORE INTO profile_variants (profile_id, station_callsign, kind, dxcc_counts) "
              "SELECT DISTINCT p.profile_id, upper(trim(l.stationcall)), "
              " CASE WHEN upper(trim(l.stationcall)) LIKE '%/MM' THEN 'MM' "
              "      WHEN upper(trim(l.stationcall)) LIKE '%/AM' THEN 'AM' "
              "      WHEN upper(trim(l.stationcall)) LIKE '%/P'  THEN 'P' "
              "      WHEN upper(trim(l.stationcall)) LIKE '%/M'  THEN 'M' "
              "      WHEN l.logtype IS NOT NULL AND l.logtype<>'DX' THEN 'CONTEST' "
              "      ELSE 'OTHER' END, "
              " CASE WHEN upper(trim(l.stationcall)) LIKE '%/MM' THEN 0 ELSE 1 END "
              "FROM logs l JOIN profiles p ON p.callsign = "
              " CASE WHEN instr(upper(trim(l.stationcall)),'/')>0 "
              "  THEN substr(upper(trim(l.stationcall)),1,instr(upper(trim(l.stationcall)),'/')-1) "
              "  ELSE upper(trim(l.stationcall)) END "
              "WHERE upper(trim(l.stationcall)) <> p.callsign"),
            QStringLiteral(
              "UPDATE log SET "
              " profile_id = (SELECT p.profile_id FROM logs l JOIN profiles p ON p.callsign = "
              "   CASE WHEN instr(upper(trim(l.stationcall)),'/')>0 "
              "    THEN substr(upper(trim(l.stationcall)),1,instr(upper(trim(l.stationcall)),'/')-1) "
              "    ELSE upper(trim(l.stationcall)) END WHERE l.id = log.lognumber), "
              " variant_id = (SELECT v.variant_id FROM logs l JOIN profile_variants v "
              "   ON v.station_callsign = upper(trim(l.stationcall)) WHERE l.id = log.lognumber)"),
            QStringLiteral("CREATE INDEX IF NOT EXISTS idx_log_profile ON log (profile_id)")
        };
        for (const QString &s : steps) {
            q.prepare(s);
            if (!exec(q, "migrate")) {
                db.rollback();
                if (errorOut) *errorOut = q.lastError().text();
                return false;
            }
        }
        db.commit();
    }

    // Trigeris: katrs jauns QSO automatiski sanem profile_id/variant_id
    // pec lognumber -> logs.stationcall kartejuma. Darbojas visiem ievades
    // celiem (manuala ievade, ADIF imports, UDP) bez izmainam KLog koda.
    q.prepare(QStringLiteral(
        "CREATE TRIGGER IF NOT EXISTS trg_log_profile "
        "AFTER INSERT ON log "
        "WHEN NEW.profile_id IS NULL "
        "BEGIN "
        " UPDATE log SET "
        "  profile_id = (SELECT p.profile_id FROM logs l JOIN profiles p ON p.callsign = "
        "    CASE WHEN instr(upper(trim(l.stationcall)),'/')>0 "
        "     THEN substr(upper(trim(l.stationcall)),1,instr(upper(trim(l.stationcall)),'/')-1) "
        "     ELSE upper(trim(l.stationcall)) END WHERE l.id = NEW.lognumber), "
        "  variant_id = (SELECT v.variant_id FROM logs l JOIN profile_variants v "
        "    ON v.station_callsign = upper(trim(l.stationcall)) WHERE l.id = NEW.lognumber) "
        " WHERE id = NEW.id; "
        "END"));
    if (!exec(q, "createTrigger")) return false;

    return true;
}

QList<Profile> ProfileManager::listProfiles() const
{
    QList<Profile> out;
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral(
        "SELECT p.profile_id, p.callsign, p.operator_name, p.gridsquare, p.qth,"
        " p.cq_zone, p.itu_zone, p.dxcc, p.comment,"
        " (SELECT count(*) FROM log WHERE log.profile_id = p.profile_id) "
        "FROM profiles p ORDER BY p.callsign"));
    if (!exec(q, "listProfiles")) return out;
    while (q.next()) {
        Profile p;
        p.id = q.value(0).toInt();
        p.callsign = q.value(1).toString();
        p.operatorName = q.value(2).toString();
        p.gridsquare = q.value(3).toString();
        p.qth = q.value(4).toString();
        p.cqZone = q.value(5).toInt();
        p.ituZone = q.value(6).toInt();
        p.dxcc = q.value(7).toInt();
        p.comment = q.value(8).toString();
        p.qsoCount = q.value(9).toInt();
        out.append(p);
    }
    return out;
}

Profile ProfileManager::getProfile(int id) const
{
    Profile p;
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral(
        "SELECT profile_id, callsign, operator_name, gridsquare, qth,"
        " cq_zone, itu_zone, dxcc, comment FROM profiles WHERE profile_id = :id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (exec(q, "getProfile") && q.next()) {
        p.id = q.value(0).toInt();
        p.callsign = q.value(1).toString();
        p.operatorName = q.value(2).toString();
        p.gridsquare = q.value(3).toString();
        p.qth = q.value(4).toString();
        p.cqZone = q.value(5).toInt();
        p.ituZone = q.value(6).toInt();
        p.dxcc = q.value(7).toInt();
        p.comment = q.value(8).toString();
    }
    return p;
}

int ProfileManager::createProfile(const Profile &p)
{
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral(
        "INSERT INTO profiles (callsign, operator_name, gridsquare, qth,"
        " cq_zone, itu_zone, dxcc, comment) "
        "VALUES (:c,:o,:g,:q,:cq,:itu,:dx,:cm)"));
    q.bindValue(QStringLiteral(":c"),  p.callsign.trimmed().toUpper());
    q.bindValue(QStringLiteral(":o"),  p.operatorName);
    q.bindValue(QStringLiteral(":g"),  p.gridsquare);
    q.bindValue(QStringLiteral(":q"),  p.qth);
    q.bindValue(QStringLiteral(":cq"), p.cqZone);
    q.bindValue(QStringLiteral(":itu"),p.ituZone);
    q.bindValue(QStringLiteral(":dx"), p.dxcc);
    q.bindValue(QStringLiteral(":cm"), p.comment);
    if (!exec(q, "createProfile")) return -1;
    return q.lastInsertId().toInt();
}

bool ProfileManager::updateProfile(const Profile &p)
{
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral(
        "UPDATE profiles SET callsign=:c, operator_name=:o, gridsquare=:g,"
        " qth=:q, cq_zone=:cq, itu_zone=:itu, dxcc=:dx, comment=:cm "
        "WHERE profile_id=:id"));
    q.bindValue(QStringLiteral(":c"),  p.callsign.trimmed().toUpper());
    q.bindValue(QStringLiteral(":o"),  p.operatorName);
    q.bindValue(QStringLiteral(":g"),  p.gridsquare);
    q.bindValue(QStringLiteral(":q"),  p.qth);
    q.bindValue(QStringLiteral(":cq"), p.cqZone);
    q.bindValue(QStringLiteral(":itu"),p.ituZone);
    q.bindValue(QStringLiteral(":dx"), p.dxcc);
    q.bindValue(QStringLiteral(":cm"), p.comment);
    q.bindValue(QStringLiteral(":id"), p.id);
    return exec(q, "updateProfile");
}

bool ProfileManager::deleteProfile(int id, QString *errorOut)
{
    const int n = qsoCount(id);
    if (n > 0) {
        if (errorOut)
            *errorOut = QObject::tr("Profila ir %1 QSO. Vispirms eksporte tos "
                                    "ADIF formata vai parcel uz citu profilu.").arg(n);
        return false;
    }
    QSqlDatabase db = QSqlDatabase::database();
    db.transaction();
    QSqlQuery q(db);
    q.prepare(QStringLiteral("DELETE FROM profile_variants WHERE profile_id=:id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (!exec(q, "deleteVariantsOfProfile")) { db.rollback(); return false; }
    q.prepare(QStringLiteral("DELETE FROM profiles WHERE profile_id=:id"));
    q.bindValue(QStringLiteral(":id"), id);
    if (!exec(q, "deleteProfile")) { db.rollback(); return false; }
    db.commit();
    return true;
}

int ProfileManager::qsoCount(int profileId) const
{
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral("SELECT count(*) FROM log WHERE profile_id=:id"));
    q.bindValue(QStringLiteral(":id"), profileId);
    if (exec(q, "qsoCount") && q.next())
        return q.value(0).toInt();
    return 0;
}

QList<ProfileVariant> ProfileManager::listVariants(int profileId) const
{
    QList<ProfileVariant> out;
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral(
        "SELECT variant_id, profile_id, station_callsign, kind, gridsquare,"
        " dxcc_counts FROM profile_variants WHERE profile_id=:id "
        "ORDER BY station_callsign"));
    q.bindValue(QStringLiteral(":id"), profileId);
    if (!exec(q, "listVariants")) return out;
    while (q.next()) {
        ProfileVariant v;
        v.id = q.value(0).toInt();
        v.profileId = q.value(1).toInt();
        v.stationCallsign = q.value(2).toString();
        v.kind = q.value(3).toString();
        v.gridsquare = q.value(4).toString();
        v.dxccCounts = q.value(5).toInt() != 0;
        out.append(v);
    }
    return out;
}

int ProfileManager::createVariant(const ProfileVariant &v)
{
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral(
        "INSERT INTO profile_variants (profile_id, station_callsign, kind,"
        " gridsquare, dxcc_counts) VALUES (:p,:s,:k,:g,:d)"));
    q.bindValue(QStringLiteral(":p"), v.profileId);
    q.bindValue(QStringLiteral(":s"), v.stationCallsign.trimmed().toUpper());
    q.bindValue(QStringLiteral(":k"), v.kind);
    q.bindValue(QStringLiteral(":g"), v.gridsquare);
    q.bindValue(QStringLiteral(":d"), v.dxccCounts ? 1 : 0);
    if (!exec(q, "createVariant")) return -1;
    return q.lastInsertId().toInt();
}

bool ProfileManager::deleteVariant(int variantId)
{
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral("DELETE FROM profile_variants WHERE variant_id=:id"));
    q.bindValue(QStringLiteral(":id"), variantId);
    return exec(q, "deleteVariant");
}
