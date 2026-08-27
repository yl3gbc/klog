#include "startprofiledialog.h"

#include <QTableWidget>
#include <QHeaderView>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QSettings>
#include <QLineEdit>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QApplication>
#include <QStyleFactory>

static void forceLightPalette(QWidget *w)
{
    QPalette p;
    p.setColor(QPalette::Window,          QColor(0xf0,0xf0,0xf0));
    p.setColor(QPalette::WindowText,      Qt::black);
    p.setColor(QPalette::Base,            Qt::white);
    p.setColor(QPalette::AlternateBase,   QColor(0xe9,0xe9,0xe9));
    p.setColor(QPalette::Text,            Qt::black);
    p.setColor(QPalette::Button,          QColor(0xf0,0xf0,0xf0));
    p.setColor(QPalette::ButtonText,      Qt::black);
    p.setColor(QPalette::Highlight,       QColor(0x30,0x8c,0xc6));
    p.setColor(QPalette::HighlightedText, Qt::white);
    p.setColor(QPalette::ToolTipBase,     Qt::white);
    p.setColor(QPalette::ToolTipText,     Qt::black);
    w->setPalette(p);
}

static const char *SETT_OPEN_LAST   = "profiles/openLastOnStart";
static const char *SETT_LAST_PROFILE= "profiles/lastProfileId";

#include <QDir>
// Tas pats ini fails, ko lieto KLog (Utilities::getCfgFile ekvivalents)
static QString klogngCfgFile()
{
    return QDir::homePath() + QStringLiteral("/.klogng/klogrc");
}

StartProfileDialog::StartProfileDialog(ProfileManager *pm_, QWidget *parent)
    : QDialog(parent), pm(pm_)
{
    setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    forceLightPalette(this);
    setWindowTitle(tr("Profila izvele"));
    setModal(true);
    resize(420, 380);

    table = new QTableWidget(this);
    table->setColumnCount(3);
    table->setHorizontalHeaderLabels({tr("Zime"), tr("QSO"), tr("Piezime")});
    table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Stretch);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    connect(table, &QTableWidget::cellDoubleClicked,
            this, [this](int, int){ openSelected(); });

    openBtn   = new QPushButton(tr("Atvert"), this);
    auto *newBtn = new QPushButton(tr("Jauns profils"), this);
    editBtn   = new QPushButton(tr("Redigert"), this);
    deleteBtn = new QPushButton(tr("Dzest"), this);
    auto *cancelBtn = new QPushButton(tr("Atcelt"), this);
    openBtn->setDefault(true);

    connect(openBtn,   &QPushButton::clicked, this, &StartProfileDialog::openSelected);
    connect(newBtn,    &QPushButton::clicked, this, &StartProfileDialog::newProfile);
    connect(editBtn,   &QPushButton::clicked, this, &StartProfileDialog::editSelected);
    connect(deleteBtn, &QPushButton::clicked, this, &StartProfileDialog::deleteSelected);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    openLastCheck = new QCheckBox(tr("Nakamreiz atvert so profilu automatiski"), this);
    QSettings sett(klogngCfgFile(), QSettings::IniFormat);
    openLastCheck->setChecked(sett.value(QLatin1String(SETT_OPEN_LAST), false).toBool());

    auto *btnCol = new QVBoxLayout;
    btnCol->addWidget(openBtn);
    btnCol->addWidget(cancelBtn);
    btnCol->addSpacing(16);
    btnCol->addWidget(newBtn);
    btnCol->addWidget(editBtn);
    btnCol->addWidget(deleteBtn);
    btnCol->addStretch();

    auto *mid = new QHBoxLayout;
    mid->addWidget(table, 1);
    mid->addLayout(btnCol);

    auto *root = new QVBoxLayout(this);
    root->addLayout(mid, 1);
    root->addWidget(openLastCheck);

    reload();
}

