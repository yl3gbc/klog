#include "contestpanel.h"
#include <QLabel>
#include <QGridLayout>
#include <QVBoxLayout>
#include <QFile>
#include <QTextStream>

ContestPanel::ContestPanel(QWidget *parent) : QWidget(parent)
{
    roundLabel = new QLabel(tr("No contest"), this);
    roundLabel->setAlignment(Qt::AlignCenter);
    roundLabel->setStyleSheet(QStringLiteral("font-weight: bold;"));

    scoreLabel = new QLabel(QString(), this);
    scoreLabel->setAlignment(Qt::AlignCenter);

    grid = new QGridLayout;
    grid->setSpacing(2);

    QVBoxLayout *lay = new QVBoxLayout(this);
    lay->addWidget(roundLabel);
    lay->addLayout(grid, 1);
    lay->addWidget(scoreLabel);
}

void ContestPanel::loadDistricts(const QString &path)
{
    order.clear();
    qDeleteAll(cells);
    cells.clear();

    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    QTextStream in(&f);
    while (!in.atEnd())
    {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;
        const QString code = line.section(QLatin1Char(';'), 0, 0).trimmed().toUpper();
        const QString name = line.section(QLatin1Char(';'), 1, 1).trimmed();
        if (code.isEmpty()) continue;
        order << code;
        QLabel *c = new QLabel(code, this);
        c->setAlignment(Qt::AlignCenter);
        c->setToolTip(name);
        c->setFrameStyle(QFrame::StyledPanel | QFrame::Raised);
        c->setMinimumWidth(34);
        cells.insert(code, c);
    }
    f.close();

    int row = 0, col = 0;
    for (const QString &code : order)
    {
        grid->addWidget(cells.value(code), row, col);
        if (++col >= 8) { col = 0; ++row; }
    }
    setWorkedInRound(QSet<QString>());
}

void ContestPanel::setRound(int n, const QString &timeLeft)
{
    if (!roundLabel) return;
    if (n <= 0)
        roundLabel->setText(tr("Outside contest hours"));
    else
        roundLabel->setText(tr("Round %1 — %2 left").arg(n).arg(timeLeft));
}

void ContestPanel::setWorkedInRound(const QSet<QString> &codes)
{
    for (auto it = cells.cbegin(); it != cells.cend(); ++it)
    {
        const bool done = codes.contains(it.key());
        it.value()->setStyleSheet(done
            ? QStringLiteral("background:#2e7d32; color:white;")
            : QStringLiteral("background:#e0e0e0; color:#555;"));
    }
}

void ContestPanel::setScore(int qsos, int mults, long long cwssb, long long ft8)
{
    if (!scoreLabel) return;
    scoreLabel->setText(tr("Round: %1 QSO, %2 mult — CW/SSB: %3, FT8: %4")
                            .arg(qsos).arg(mults).arg(cwssb).arg(ft8));
}
