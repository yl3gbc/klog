#ifndef KLOGNG_WSJTXLOGWATCHER_H
#define KLOGNG_WSJTXLOGWATCHER_H

#include <QObject>
#include <QFileSystemWatcher>
#include <QTimer>
#include <QString>

class DataProxy_SQLite;

// Seko WSJT-X/JTDX zurnala failam un pievieno QSO, kuru bazee nav.
// Ta QSO nonak zurnala neatkarigi no programmu palaisanas secibas:
// ja UDP strada, tie nak pa to; ja ne, tos panem no faila.
class WSJTXLogWatcher : public QObject
{
    Q_OBJECT
public:
    explicit WSJTXLogWatcher(DataProxy_SQLite *dp, QObject *parent = nullptr);
    void setLogFile(const QString &path);
    void checkNow();

signals:
    void qsosImported(int count);

private slots:
    void onFileChanged(const QString &path);
    void doCheck();

private:
    DataProxy_SQLite *dataProxy = nullptr;
    QFileSystemWatcher watcher;
    QTimer debounce;
    QString file;
};

#endif
