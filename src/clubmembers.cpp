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
            if (line.isEmpty()) continue;
            const int sep = line.indexOf(QLatin1Char(';'));
            if (sep < 0)
                c.members.insert(line.toUpper(), QString());        // tikai zime
            else
            {
                // zime;numurs vai zime;numurs;no;lidz - nemam tikai numuru
                QString rest = line.mid(sep + 1);
                const int sep2 = rest.indexOf(QLatin1Char(';'));
                if (sep2 >= 0) rest = rest.left(sep2);
                c.members.insert(line.left(sep).toUpper(), rest.trimmed());
            }
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
        if (!cl.members.contains(c) && !cl.members.contains(base)) continue;
        const QString num = cl.members.contains(c) ? cl.members.value(c)
                                                   : cl.members.value(base);
        out << (num.isEmpty() ? cl.shortName
                              : QStringLiteral("%1 #%2").arg(cl.shortName, num));
    }
    return out;
}
