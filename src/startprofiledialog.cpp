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

static const char *SETT_OPEN_LAST   = "profiles/openLastOnStart";
static const char *SETT_LAST_PROFILE= "profiles/lastProfileId";

StartProfileDialog::StartProfileDialog(ProfileManager *pm_, QWidget *parent)
    : QDialog(parent), pm(pm_)
{
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
    QSettings sett;
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
    QSettings sett;
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

int StartProfileDialog::chooseProfileOnStartup(ProfileManager *pm, QWidget *parent)
{
    QSettings sett;
    if (sett.value(QLatin1String(SETT_OPEN_LAST), false).toBool()) {
        const int last = sett.value(QLatin1String(SETT_LAST_PROFILE), -1).toInt();
        if (last > 0 && pm->getProfile(last).id == last)
            return last;
    }
    StartProfileDialog dlg(pm, parent);
    if (dlg.exec() == QDialog::Accepted)
        return dlg.selectedProfileId();
    return -1;
}
