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

private:
    QStringList order;
    QHash<QString, QLabel *> cells;
    QLabel *roundLabel = nullptr;
    QLabel *scoreLabel = nullptr;
    QGridLayout *grid = nullptr;
};

#endif
