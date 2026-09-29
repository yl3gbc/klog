#include "contestdef.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

static QStringList strList(const QJsonValue &v)
{
    QStringList out;
    for (const QJsonValue &x : v.toArray()) out << x.toString();
    return out;
}

bool ContestDef::load(const QString &path)
{
    QFile f(path);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        err = QStringLiteral("cannot open %1").arg(path);
        return false;
    }
    QJsonParseError pe;
    const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &pe);
    f.close();
    if (pe.error != QJsonParseError::NoError || !doc.isObject())
    {
        err = QStringLiteral("JSON error: %1").arg(pe.errorString());
        return false;
    }
    const QJsonObject o = doc.object();

    id           = o.value("id").toString();
    name         = o.value("name").toString();
    date         = o.value("date").toString();
    timeZone     = o.value("time_zone").toString("local");
    exportFormat = o.value("export").toString();
    bands        = strList(o.value("bands"));
    exchSent     = strList(o.value("exchange_sent"));
    exchRcvd     = strList(o.value("exchange_rcvd"));
    multiplier   = o.value("multiplier").toString();
    multPerRound = o.value("multiplier_per_round").toBool(false);
    ownDistrictNoMult = o.value("own_district_no_mult").toBool(false);
    dupeScope    = o.value("dupe_scope").toString();
    pointsPerQSO = o.value("points_per_qso").toInt(1);
    maxPowerW    = o.value("max_power_w").toInt(0);

    rounds.clear();
    for (const QJsonValue &rv : o.value("rounds").toArray())
    {
        const QJsonObject ro = rv.toObject();
        ContestRound r;
        r.n     = ro.value("n").toInt();
        r.start = QTime::fromString(ro.value("start").toString(), QStringLiteral("HH:mm"));
        r.end   = QTime::fromString(ro.value("end").toString(), QStringLiteral("HH:mm"));
        r.modes = strList(ro.value("modes"));
        r.exchSent   = ro.contains("exchange_sent") ? strList(ro.value("exchange_sent")) : exchSent;
        r.exchRcvd   = ro.contains("exchange_rcvd") ? strList(ro.value("exchange_rcvd")) : exchRcvd;
        r.multiplier = ro.contains("multiplier") ? ro.value("multiplier").toString() : multiplier;
        rounds.append(r);
    }
    return !id.isEmpty() && !rounds.isEmpty();
}

int ContestDef::roundAt(const QTime &t) const
{
    for (const ContestRound &r : rounds)
        if (r.start.isValid() && r.end.isValid() && t >= r.start && t <= r.end)
            return r.n;
    return 0;
}

const ContestRound *ContestDef::round(int n) const
{
    for (const ContestRound &r : rounds)
        if (r.n == n) return &r;
    return nullptr;
}
