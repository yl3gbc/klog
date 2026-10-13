#ifndef KLOGNG_CONTESTPANEL_H
#define KLOGNG_CONTESTPANEL_H

#include <QWidget>
#include <QHash>
#include <QSet>
#include <QStringList>

class QLabel;
class QGridLayout;

class ContestPanel : public QWidget
{
    Q_OBJECT
public:
    explicit ContestPanel(QWidget *parent = nullptr);
    void loadDistricts(const QString &path);
    void setRound(int n, const QString &timeLeft);
    void setWorkedInRound(const QSet<QString> &codes);
    void setScore(int qsos, int mults, long long cwssb, long long ft8);
    // Zonu matrica (CQ WW): joslas x CQ zonas, plus valstu skaits uz joslu
    void setZoneMatrix(const QStringList &bands);
    void setWorkedZones(const QString &band, const QSet<int> &zones);
    void setCountryCount(const QString &band, int count);
    // FT8 turees: rajonu rezgis paslepts, raditi tikai kvadrati
    void setFt8Mode(bool on);
    void setWorkedSquares(const QStringList &squares);

private:
    QStringList order;
    QHash<QString, QLabel *> cells;
    QLabel *roundLabel = nullptr;
    QLabel *scoreLabel = nullptr;
    QWidget *matrixWidget = nullptr;
    QHash<QString, QList<QLabel *>> zoneCells;
    QHash<QString, QLabel *> countryCells;
    bool zoneMode = false;
    QLabel *squaresLabel = nullptr;
    QGridLayout *grid = nullptr;
};

#endif
