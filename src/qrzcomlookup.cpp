#include "qrzcomlookup.h"
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QRegularExpression>

static QString qrzTag(const QString &xml, const QString &tag)
{
    QRegularExpression re("<" + tag + ">([^<]*)</" + tag + ">",
                          QRegularExpression::CaseInsensitiveOption);
    const auto m = re.match(xml);
    return m.hasMatch() ? m.captured(1).trimmed() : QString();
}

QRZComLookup::QRZComLookup(QObject *parent) : QObject(parent) {}

void QRZComLookup::setCredentials(const QString &u, const QString &p)
{
    user = u.trimmed();
    pass = p;
    sid.clear();
}

void QRZComLookup::lookup(const QString &call)
{
    if (!isReady() || call.trimmed().length() < 3) return;
    QString c = call.trimmed().toUpper();
    const QStringList parts = c.split(QLatin1Char('/'), Qt::SkipEmptyParts);
    const QString full = c;
    if (parts.size() > 1)
    {
        c = parts.at(0);
        for (const QString &p : parts) if (p.length() > c.length()) c = p;
    }
    if (c != full) origCall.insert(c, full);
    if (sid.isEmpty())
    {
        if (!pending.contains(c)) pending << c;
        const QString url =
            QStringLiteral("https://xml.qrz.com/xml/1.34?username=%1;password=%2;agent=KLogNG")
            .arg(user, pass);
        QNetworkReply *r = net.get(QNetworkRequest(QUrl(url)));
        connect(r, &QNetworkReply::finished, this, &QRZComLookup::onLogin);
        return;
    }
    doLookup(c);
}

void QRZComLookup::onLogin()
{
    QNetworkReply *r = qobject_cast<QNetworkReply *>(sender());
    if (!r) return;
    r->deleteLater();
    if (r->error() != QNetworkReply::NoError) { pending.clear(); return; }
    const QString xml = QString::fromUtf8(r->readAll());
    sid = qrzTag(xml, QStringLiteral("Key"));
    if (sid.isEmpty()) { pending.clear(); return; }
    const QStringList q = pending;
    pending.clear();
    for (const QString &c : q) doLookup(c);
}

void QRZComLookup::doLookup(const QString &call)
{
    const QString url = QStringLiteral("https://xml.qrz.com/xml/1.34?s=%1;callsign=%2")
                            .arg(sid, call);
    QNetworkReply *r = net.get(QNetworkRequest(QUrl(url)));
    r->setProperty("call", call);
    connect(r, &QNetworkReply::finished, this, &QRZComLookup::onLookup);
}

void QRZComLookup::onLookup()
{
    QNetworkReply *r = qobject_cast<QNetworkReply *>(sender());
    if (!r) return;
    r->deleteLater();
    if (r->error() != QNetworkReply::NoError) return;

    const QString call = r->property("call").toString();
    const QString xml  = QString::fromUtf8(r->readAll());

    // Sesija beigusies - pieteicamies no jauna
    if (xml.contains(QStringLiteral("Session Timeout"), Qt::CaseInsensitive) ||
        xml.contains(QStringLiteral("Invalid session key"), Qt::CaseInsensitive))
    {
        sid.clear();
        lookup(call);
        return;
    }

    QString name = qrzTag(xml, QStringLiteral("name_fmt"));
    if (name.isEmpty())
    {
        const QString f = qrzTag(xml, QStringLiteral("fname"));
        const QString l = qrzTag(xml, QStringLiteral("name"));
        name = f.isEmpty() ? l : (l.isEmpty() ? f : f + QLatin1Char(' ') + l);
    }
    // QTH: addr2 ir pilseta; nogriezam pasta indeksu, ja tas ir prieksa
    QString qth = qrzTag(xml, QStringLiteral("addr2"));
    static const QRegularExpression zip("^\\s*\\d{4,6}\\s+");
    qth.remove(zip);

    // Atdodam PILNO zimi, ne bazes - citadi UPDATE neatrod DB6LL/P
    const QString out = origCall.value(call, call);
    origCall.remove(call);
    emit dataReady(out, name, qth, qrzTag(xml, QStringLiteral("grid")));
}
