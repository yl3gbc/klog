#include "wsjtxlogwatcher.h"
#include "dataproxy_sqlite.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QSqlQuery>

WSJTXLogWatcher::WSJTXLogWatcher(DataProxy_SQLite *dp, QObject *parent)
    : QObject(parent), dataProxy(dp)
{
    debounce.setSingleShot(true);
    debounce.setInterval(2000);          // failu raksta pakapeniski
    connect(&debounce, &QTimer::timeout, this, &WSJTXLogWatcher::doCheck);
    connect(&watcher, &QFileSystemWatcher::fileChanged,
            this, &WSJTXLogWatcher::onFileChanged);
}

void WSJTXLogWatcher::setLogFile(const QString &path)
{
    if (!watcher.files().isEmpty())
        watcher.removePaths(watcher.files());
    file = path.trimmed();
    if (file.isEmpty() || !QFileInfo::exists(file)) return;
    watcher.addPath(file);
}

void WSJTXLogWatcher::onFileChanged(const QString &path)
{
    // Dazas programmas failu parraksta, tapec sekosana var nokrist
    if (!watcher.files().contains(path) && QFileInfo::exists(path))
        watcher.addPath(path);
    debounce.start();
}

void WSJTXLogWatcher::checkNow()
{
    doCheck();
}

void WSJTXLogWatcher::doCheck()
{
    if (file.isEmpty() || !dataProxy) return;
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) return;
    const QString txt = QString::fromUtf8(f.readAll());
    f.close();

    int p = txt.indexOf(QStringLiteral("<eoh>"), 0, Qt::CaseInsensitive);
    const QString body = (p >= 0) ? txt.mid(p + 5) : txt;

    static const QRegularExpression reCall("<call:(\\d+)[^>]*>([^<]*)",
                                           QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reDate("<qso_date:(\\d+)[^>]*>([^<]*)",
                                           QRegularExpression::CaseInsensitiveOption);
    static const QRegularExpression reTime("<time_on:(\\d+)[^>]*>([^<]*)",
                                           QRegularExpression::CaseInsensitiveOption);

    int missing = 0;
    const QStringList recs = body.split(QStringLiteral("<eor>"), Qt::SkipEmptyParts,
                                        Qt::CaseInsensitive);
    for (const QString &r : recs)
    {
        const auto mc = reCall.match(r);
        const auto md = reDate.match(r);
        const auto mt = reTime.match(r);
        if (!mc.hasMatch() || !md.hasMatch() || !mt.hasMatch()) continue;

        const QString call = mc.captured(2).trimmed().toUpper();
        const QString d = md.captured(2).trimmed();
        const QString t = mt.captured(2).trimmed();
        if (call.isEmpty() || d.length() < 8 || t.length() < 4) continue;

        const QString iso = QStringLiteral("%1-%2-%3 %4:%5")
            .arg(d.left(4), d.mid(4,2), d.mid(6,2), t.left(2), t.mid(2,2));

        QSqlQuery q;
        q.prepare("SELECT count(*) FROM log WHERE call=:c AND substr(qso_date,1,16)=:d");
        q.bindValue(":c", call);
        q.bindValue(":d", iso);
        if (q.exec() && q.next() && q.value(0).toInt() == 0)
            missing++;
    }
    if (missing > 0)
        emit qsosImported(missing);
}
