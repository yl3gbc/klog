#ifndef KLOGNG_PROFILEMANAGER_H
#define KLOGNG_PROFILEMANAGER_H

#include <QString>
#include <QList>
#include <QSqlQuery>

struct Profile {
    int     id = -1;
    QString callsign;
    QString operatorName;
    QString gridsquare;
    QString qth;
    int     cqZone  = 0;
    int     ituZone = 0;
    int     dxcc    = 0;
    QString comment;
    int     qsoCount = 0;
};

struct ProfileVariant {
    int     id = -1;
    int     profileId = -1;
    QString stationCallsign;
    QString kind;
    QString gridsquare;
    bool    dxccCounts = true;
};

class ProfileManager
{
public:
    ProfileManager() = default;

    bool ensureSchemaAndMigrate(QString *errorOut = nullptr);

    QList<Profile> listProfiles() const;
    Profile getProfile(int id) const;
    int  createProfile(const Profile &p);
    bool updateProfile(const Profile &p);
    bool deleteProfile(int id, QString *errorOut);

    QList<ProfileVariant> listVariants(int profileId) const;
    int  createVariant(const ProfileVariant &v);
    bool deleteVariant(int variantId);

    int  qsoCount(int profileId) const;

private:
    bool exec(QSqlQuery &q, const char *ctx) const;
};

#endif
