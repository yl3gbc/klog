#include "dokdata.h"
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QDateTime>
#include <QDir>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QProcess>

DOKData::DOKData(const QString &dataDir, QObject *parent)
    : QObject(parent), dir(dataDir)
{
    reload();
}

static QDate parseDE(const QString &s)
{
    // dd.mm.yy
    const QStringList p = s.trimmed().split(QLatin1Char('.'));
    if (p.size() != 3) return QDate();
    int y = p.at(2).toInt();
    y += (y < 70) ? 2000 : 1900;
    return QDate(y, p.at(1).toInt(), p.at(0).toInt());
}

QString DOKData::dokName(const QString &dok) const
{
    return names.value(dok.trimmed().toUpper());
}

QString DOKData::specialDOK(const QString &call, const QDate &date) const
{
    const auto it = special.constFind(call.trimmed().toUpper());
    if (it == special.constEnd()) return QString();
    for (const Period &p : it.value())
        if (p.from.isValid() && date >= p.from && date <= p.to)
            return p.dok;
    return QString();
}

void DOKData::reload()
{
    special.clear();
    names.clear();

    QFile fd(dir + QStringLiteral("/dok.csv"));
    if (fd.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&fd);
        while (!in.atEnd())
        {
            const QStringList f = in.readLine().split(QLatin1Char(';'));
            if (f.size() >= 3 && !f.at(0).trimmed().isEmpty())
                names.insert(f.at(0).trimmed().toUpper(), f.at(2).trimmed());
        }
        fd.close();
    }

    QFile fs(dir + QStringLiteral("/sdok.csv"));
    if (fs.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&fs);
        while (!in.atEnd())
        {
            const QStringList f = in.readLine().split(QLatin1Char(';'));
            if (f.size() < 5) continue;
            const QString call = f.at(2).trimmed().toUpper();
            if (call.isEmpty()) continue;
            Period p;
            p.dok  = f.at(0).trimmed().toUpper();
            p.from = parseDE(f.at(3));
            p.to   = parseDE(f.at(4));
            // Tukss beigu datums = vel speka
            if (!p.to.isValid()) p.to = QDate(2099, 12, 31);
            if (p.from.isValid())
                special[call].append(p);
        }
        fs.close();
    }
}

void DOKData::updateIfStale()
{
    QFileInfo fi(dir + QStringLiteral("/sdok.csv"));
    if (fi.exists() && fi.lastModified().daysTo(QDateTime::currentDateTime()) < 7)
        return;
    QNetworkRequest req((QUrl(QStringLiteral("https://www.df2et.de/cqrlog/doks.tar.gz"))));
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("KLogNG"));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *r = net.get(req);
    connect(r, &QNetworkReply::finished, this, &DOKData::onReply);
}

void DOKData::onReply()
{
    QNetworkReply *r = qobject_cast<QNetworkReply *>(sender());
    if (!r) return;
    r->deleteLater();
    if (r->error() != QNetworkReply::NoError) return;
    const QByteArray data = r->readAll();
    if (data.size() < 10000) return;

    QDir().mkpath(dir);
    const QString arch = dir + QStringLiteral("/doks.tar.gz");
    QFile f(arch);
    if (!f.open(QIODevice::WriteOnly)) return;
    f.write(data);
    f.close();
    QProcess::execute(QStringLiteral("tar"),
                      QStringList() << QStringLiteral("xzf") << arch
                                    << QStringLiteral("-C") << dir);
    reload();
}
