#include "logupdater.h"
#include "qrzcomlookup.h"
#include "hamqth.h"
#include <QSqlQuery>

LogUpdater::LogUpdater(QRZComLookup *q, HamQTH *h, QObject *parent)
    : QObject(parent), qrz(q), hamqth(h)
{
    timer.setInterval(1200);          // viena zime ~sekunde
    connect(&timer, &QTimer::timeout, this, &LogUpdater::next);
    if (qrz)    connect(qrz, &QRZComLookup::dataReady, this, &LogUpdater::onQRZ);
    if (hamqth) connect(hamqth, &HamQTH::dataReady, this, &LogUpdater::onDOK);
}

void LogUpdater::start(const QStringList &calls)
{
    queue = calls;
    total = queue.size();
    done = 0;
    updated = 0;
    if (total > 0) timer.start();
}

void LogUpdater::stop()
{
    timer.stop();
    queue.clear();
    emit finished(updated);
}

void LogUpdater::next()
{
    if (queue.isEmpty())
    {
        timer.stop();
        emit finished(updated);
        return;
    }
    const QString c = queue.takeFirst();
    done++;
    if (qrz && qrz->isReady())    qrz->lookup(c);
    if (hamqth && hamqth->isReady()) hamqth->lookup(c);
    emit progress(done, total, updated);
}

void LogUpdater::onQRZ(const QString &call, const QString &name,
                       const QString &qth, const QString &grid)
{
    if (call.isEmpty()) return;
    QStringList sets;
    if (!name.isEmpty())
        sets << "name=CASE WHEN name IS NULL OR name='' THEN :n ELSE name END";
    if (!qth.isEmpty())
        sets << "qth=CASE WHEN qth IS NULL OR qth='' THEN :q ELSE qth END";
    if (!grid.isEmpty())
        sets << "gridsquare=CASE WHEN gridsquare IS NULL OR gridsquare='' THEN :g ELSE gridsquare END";
    if (sets.isEmpty()) return;
    QSqlQuery s;
    s.prepare("UPDATE log SET " + sets.join(", ") + " WHERE call=:c");
    if (!name.isEmpty()) s.bindValue(":n", name);
    if (!qth.isEmpty())  s.bindValue(":q", qth);
    if (!grid.isEmpty()) s.bindValue(":g", grid.toUpper());
    s.bindValue(":c", call.toUpper());
    if (s.exec() && s.numRowsAffected() > 0) updated++;
}

void LogUpdater::onDOK(const QString &call, const QString &dok, const QString &,
                       const QString &, const QString &)
{
    if (call.isEmpty() || dok.isEmpty()) return;
    QSqlQuery s;
    s.prepare("UPDATE log SET darc_dok=:d WHERE call=:c "
              "AND (darc_dok IS NULL OR darc_dok='')");
    s.bindValue(":d", dok);
    s.bindValue(":c", call.toUpper());
    s.exec();
}
