#include "clubmembers.h"
#include <QDir>
#include <QFile>
#include <QTextStream>
#include "callsignservices.h"

ClubMembers::ClubMembers(const QString &dataDir, QObject *parent)
    : QObject(parent), dir(dataDir)
{
    reload();
}

void ClubMembers::reload()
{
    clubs.clear();
    QDir d(dir);
    const QStringList files = d.entryList(QStringList() << QStringLiteral("*.txt"), QDir::Files);
    for (const QString &fn : files)
    {
        QFile f(d.filePath(fn));
        if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) continue;
        QTextStream in(&f);
        Club c;
        c.shortName = in.readLine().trimmed();
        c.fullName  = in.readLine().trimmed();
        while (!in.atEnd())
        {
            const QString line = in.readLine().trimmed();
            const int sep = line.indexOf(QLatin1Char(';'));
            if (sep > 0)
                c.members.insert(line.left(sep).toUpper(), line.mid(sep + 1).trimmed());
        }
        f.close();
        if (!c.shortName.isEmpty() && !c.members.isEmpty())
            clubs.append(c);
    }
}

QStringList ClubMembers::lookup(const QString &call) const
{
    QStringList out;
    const QString c = call.trimmed().toUpper();
    if (c.length() < 3) return out;
    const QString base = CallsignServices::baseCall(c);
    for (const Club &cl : clubs)
    {
        QString num = cl.members.value(c);
        if (num.isEmpty()) num = cl.members.value(base);
        if (!num.isEmpty())
            out << QStringLiteral("%1 #%2").arg(cl.shortName, num);
    }
    return out;
}
