#ifndef KLOGNG_LOGUPDATER_H
#define KLOGNG_LOGUPDATER_H

#include <QObject>
#include <QStringList>
#include <QTimer>

class QRZComLookup;
class HamQTH;

class LogUpdater : public QObject
{
    Q_OBJECT
public:
    explicit LogUpdater(QRZComLookup *qrz, HamQTH *hq, QObject *parent = nullptr);
    void start(const QStringList &calls);
    void stop();
    bool isRunning() const { return timer.isActive(); }

signals:
    void progress(int done, int total, int updated);
    void finished(int updated);

private slots:
    void next();
    void onQRZ(const QString &call, const QString &name,
               const QString &qth, const QString &grid);
    void onDOK(const QString &call, const QString &dok, const QString &,
               const QString &, const QString &);

private:
    QRZComLookup *qrz = nullptr;
    HamQTH *hamqth = nullptr;
    QStringList queue;
    QTimer timer;
    int total = 0, done = 0, updated = 0;
};

#endif
