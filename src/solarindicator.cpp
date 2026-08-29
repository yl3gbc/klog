#include "solarindicator.h"
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QXmlStreamReader>

SolarIndicator::SolarIndicator(QWidget *parent) : QLabel(parent)
{
    setTextFormat(Qt::RichText);
    setTextFormat(Qt::RichText);
    setText(QStringLiteral("SFI -- A -- K --"));
    setStyleSheet(QStringLiteral(
        "QLabel { font-family: monospace; font-size: 11px; color: #555;"
        " background: #fafafa; padding: 2px 10px;"
        " border: 1px solid #d0d0d0; border-radius: 10px; }"));
    setToolTip(tr("Solar flux, A and K index. Source: hamqsl.com"));
    setMargin(4);
    connect(&timer, &QTimer::timeout, this, &SolarIndicator::refresh);
    timer.start(60 * 60 * 1000);
    QTimer::singleShot(3000, this, &SolarIndicator::refresh);
}

void SolarIndicator::refresh()
{
    QNetworkRequest req(QUrl(QStringLiteral("https://www.hamqsl.com/solarxml.php")));
    req.setHeader(QNetworkRequest::UserAgentHeader, QStringLiteral("KLogNG"));
    QNetworkReply *r = net.get(req);
    connect(r, &QNetworkReply::finished, this, &SolarIndicator::onReply);
}

void SolarIndicator::onReply()
{
    QNetworkReply *r = qobject_cast<QNetworkReply *>(sender());
    if (!r) return;
    r->deleteLater();
    if (r->error() != QNetworkReply::NoError) return;

    QString sfi, a, k;
    QXmlStreamReader xml(r->readAll());
    while (!xml.atEnd())
    {
        if (xml.readNextStartElement())
        {
            const QString n = xml.name().toString();
            if (n == QLatin1String("solarflux"))   sfi = xml.readElementText().trimmed();
            else if (n == QLatin1String("aindex")) a = xml.readElementText().trimmed();
            else if (n == QLatin1String("kindex")) k = xml.readElementText().trimmed();
        }
    }
    if (sfi.isEmpty()) return;
    setText(QStringLiteral(
        "SFI <b style='color:#222'>%1</b>&nbsp; A <b style='color:#222'>%2</b>"
        "&nbsp; K <b style='color:#222'>%3</b>").arg(sfi, a, k));
}