void StartProfileDialog::reload()
{
    const QList<Profile> profiles = pm->listProfiles();
    table->setRowCount(profiles.size());
    int row = 0;
    for (const Profile &p : profiles) {
        auto *c0 = new QTableWidgetItem(p.callsign);
        c0->setData(Qt::UserRole, p.id);
        table->setItem(row, 0, c0);
        table->setItem(row, 1, new QTableWidgetItem(QString::number(p.qsoCount)));
        table->setItem(row, 2, new QTableWidgetItem(p.comment));
        ++row;
    }
    const bool any = !profiles.isEmpty();
    openBtn->setEnabled(any);
    editBtn->setEnabled(any);
    deleteBtn->setEnabled(any);
    if (any)
        table->selectRow(0);
}

int StartProfileDialog::currentRowProfileId() const
{
    const int row = table->currentRow();
    if (row < 0) return -1;
    return table->item(row, 0)->data(Qt::UserRole).toInt();
}

void StartProfileDialog::openSelected()
{
    const int id = currentRowProfileId();
    if (id < 0) return;
    selectedId = id;
    QSettings sett(klogngCfgFile(), QSettings::IniFormat);
    sett.setValue(QLatin1String(SETT_OPEN_LAST), openLastCheck->isChecked());
    sett.setValue(QLatin1String(SETT_LAST_PROFILE), id);
    accept();
}

