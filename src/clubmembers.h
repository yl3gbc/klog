#ifndef KLOGNG_CLUBMEMBERS_H
#define KLOGNG_CLUBMEMBERS_H

#include <QObject>
#include <QHash>
#include <QString>
#include <QStringList>

// Klubu biedru saraksti CQRLOG formata:
//   1. rinda - isais nosaukums, 2. - pilnais, talak zime;numurs
// Numurs pieder operatoram, tapec meklē arī bāzes zīmi bez prefiksa.
class ClubMembers : public QObject
{
    Q_OBJECT
public:
    explicit ClubMembers(const QString &dataDir, QObject *parent = nullptr);

    // Atgriež "AGB #265" formā, katram klubam, kur zīme atrasta
    QStringList lookup(const QString &call) const;
    void reload();
    int clubCount() const { return clubs.size(); }

private:
    struct Club { QString shortName, fullName; QHash<QString, QString> members; };
    QString dir;
    QList<Club> clubs;
};

#endif
