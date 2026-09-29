#ifndef KLOGNG_CONTESTDEF_H
#define KLOGNG_CONTESTDEF_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QTime>
#include <QDate>

// Viena sacensibu tuure
struct ContestRound {
    int n = 0;
    QTime start, end;
    QStringList modes;
    QStringList exchSent, exchRcvd;
    QString multiplier;          // "district", "grid4" vai tukss
};

// Sacensibu apraksts no data/contests/*.json
class ContestDef
{
public:
    bool load(const QString &path);

    QString id, name, date, timeZone, exportFormat;
    QStringList bands;
    QStringList exchSent, exchRcvd;
    QString multiplier;
    bool multPerRound = false;
    bool ownDistrictNoMult = false;
    QString dupeScope;           // "round+mode"
    int pointsPerQSO = 1;
    int maxPowerW = 0;
    QList<ContestRound> rounds;

    // Kura tuure sim laikam; 0 ja neviena
    int roundAt(const QTime &t) const;
    const ContestRound *round(int n) const;

    QString lastError() const { return err; }

private:
    QString err;
};

#endif