static bool editProfileDialog(QWidget *parent, Profile &p, const QString &title)
{
    QDialog d(parent);
    d.setWindowTitle(title);
    QFormLayout form(&d);
    QLineEdit call(p.callsign), name(p.operatorName),
              grid(p.gridsquare), comment(p.comment);
    form.addRow(QObject::tr("Izsaukuma zime:"), &call);
    form.addRow(QObject::tr("Operatora vards:"), &name);
    form.addRow(QObject::tr("Lokators:"), &grid);
    form.addRow(QObject::tr("Piezime:"), &comment);
    QDialogButtonBox box(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    form.addRow(&box);
    QObject::connect(&box, &QDialogButtonBox::accepted, &d, &QDialog::accept);
    QObject::connect(&box, &QDialogButtonBox::rejected, &d, &QDialog::reject);
    if (d.exec() != QDialog::Accepted)
        return false;
    if (call.text().trimmed().isEmpty())
        return false;
    p.callsign = call.text().trimmed().toUpper();
    p.operatorName = name.text();
    p.gridsquare = grid.text().trimmed().toUpper();
    p.comment = comment.text();
    return true;
}

void StartProfileDialog::newProfile()
{
    Profile p;
    if (!editProfileDialog(this, p, tr("Jauns profils")))
        return;
    if (pm->createProfile(p) < 0)
        QMessageBox::warning(this, tr("Kluda"),
            tr("Profilu neizdevas izveidot. Vai zime %1 jau eksiste?").arg(p.callsign));
    reload();
}

void StartProfileDialog::editSelected()
{
    const int id = currentRowProfileId();
    if (id < 0) return;
    Profile p = pm->getProfile(id);
    if (!editProfileDialog(this, p, tr("Rediget profilu %1").arg(p.callsign)))
        return;
    pm->updateProfile(p);
    reload();
}

void StartProfileDialog::deleteSelected()
{
    const int id = currentRowProfileId();
    if (id < 0) return;
    const Profile p = pm->getProfile(id);
    const int n = pm->qsoCount(id);
    if (QMessageBox::question(this, tr("Dzest profilu?"),
            tr("Dzest profilu %1 (%2 QSO)?").arg(p.callsign).arg(n))
        != QMessageBox::Yes)
        return;
    QString err;
    if (!pm->deleteProfile(id, &err))
        QMessageBox::warning(this, tr("Nevar dzest"), err);
    reload();
}

#include <QSqlQuery>
#include <QSqlDatabase>
#include <QVariant>
#include <QDate>

static int lognumberForProfile(ProfileManager *pm, int profileId)
{
    const Profile p = pm->getProfile(profileId);
    if (p.id < 0) return -1;
    QSqlQuery q(QSqlDatabase::database());
    q.prepare(QStringLiteral(
        "SELECT id FROM logs WHERE upper(trim(stationcall)) = :c "
        "ORDER BY id LIMIT 1"));
    q.bindValue(QStringLiteral(":c"), p.callsign);
    if (q.exec() && q.next())
        return q.value(0).toInt();
    // logs ieraksta nav (jauns profils) - izveidojam
    q.prepare(QStringLiteral(
        "INSERT INTO logs (logdate, stationcall, logtype, logtypen) "
        "VALUES (:d, :c, 'DX', 1)"));
    q.bindValue(QStringLiteral(":d"), QDate::currentDate().toString(QStringLiteral("yyyy-MM-dd")));
    q.bindValue(QStringLiteral(":c"), p.callsign);
    if (q.exec())
        return q.lastInsertId().toInt();
    return -1;
}

int StartProfileDialog::chooseProfileOnStartup(ProfileManager *pm, QWidget *parent)
{
    qApp->setStyle(QStyleFactory::create(QStringLiteral("Fusion")));
    QPalette lp;
    lp.setColor(QPalette::Window,          QColor(0xf0,0xf0,0xf0));
    lp.setColor(QPalette::WindowText,      Qt::black);
    lp.setColor(QPalette::Base,            Qt::white);
    lp.setColor(QPalette::AlternateBase,   QColor(0xe9,0xe9,0xe9));
    lp.setColor(QPalette::Text,            Qt::black);
    lp.setColor(QPalette::Button,          QColor(0xf0,0xf0,0xf0));
    lp.setColor(QPalette::ButtonText,      Qt::black);
    lp.setColor(QPalette::Highlight,       QColor(0x30,0x8c,0xc6));
    lp.setColor(QPalette::HighlightedText, Qt::white);
    lp.setColor(QPalette::ToolTipBase,     Qt::white);
    lp.setColor(QPalette::ToolTipText,     Qt::black);
    lp.setColor(QPalette::PlaceholderText, QColor(0x80,0x80,0x80));
    qApp->setPalette(lp);

    QSettings sett(klogngCfgFile(), QSettings::IniFormat);
    if (sett.value(QLatin1String(SETT_OPEN_LAST), false).toBool()) {
        const int last = sett.value(QLatin1String(SETT_LAST_PROFILE), -1).toInt();
        if (last > 0 && pm->getProfile(last).id == last)
        {
            const int ln = lognumberForProfile(pm, last);
            if (ln > 0)
            {
                QSettings s2(klogngCfgFile(), QSettings::IniFormat);
                s2.setValue(QStringLiteral("SelectedLog"), ln);
                const Profile ap = pm->getProfile(last);
                if (!ap.callsign.isEmpty())
                    s2.setValue(QStringLiteral("Callsign"), ap.callsign);
                if (!ap.gridsquare.isEmpty())
                    s2.setValue(QStringLiteral("StationLocator"), ap.gridsquare);
                if (!ap.operatorName.isEmpty())
                    s2.setValue(QStringLiteral("Operators"), ap.operatorName);
                s2.sync();
            }
            return last;
        }
    }
    StartProfileDialog dlg(pm, parent);
    if (dlg.exec() == QDialog::Accepted)
    {
        const int ln = lognumberForProfile(pm, dlg.selectedProfileId());
        if (ln > 0)
        {
            QSettings s2(klogngCfgFile(), QSettings::IniFormat);
            s2.setValue(QStringLiteral("SelectedLog"), ln);
            const Profile ap = pm->getProfile(dlg.selectedProfileId());
            if (!ap.callsign.isEmpty())
                s2.setValue(QStringLiteral("Callsign"), ap.callsign);
            if (!ap.gridsquare.isEmpty())
                s2.setValue(QStringLiteral("StationLocator"), ap.gridsquare);
            if (!ap.operatorName.isEmpty())
                s2.setValue(QStringLiteral("Operators"), ap.operatorName);
            s2.sync();
        }
        return dlg.selectedProfileId();
    }
    return -1;
}
