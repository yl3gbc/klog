#include "callsignservices.h"
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileInfo>
#include <QDateTime>
#include <QDir>

CallsignServices::CallsignServices(const QString &dataDir, QObject *parent)
    : QObject(parent), dir(dataDir)
{
    reload();
    connect(&daily, &QTimer::timeout, this, &CallsignServices::updateIfStale);
    daily.start(6 * 60 * 60 * 1000);          // parbauda ik pa 6 stundam
    QTimer::singleShot(5000, this, &CallsignServices::updateIfStale);
}

// Bazes zime bez prefiksa un sufiksa: YL/EU1EU/P -> EU1EU
QString CallsignServices::baseCall(const QString &call)
{
    QString c = call.trimmed().toUpper();
    if (c.isEmpty()) return c;
    const QStringList parts = c.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (parts.size() < 2) return c;
    // garaka dala parasti ir bazes zime
    QString best = parts.at(0);
    for (const QString &p : parts)
        if (p.length() > best.length()) best = p;
    return best;
}

void CallsignServices::reload()
{
    lotw.clear();
    eqsl.clear();

    QFile fl(dir + QStringLiteral("/lotw.csv"));
    if (fl.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&fl);
        while (!in.atEnd())
        {
            const QString line = in.readLine();
            const int c = line.indexOf(QLatin1Char(','));
            if (c > 0) lotw.insert(line.left(c).trimmed().toUpper());
        }
        fl.close();
    }

    QFile fe(dir + QStringLiteral("/eqsl.txt"));
    if (fe.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QTextStream in(&fe);
        bool first = true;
        while (!in.atEnd())
        {
            const QString line = in.readLine().trimmed().toUpper();
            if (first) { first = false; continue; }
            if (!line.isEmpty()) eqsl.insert(line);
        }
        fe.close();
    }
}

bool CallsignServices::usesLoTW(const QString &call) const
{
    if (call.trimmed().isEmpty()) return false;
    const QString c = call.trimmed().toUpper();
    return lotw.contains(c) || lotw.contains(baseCall(c));
}

bool CallsignServices::useseQSL(const QString &call) const
{
    if (call.trimmed().isEmpty()) return false;
    const QString c = call.trimmed().toUpper();
    return eqsl.contains(c) || eqsl.contains(baseCall(c));
}

// Lejupielade, ja fails vecaks par 24 stundam
void CallsignServices::updateIfStale()
{
    const QDateTime now = QDateTime::currentDateTime();
    struct { const char *url; const char *file; } src[] = {
        { "https://lotw.arrl.org/lotw-user-activity.csv", "lotw.csv" },
        { "https://www.eqsl.cc/qslcard/DownloadedFiles/AGMemberList.txt", "eqsl.txt" },
    };
    for (const auto &s : src)
    {
        QFileInfo fi(dir + QLatin1Char('/') + QLatin1String(s.file));
        if (!fi.exists() || fi.lastModified().daysTo(now) >= 1)
            fetch(QLatin1String(s.url), QLatin1String(s.file));
    }
}

void CallsignServices::fetch(const QString &url, const QString &fileName)
{
    QNetworkRequest req((QUrl(url)));
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("KLogNG"));
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);
    QNetworkReply *r = net.get(req);
    r->setProperty("fileName", fileName);
    connect(r, &QNetworkReply::finished, this, &CallsignServices::onReply);
}

void CallsignServices::onReply()
{
    QNetworkReply *r = qobject_cast<QNetworkReply *>(sender());
    if (!r) return;
    r->deleteLater();
    if (r->error() != QNetworkReply::NoError) return;

    const QByteArray data = r->readAll();
    if (data.size() < 10000) return;          // aizdomigi mazs - neaiztiekam veco

    QDir().mkpath(dir);
    QFile f(dir + QLatin1Char('/') + r->property("fileName").toString());
    if (f.open(QIODevice::WriteOnly))
    {
        f.write(data);
        f.close();
        reload();
    }
}
