#include "hamqth.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QUrlQuery>
#include <QRegularExpression>

static QString tagValue(const QString &xml, const QString &tag)
{
    QRegularExpression re("<" + tag + ">([^<]*)</" + tag + ">");
    const auto m = re.match(xml);
    return m.hasMatch() ? m.captured(1).trimmed() : QString();
}

HamQTH::HamQTH(QObject *parent) : QObject(parent) {}

void HamQTH::setCredentials(const QString &u, const QString &p)
{
    // HamQTH pieteiksanas ir registrjutiga - lietotajvards ar LIELAJIEM
    user = u.trimmed().toUpper();
    pass = p;
    sid.clear();
}

void HamQTH::lookup(const QString &call)
{
    if (!isReady() || call.trimmed().length() < 3) return;
    // /P, /A, /M ir pagaidu darbibas vietas - DOK ir bazes zimei
    QString c = call.trimmed().toUpper();
    const QStringList parts = c.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    if (parts.size() > 1)
    {
        c = parts.at(0);
        for (const QString &p : parts) if (p.length() > c.length()) c = p;
    }
    if (sid.isEmpty())
    {
        if (!pending.contains(c)) pending << c;
        QUrl url("https://www.hamqth.com/xml.php");
        QUrlQuery q;
        q.addQueryItem("u", user);
        q.addQueryItem("p", pass);
        url.setQuery(q);
        QNetworkReply *r = net.get(QNetworkRequest(url));
        connect(r, &QNetworkReply::finished, this, &HamQTH::onLogin);
        return;
    }
    doLookup(c);
}

void HamQTH::onLogin()
{
    QNetworkReply *r = qobject_cast<QNetworkReply *>(sender());
    if (!r) return;
    r->deleteLater();
    if (r->error() != QNetworkReply::NoError) { pending.clear(); return; }

    const QString xml = QString::fromUtf8(r->readAll());
    sid = tagValue(xml, QStringLiteral("session_id"));
    qWarning() << "KLOGNG HAMQTH LOGIN: sid=" << (sid.isEmpty() ? QStringLiteral("TUKSS") : sid.left(8))
               << " pending=" << pending.size() << " atbilde=" << xml.left(200);
    if (sid.isEmpty()) { pending.clear(); return; }

    const QStringList q = pending;
    pending.clear();
    for (const QString &c : q) doLookup(c);
}

void HamQTH::doLookup(const QString &call)
{
    qWarning() << "KLOGNG HAMQTH DOLOOKUP:" << call;
    QUrl url("https://www.hamqth.com/xml.php");
    QUrlQuery q;
    q.addQueryItem("id", sid);
    q.addQueryItem("callsign", call.toLower());
    q.addQueryItem("prg", "KLogNG");
    url.setQuery(q);
    QNetworkReply *r = net.get(QNetworkRequest(url));
    r->setProperty("call", call);
    connect(r, &QNetworkReply::finished, this, &HamQTH::onLookup);
}

void HamQTH::onLookup()
{
    QNetworkReply *r = qobject_cast<QNetworkReply *>(sender());
    if (!r) return;
    r->deleteLater();
    if (r->error() != QNetworkReply::NoError) return;

    const QString call = r->property("call").toString();
    const QString xml  = QString::fromUtf8(r->readAll());

    // Sesija beigusies - pieteicamies no jauna un atkartojam
    if (xml.contains(QStringLiteral("<error>"), Qt::CaseInsensitive) &&
        xml.contains(QStringLiteral("session"), Qt::CaseInsensitive))
    {
        sid.clear();
        lookup(call);
        return;
    }

    qWarning() << "KLOGNG HAMQTH ATBILDE:" << call
               << "dok=" << tagValue(xml, QStringLiteral("dok"))
               << "nick=" << tagValue(xml, QStringLiteral("nick"))
               << "err=" << xml.contains(QStringLiteral("<error>"));
    emit dataReady(call,
                   tagValue(xml, QStringLiteral("dok")),
                   tagValue(xml, QStringLiteral("nick")),
                   tagValue(xml, QStringLiteral("qth")),
                   tagValue(xml, QStringLiteral("grid")));
}
