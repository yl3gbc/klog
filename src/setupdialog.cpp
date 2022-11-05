/***************************************************************************
                          setupdialog.cpp  -  description
                             -------------------
    begin                : sept 2011
    copyright            : (C) 2011 by Jaime Robles
    email                : jaime@robles.es
 ***************************************************************************/

/*****************************************************************************
 * This file is part of KLog.                                                *
 *                                                                           *
 *    KLog is free software: you can redistribute it and/or modify           *
 *    it under the terms of the GNU General Public License as published by   *
 *    the Free Software Foundation, either version 3 of the License, or      *
 *    (at your option) any later version.                                    *
 *                                                                           *
 *    KLog is distributed in the hope that it will be useful,                *
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *    GNU General Public License for more details.                           *
 *                                                                           *
 *    You should have received a copy of the GNU General Public License      *
 *    along with KLog.  If not, see <https://www.gnu.org/licenses/>.         *
 *                                                                           *
 *****************************************************************************/

#include "setupdialog.h"
//#include <QDebug>

/*
This class calls all the othet "Setup..." to manage the configuration

*/

SetupDialog::SetupDialog(DataProxy_SQLite *dp, QWidget *parent)
{
    //qDebug() << Q_FUNC_INFO << ": " << _configFile << "/" << _softwareVersion << "/" << QString::number(_page) << util->boolToQString(_firstTime);

    logLevel = None;
    constrid = 2;
    util = new Utilities(Q_FUNC_INFO);
    firstTime = true;
    latestBackup = QString();
    dataProxy = dp;

    configFileName = QString();
    version = QString();
    pageRequested = 0;

    int logsPageTabN=-1;
    //qDebug() << Q_FUNC_INFO << ": 01" << QT_ENDL;

    locator = new Locator();

    tabWidget = new QTabWidget;
    //qDebug() << Q_FUNC_INFO << ": 01.0" << QT_ENDL;
    userDataPage = new SetupPageUserDataPage(dataProxy);
    //qDebug() << Q_FUNC_INFO << ": 01.10" << QT_ENDL;
    bandModePage = new SetupPageBandMode(dataProxy, this);
    //qDebug() << Q_FUNC_INFO << ": 01.20" << QT_ENDL;
    dxClusterPage = new SetupPageDxCluster(this);
    //qDebug() << Q_FUNC_INFO << ": 01.30" << QT_ENDL;
    colorsPage = new SetupPageColors(this);
    //qDebug() << Q_FUNC_INFO << ": 01.40" << QT_ENDL;
    miscPage = new SetupPageMisc(this);
    //qDebug() << Q_FUNC_INFO << ": 01.50" << QT_ENDL;
    worldEditorPage = new SetupPageWorldEditor (dataProxy, this);
    //qDebug() << Q_FUNC_INFO << ": 01.60" << QT_ENDL;
    logsPage = new SetupPageLogs(dataProxy, this);
    //qDebug() << Q_FUNC_INFO << ": 01.70" << QT_ENDL;
    eLogPage = new SetupPageELog(this);
    //qDebug() << Q_FUNC_INFO << ": 01.80" << QT_ENDL;
    UDPPage = new SetupPageUDP(this);
    //qDebug() << Q_FUNC_INFO << ": 01.90" << QT_ENDL;
    satsPage = new SetupPageSats(dataProxy, this);
    //qDebug() << Q_FUNC_INFO << ": 01.100" << QT_ENDL;
    hamlibPage = new SetupPageHamLib(dataProxy, this);
    //qDebug() << Q_FUNC_INFO << ": 01.101" << QT_ENDL;
    logViewPage = new SetupPageLogView(dataProxy, this);
    //qDebug() << Q_FUNC_INFO << ": 02" << QT_ENDL;

    tabWidget->addTab(userDataPage, tr("User data"));
    tabWidget->addTab(bandModePage, tr("Bands/Modes"));
    tabWidget->addTab(logViewPage, tr("Log widget"));
    tabWidget->addTab(dxClusterPage, tr("D&X-Cluster"));
    tabWidget->addTab(colorsPage, tr("Colors"));
    tabWidget->addTab(miscPage, tr("Misc"));
    tabWidget->addTab(worldEditorPage, tr("World Editor"));
    logsPageTabN = tabWidget->addTab(logsPage, tr("Logs"));
    tabWidget->addTab(eLogPage, tr("eLog"));
    tabWidget->addTab(UDPPage, tr("WSJT-X"));
    tabWidget->addTab(satsPage , tr("Satellites"));
    //qDebug() << Q_FUNC_INFO << ": 02.100" << QT_ENDL;
    tabWidget->addTab(hamlibPage, tr ("HamLib"));
    //qDebug() << "SetupDialog::SetupDialog 03" << QT_ENDL;

    closeButton = new QPushButton(tr("Cancel"));
    okButton = new QPushButton(tr("OK"));

    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(tabWidget);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(okButton);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addLayout(buttonsLayout);

    //qDebug() << Q_FUNC_INFO << ": 04" << QT_ENDL;

    setLayout(mainLayout);
    setWindowTitle(tr("Settings"));


    connectActions();
    //qDebug() << Q_FUNC_INFO << " - END" << QT_ENDL;
}

void SetupDialog::init(const QString &_configFile, const QString &_softwareVersion, const int _page, const bool _firstTime)
{
    util->setLongPrefixes(dataProxy->getLongPrefixes());
    util->setSpecialCalls(dataProxy->getSpecialCallsigns());
    firstTime = _firstTime;
    configFileName = _configFile;
    version = _softwareVersion;
    pageRequested = _page;

    slotReadConfigData();
    //qDebug() << Q_FUNC_INFO << ": 05.1" << QT_ENDL;

    if ((pageRequested==6) && (logsPageTabN>0))// The user is opening a new log
    {
        //qDebug() << Q_FUNC_INFO << ": 5.2" << QT_ENDL;
        tabWidget->setCurrentIndex(logsPageTabN);
    }
    //qDebug() << Q_FUNC_INFO << ": 5.3" << QT_ENDL;
    nolog = !(haveAtleastOneLog());
    connect(closeButton, SIGNAL(clicked()), this, SLOT(slotCancelButtonClicked()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(slotOkButtonClicked()));
    connectActions();
    //qDebug() << Q_FUNC_INFO << " - END" << QT_ENDL;

}

SetupDialog::~SetupDialog()
{
    //qDebug() << Q_FUNC_INFO  << QT_ENDL;
    delete(locator);
    delete(userDataPage);
    delete(bandModePage);
    delete(dxClusterPage);
    delete(miscPage);
    delete(worldEditorPage);
    delete(logsPage);
    delete(eLogPage);
    delete(colorsPage);
    delete(UDPPage);
    delete(satsPage);
    delete(hamlibPage);
    delete(logViewPage);
    delete(util);
}

void SetupDialog::connectActions()
{
    logEvent(Q_FUNC_INFO, "Start", Debug);
    connect(closeButton, SIGNAL(clicked()), this, SLOT(slotCancelButtonClicked()));
    connect(okButton, SIGNAL(clicked()), this, SLOT(slotOkButtonClicked()));
    connect (logsPage, SIGNAL(newLogData(QStringList)), this, SLOT(slotAnalyzeNewLogData(QStringList)));
    connect(logsPage, SIGNAL(focusOK()), this, SLOT(slotFocusOK()) );
    connect (userDataPage, SIGNAL(mainCallsignSignal(QString)), this, SLOT(slotSetStationCallSign(QString)));
    connect (userDataPage, SIGNAL(operatorsSignal(QString)), this, SLOT(slotSetOperators(QString)));
    connect (userDataPage, SIGNAL(enterKey()), this, SLOT(slotOkButtonClicked()));
    //connect (lotwPage, SIGNAL(enterKey()), this, SLOT(slotOkButtonClicked()));
    connect (eLogPage, SIGNAL(enterKey()), this, SLOT(slotOkButtonClicked()));
    //connect (clubLogPage, SIGNAL(enterKey()), this, SLOT(slotOkButtonClicked()));
    connect (eLogPage, SIGNAL(qrzcomAuto(bool)), this, SLOT(slotQRZCOMAuto(bool)));

    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::slotQRZCOMAuto(const bool _b)
{
    emit qrzcomAuto(_b);
}

void SetupDialog::setData(const QString &_configFile, const QString &_softwareVersion, const int _page, const bool _firstTime)
{
      //qDebug() << "SetupDialog::setData: " << "/" << _configFile << "/" << _softwareVersion << "/" << QString::number(_page) << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    nolog = !(haveAtleastOneLog());
    firstTime = _firstTime;
    if (firstTime)
    {
          //qDebug() << "SetupDialog::setData FIRST TIME! " << QT_ENDL;
    }
    else
    {
          //qDebug() << "SetupDialog::setData NOT FIRST TIME! " << QT_ENDL;
        miscPage->setUseDefaultDBPath(miscPage->getDefaultDBPath());
    }

    setConfigFile(_configFile);
    setSoftVersion(_softwareVersion);
    slotReadConfigData ();
    setPage(_page);
    //removeBandModeDuplicates();
    logEvent(Q_FUNC_INFO, "END", Debug);
      //qDebug() << "SetupDialog::setData - END" << QT_ENDL;
}

void SetupDialog::setConfigFile(const QString &_configFile)
{
       //qDebug() << "SetupDialog::setConfigFile" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    configFileName = _configFile;
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::setSoftVersion(const QString &_softwareVersion)
{
       //qDebug() << "SetupDialog::setSoftVersion" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    version = _softwareVersion;
    logEvent(Q_FUNC_INFO, "END", Debug);
}


void SetupDialog::setPage(const int _page)
{
       //qDebug() << "SetupDialog::setPage("<<QString::number(_page) << ")" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    pageRequested = _page;

    if ((pageRequested==6) && (logsPageTabN>0))// The user is opening a new log
    {
        tabWidget->setCurrentIndex(pageRequested);
    }
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::slotCancelButtonClicked()
{
      //qDebug() << "SetupDialog::slotCancelButtonClicked" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    if (firstTime || nolog)
    {
        if (nolog)
        {
            QMessageBox msgBox;
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            msgBox.setDefaultButton(QMessageBox::Yes);
            msgBox.setText(tr("You need to enter at least one log in the Logs tab."));
            msgBox.setInformativeText(tr("Do you want to add one log in the Logs tab or exit KLog?\n(Click Yes to add a log or No to exit KLog)"));
            int ret = msgBox.exec();
            if (ret == QMessageBox::No)
            {
                emit debugLog (Q_FUNC_INFO, "END-1", logLevel);
                emit exitSignal(2);
                return;
            }
            else
            {
                emit debugLog (Q_FUNC_INFO, "END-2", logLevel);
                return;
            }
        }
    }
    hamlibPage->stopHamlib();
    QDialog::reject ();
    close();
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::createIcons()
{
       //qDebug() << "SetupDialog::createIcons" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    QListWidgetItem *configButton = new QListWidgetItem(contentsWidget);
    configButton->setIcon(QIcon(":/images/config.png"));
    configButton->setText(tr("User data"));
    configButton->setTextAlignment(Qt::AlignHCenter);
    configButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *logsButton = new QListWidgetItem(contentsWidget);
    logsButton->setIcon(QIcon(":/images/config.png"));
    logsButton->setText(tr("Logs"));
    logsButton->setTextAlignment(Qt::AlignHCenter);
    logsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *bandsButton = new QListWidgetItem(contentsWidget);
    bandsButton->setIcon(QIcon(":/images/query.png"));
    bandsButton->setText(tr("Bands/Modes"));
    bandsButton->setTextAlignment(Qt::AlignHCenter);
    bandsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *dxClusterButton = new QListWidgetItem(contentsWidget);
    dxClusterButton->setIcon(QIcon(":/images/query.png"));
    dxClusterButton->setText(tr("DX-Cluster"));
    dxClusterButton->setTextAlignment(Qt::AlignHCenter);
    dxClusterButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *colorsButton  = new QListWidgetItem(contentsWidget);
    colorsButton->setIcon(QIcon(":/images/query.png"));
    colorsButton->setText(tr("Colors"));
    colorsButton->setTextAlignment(Qt::AlignHCenter);
    colorsButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *miscButton  = new QListWidgetItem(contentsWidget);
    miscButton->setIcon(QIcon(":/images/query.png"));
    miscButton->setText(tr("Misc"));
    miscButton->setTextAlignment(Qt::AlignHCenter);
    miscButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *worldButton  = new QListWidgetItem(contentsWidget);
    worldButton->setIcon(QIcon(":/images/query.png"));
    worldButton->setText(tr("World"));
    worldButton->setTextAlignment(Qt::AlignHCenter);
    worldButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect(contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)),
            this, SLOT(changePage(QListWidgetItem*,QListWidgetItem*)));
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
       //qDebug() << "SetupDialog::changePage" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
    logEvent(Q_FUNC_INFO, "END", Debug);
}



void SetupDialog::slotOkButtonClicked()
{
    //qDebug() << "SetupDialog::slotOkButtonClicked" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);

    if (!miscPage->areDBPathChangesApplied())
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(tr("DB has not been moved to new path."));
        msgBox.setInformativeText(tr("Go to the Misc tab and click on Move DB\n or the DB will not be moved to the new location."));
        msgBox.exec();
        emit debugLog (Q_FUNC_INFO, "END-1", logLevel);
        return;
    }

    if (!util->isValidCall(userDataPage->getMainCallsign())){ //
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("You need to enter at least a valid callsign."));
        msgBox.setInformativeText(tr("Go to the User tab and enter valid callsign."));
        msgBox.exec();
        emit debugLog (Q_FUNC_INFO, "END-2", logLevel);
        return;
    }

    if (!haveAtleastOneLog())
    {
        //qDebug() << "SetupDialog::slotOkButtonClicked - NO LOG!" << QT_ENDL;
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Information);
        msgBox.setText(tr("You have not selected the kind of log you want."));
        msgBox.setInformativeText(tr("You will be redirected to the Log tab.\nPlease add and select the kind of log you want to use."));
        msgBox.exec();

        tabWidget->setCurrentIndex(tabWidget->indexOf(logsPage));
        logsPage->createNewLog();
        //emit newLogRequested(true); // Signal to be catched by logsPage true show new log

        emit debugLog (Q_FUNC_INFO, "END-3", logLevel);
        return;
    }
    //qDebug() << "SetupDialog::slotOkButtonClicked - 10" << QT_ENDL;

    QSettings settings(configFileName, QSettings::IniFormat);

        //QRZ/CQ/ITU/CONTEST
    settings.setValue("version", version);
    settings.setValue("callsign", userDataPage->getMainCallsign());

    //stream << "Callsign="  << userDataPage->getMainCallsign() << ";" << QT_ENDL;
    if ((userDataPage->getOperators()).length() >= 3)
    { // There are no valid calls with less than 3 Chars
            settings.setValue("operators", userDataPage->getOperators());
    }

    settings.setValue("cqz", userDataPage->getCQz());
    settings.setValue("ituz", userDataPage->getITUz() );

    if ( locator->isValidLocator(userDataPage->getStationLocator()) )
    {
        settings.setValue("stationlocator", userDataPage->getStationLocator());
    }

    if ((!(userDataPage->getName()).isNull()) && (  (userDataPage->getName()).length() > 0   ))
    {
        settings.setValue("name", userDataPage->getName());
    }
    if ((!(userDataPage->getAddress1()).isNull()) && (  (userDataPage->getAddress1()).length() > 0   ))
    {
        settings.setValue("address1", userDataPage->getAddress1());
        }
        if ((!(userDataPage->getAddress2()).isNull())  && (  (userDataPage->getAddress2()).length() > 0   ))
        {
            settings.setValue("address2", userDataPage->getAddress2());
        }
        if ((!(userDataPage->getAddress3()).isNull()) && (  (userDataPage->getAddress3()).length() > 0   ))
        {
            settings.setValue("address3", userDataPage->getAddress3());
        }
        if ((!(userDataPage->getAddress4()).isNull()) && (  (userDataPage->getAddress4()).length() > 0   ))
        {
            settings.setValue("address4", userDataPage->getAddress4());
        }

        if ((!(userDataPage->getCity()).isNull()) && (  (userDataPage->getCity()).length() > 0   ))
        {
            settings.setValue("city", userDataPage->getCity());
        }
        if ((!(userDataPage->getZipCode()).isNull()) && (  (userDataPage->getZipCode()).length() > 0   ))
        {
            settings.setValue("zipcode", userDataPage->getZipCode());
        }
        if ((!(userDataPage->getProvince()).isNull()) && (  (userDataPage->getProvince()).length() > 0   ))
        {
            settings.setValue("provincestate", userDataPage->getProvince());
        }
        if ((!(userDataPage->getCountry()).isNull()) && (  (userDataPage->getCountry()).length() > 0   ))
        {
            settings.setValue("country", userDataPage->getCountry());
        }
        if ((!(userDataPage->getRig1()).isNull()) && (  (userDataPage->getRig1()).length() > 0   ))
        {
            settings.setValue("rig1", userDataPage->getRig1());
        }
        if ((!(userDataPage->getRig2()).isNull()) && (  (userDataPage->getRig2()).length() > 0   ))
        {
            settings.setValue("rig2", userDataPage->getRig2());
        }
        if ((!(userDataPage->getRig3()).isNull()) && (  (userDataPage->getRig3()).length() > 0   ))
        {
            settings.setValue("rig3", userDataPage->getRig3());
        }
        if ((!(userDataPage->getAntenna1()).isNull()) && (  (userDataPage->getAntenna1()).length() > 0   ))
        {
            settings.setValue("antenna1", userDataPage->getAntenna1());
        }
        if ((!(userDataPage->getAntenna2()).isNull()) && (  (userDataPage->getAntenna2()).length() > 0   ))
        {
            settings.setValue("antenna2", userDataPage->getAntenna2());
        }
        if ((!(userDataPage->getAntenna3()).isNull()) && (  (userDataPage->getAntenna2()).length() > 0   ))
        {
            settings.setValue("antenna3", userDataPage->getAntenna3());
        }
        if ((userDataPage->getPower()).toFloat()>=0)
        {
            settings.setValue("power", userDataPage->getPower());
        }
        //qDebug() << "SetupDialog::slotOkButtonClicked - 20" << QT_ENDL;

        settings.setValue("bands", bandModePage->getBands());
        settings.setValue("modes", bandModePage->getModes());
        settings.setValue("logviewfields", logViewPage->getFields());

        settings.setValue("realtime", miscPage->getRealTime());
        settings.setValue("showseconds", miscPage->getShowSeconds());
        settings.setValue("utctime", miscPage->getUTCTime());


        settings.setValue("alwaysadif", miscPage->getAlwaysADIF());
        settings.setValue("usedefaultname", miscPage->getUseDefaultName());
        settings.setValue("defaultadiffile", miscPage->getDefaultFileName());

        settings.setValue("dbpath", miscPage->getDefaultDBPath());
        settings.setValue("imperialsystem", miscPage->getImperial());
        settings.setValue("sendqslwhenrec", miscPage->getSendQSLWhenRec());


        settings.setValue("showcallsigninsearch", miscPage->getShowStationCallSignInSearch());
        settings.setValue("completewithprevious", miscPage->getCompleteWithPrevious());
        settings.setValue("checknewversions", miscPage->getCheckNewVersions());


        settings.setValue("managedxmarathon", miscPage->getDXMarathon());
        settings.setValue("debuglog", miscPage->getDebugLogLevel());
        settings.setValue("sendeqslbydefault", miscPage->getSendEQSLByDefault());


        settings.setValue("deletealwaysadiFile", miscPage->getDeleteAlwaysAdiFile());
        settings.setValue("checkvalidcalls", miscPage->getCheckCalls());


        if (miscPage->getDupeTime()>0)
        {
            settings.setValue("duplicatedqsoslot", miscPage->getDupeTime());
        }

        //qDebug() << "SetupDialog::slotOkButtonClicked - 30" << QT_ENDL;

        if ( miscPage->getReportInfo())
        {
            settings.setValue("provideinfo", miscPage->getReportInfo());
        }

        if ((!(dxClusterPage->getSelectedDxClusterServer()).isNull()) && (  (dxClusterPage->getSelectedDxClusterServer()).length() > 0   ))
        {
            settings.setValue("dxclusterservertuse", dxClusterPage->getSelectedDxClusterServer());
        }

        QStringList stringList;
        stringList.clear();
        stringList << dxClusterPage->getDxclusterServersComboBox();

        if (stringList.size()>0)
        {
            settings.setValue("dxclusterservers", stringList);
        }

        settings.setValue("dxclustershowhf", dxClusterPage->getShowHFQCheckbox());
        settings.setValue("dxclustershowvhf", dxClusterPage->getShowVHFQCheckbox());
        settings.setValue("dxclustershowwarc", dxClusterPage->getShowWARCQCheckbox());

        settings.setValue("dxclustershowworked", dxClusterPage->getShowWorkedQCheckbox());
        settings.setValue("dxclustershowconfirmed", dxClusterPage->getShowConfirmedQCheckbox());
        settings.setValue("dxclustershowann", dxClusterPage->getShowANNQCheckbox());

        settings.setValue("dxclustershowwwv", dxClusterPage->getShowWWVQCheckbox());
        settings.setValue("dxclustershowwcy", dxClusterPage->getShowWCYQCheckbox());
        settings.setValue("dxclustersave", dxClusterPage->getSaveActivityQCheckbox());
        settings.setValue("dxclustersendtomap", dxClusterPage->getSendSpotsToMap());

        settings.setValue("newonecolor", colorsPage->getNewOneColor());
        settings.setValue("neededcolor", colorsPage->getNeededColor());
        settings.setValue("workedcolor", colorsPage->getWorkedColor());

        settings.setValue("ConfirmedColor", colorsPage->getConfirmedColor());
        settings.setValue("DefaultColor", colorsPage->getDefaultColor());
        settings.setValue("DarkMode", colorsPage->getDarkMode());
        settings.setValue("SelectedLog", logsPage->getSelectedLog());
        // CLUBLOG
        //qDebug() << "SetupDialog::slotOkButtonClicked - 40" << QT_ENDL;

        settings.setValue("clublogactive", eLogPage->getClubLogActive());
        settings.setValue("clublogrealtime", eLogPage->getClubLogRealTime());
        if ((eLogPage->getClubLogEmail()).length()>0)
        {
                settings.setValue("clublogemail", eLogPage->getClubLogEmail());
        }
        if ((eLogPage->getClubLogPassword()).length()>0)
        {
            settings.setValue("clublogpass", eLogPage->getClubLogPassword());
        }

        //qDebug() << "SetupDialog::slotOkButtonClicked - 50" << QT_ENDL;
        // eQSL
        settings.setValue("eqslactive", eLogPage->getEQSLActive());

        if ((eLogPage->getEQSLUser()).length()>0)
        {
            settings.setValue("eqslcall", eLogPage->getEQSLUser());
        }

        if ((eLogPage->getEQSLPassword()).length()>0)
        {
            settings.setValue("eqslpass", eLogPage->getEQSLPassword());
        }

        // eQSL - END
        // QRZ.com

            //qDebug() << "SetupDialog::slotOkButtonClicked - Storing QRZ.com data" << QT_ENDL;
        settings.setValue("qrzcomactive", eLogPage->getQRZCOMActive());

        if ((eLogPage->getQRZCOMUser()).length()>0)
        {
            settings.setValue("qrzcomuser", eLogPage->getQRZCOMUser());
        }

        if ((eLogPage->getQRZCOMPassword()).length()>0)
        {
            settings.setValue("qrzcompass", eLogPage->getQRZCOMPassword());
        }

        settings.setValue("qrzcomsubscriber", eLogPage->getQRZCOMSubscriber());
        settings.setValue("qrzcomauto", eLogPage->getQRZCOMAutoCheck());

        if ((eLogPage->getQRZCOMLogBookKEY()).length()>0)
        {
            settings.setValue("qrzcomlognookkey", eLogPage->getQRZCOMLogBookKEY());
        }
        // QRZ.com - END

        //qDebug() << "SetupDialog::slotOkButtonClicked - 60" << QT_ENDL;

        // LOTW

        settings.setValue("lotwactive", eLogPage->getLoTWActive());
        if ((eLogPage->getTQSLPath()).length()>0)
        {
            settings.setValue("lotwpath", eLogPage->getTQSLPath());
        }
        if ((eLogPage->getLoTWUser()).length()>0)
        {
            settings.setValue("lotwuser", eLogPage->getLoTWUser());
        }
        if ((eLogPage->getLoTWPass()).length()>0)
        {
            settings.setValue("lotwpass", eLogPage->getLoTWPass());
        }


        // LOTW
        //qDebug() << "SetupDialog::slotOkButtonClicked - 70" << QT_ENDL;
        //WSJTX
        settings.setValue("udpserver", UDPPage->getUDPServer() );
        settings.setValue("udpnetworkinterface", UDPPage->getNetworkInterface());
        settings.setValue("udpserverport", UDPPage->getUDPServerPort());

        settings.setValue("logfromwsjtx", UDPPage->getLogFromWSJTx());
        settings.setValue("logautofromwsjtx", UDPPage->getAutoLogFromWSJTx());
        settings.setValue("realtimefromwsjtx", UDPPage->getReaDataFromWSJTx());

        settings.setValue("infotimeout", UDPPage->getTimeout());

          //qDebug() << "SetupDialog::slotOkButtonClicked: hamlib" << QT_ENDL;


        hamlibPage->getData();

        //stream << _aa << QT_ENDL;

          //qDebug() << "SetupDialog::slotOkButtonClicked: hamlib-2: " << _aa << QT_ENDL;

        //WSJTX

        //Windows Size
        if (windowSize.length()>0)
        {
            settings.setValue("mainwindowsize", windowSize);
        }
        if (latestBackup.length()>0)
        {
            settings.setValue("LatestBackup", latestBackup);
        }

    hamlibPage->stopHamlib();
    //qDebug() << "SetupDialog::slotOkButtonClicked - just before leaving" << QT_ENDL;
    QDialog::accept();
    logEvent(Q_FUNC_INFO, "END", Debug);
    //qDebug() << "SetupDialog::slotOkButtonClicked - END" << QT_ENDL;
    //close();
}

void SetupDialog::slotReadConfigData()
{
    //qDebug() << "SetupDialog::slotReadConfigData" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    if (firstTime)
    {
        //qDebug() << "SetupDialog::slotReadConfigData - First time" << QT_ENDL;
        setDefaults();
        bands.removeDuplicates();
        modes.removeDuplicates();
        logViewFields.removeDuplicates();
        bandModePage->setActiveModes(modes);
        bandModePage->setActiveBands(bands);
        logViewPage->setActiveFields(logViewFields);
    }

    //qDebug() << "SetupDialog::slotReadConfigData - 1" << QT_ENDL;

    QFile file(configFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))  /* Flawfinder: ignore */
    {
        //qDebug() << "SetupDialog::slotReadConfigData() File not found" << configFileName << QT_ENDL;
        //firstTime = true;
        emit debugLog (Q_FUNC_INFO, "END-1", logLevel);
        return;
    }
    //qDebug() << "SetupDialog::slotReadConfigData - 2" << QT_ENDL;
    //dxClusterServers.clear();
    QSettings settings(util->getCfgFile (), QSettings::IniFormat);
    userDataPage->setMainCallsign(settings.value("callsign").toString ());


    while (!file.atEnd()){
        QByteArray line = file.readLine();
        processConfigLine(line);
        //qDebug() << "SetupDialog::slotReadConfigData - in the while" << QT_ENDL;
    }
    //qDebug() << "SetupDialog::slotReadConfigData - 3" << QT_ENDL;

    dxClusterPage->setDxclusterServersComboBox(dxClusterServers);
    dxClusterPage->setSelectedDxClusterServer(dxClusterServerToUse);

    if (modes.isEmpty())
    {
        modes << "SSB" << "CW" << "RTTY";
    }
    if (bands.isEmpty())
    {
        bands << "10M" << "12M" << "15M" << "17M" << "20M" << "40M" << "80M" << "160M";
    }
    if (logViewFields.isEmpty())
    {
        logViewFields << "qso_date" << "call" << "rst_sent" << "rst_rcvd" << "bandid" << "modeid" << "comment";
    }
    modes.removeDuplicates();
    bandModePage->setActiveModes(modes);
    bands.removeDuplicates();
    bandModePage->setActiveBands(bands);
    logViewFields.removeDuplicates();
    logViewPage->setActiveFields(logViewFields);
    //qDebug() << "SetupDialog::slotReadConfigData - END" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "END", Debug);
}

bool SetupDialog::processConfigLine(const QString &_line)
{
    //qDebug() << "SetupDialog::processConfigLine: " << _line << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);


    QString line = _line.simplified();
    //line.simplified();
    int i = 0; //aux variable
    QStringList values = line.split("=", QT_SKIP);
    QString tab = QString();

    if (line.startsWith('#')){
           //qDebug() << "SetupDialog::processConfigLine: Comment Line!" << QT_ENDL;
        emit debugLog (Q_FUNC_INFO, "END-1", logLevel);
        return true;
    }
    if (!( (line.contains('=')) && (line.contains(';')))){
           //qDebug() << "SetupDialog::processConfigLine: Wrong Line!" << QT_ENDL;
        emit debugLog (Q_FUNC_INFO, "END-2", logLevel);
        return false;
    }
    QString value = values.at(1);
    tab = (values.at(0)).toUpper();

    int endValue = value.indexOf(';');
    if (endValue>-1){

        value = value.left(value.length() - (value.length() - endValue));
    }
    value = checkAndFixASCIIinADIF(value); // Check whether the value is valid.
      //qDebug() << "SetupDialog::processConfigLine: TAB: " << tab << QT_ENDL;
      //qDebug() << "SetupDialog::processConfigLine: VALUE: " << value << QT_ENDL;
    if (tab == "CALLSIGN"){
           //qDebug() << "SetupDialog::processConfigLine: CALLSIGN: " << value << QT_ENDL;
        userDataPage->setMainCallsign(value);
    }else if (tab == "OPERATORS"){
        userDataPage->setOperators(value);
    }else if (tab=="CQZ"){
        userDataPage->setCQz((value).toInt());
    }else if (tab=="ITUZ"){
        userDataPage->setITUz((value).toInt());
    }else if (tab=="CONTEST"){
        //userDataPage->setContest(value);
    }else if (tab=="MODES"){
        readActiveModes(value);
        modes.removeDuplicates();
        bandModePage->setActiveModes(modes);
    }else if (tab=="BANDS"){
        readActiveBands(value);
        bands.removeDuplicates();
        bandModePage->setActiveBands(bands);
    //}else if (tab=="INMEMORY"){
    //    miscPage->setInMemory(value);
    }else if (tab=="LOGVIEWFIELDS"){
        logViewFields.clear();
        //qDebug() << Q_FUNC_INFO << ": " << value;
        //qDebug() << Q_FUNC_INFO << "llamando a filterValidFields";
        logViewFields << dataProxy->filterValidFields(value.split(",", QT_SKIP));
        logViewFields.removeDuplicates();
        logViewPage->setActiveFields(logViewFields);
    }else if (tab=="REALTIME"){
        miscPage->setRealTime(value);
    }else if (tab=="SHOWSECONDS"){
        miscPage->setShowSeconds (util->trueOrFalse (value));
    }else if (tab=="UTCTIME"){
        miscPage->setUTCTime(value);
    }else if (tab=="ALWAYSADIF"){
        miscPage->setAlwaysADIF(value);
    }else if (tab=="USEDEFAULTNAME"){
        miscPage->setDefaultFileName(value);
    }else if (tab=="DBPATH"){
        miscPage->setUseDefaultDBPath(value);
    }else if (tab=="DEFAULTADIFFILE"){
        miscPage->setDefaultFileName(value);
           //qDebug() << "SetupDialog::processConfigLine: FILE: " << value << QT_ENDL;
    }else if (tab=="IMPERIALSYSTEM"){
        miscPage->setImperial(value.toUpper());
    }else if (tab=="COMPLETEWITHPREVIOUS"){
        miscPage->setCompleteWithPrevious(value.toUpper());
    }else if (tab=="SENDQSLWHENREC"){
        miscPage->setSendQSLWhenRec(value.toUpper());
    }else if (tab=="MANAGEDXMARATHON"){
        miscPage->setDXMarathon(value.toUpper());
    }else if (tab=="DEBUGLOG"){
        miscPage->setDebugLogLevel(value);
    }
    else if (tab=="SHOWCALLSIGNINSEARCH"){
        miscPage->setShowStationCallSignInSearch(value.toUpper());
    }
    else if (tab=="CHECKNEWVERSIONS"){
        miscPage->setCheckNewVersions(value);
    }
    else if (tab=="PROVIDEINFO"){
        miscPage->setReportInfo(value);
    }
    /*
    else if (tab=="LOGSORT"){
        miscPage->setLogSort(value);
    }
    */
    else if (tab=="SENDEQSLBYDEFAULT"){
        miscPage->setSetEQSLByDefault(value);
    }
    else if (tab=="DUPLICATEDQSOSLOT"){
        if (value.toInt()>=0)
        {
            miscPage->setDupeTime(value.toInt());
        }
    }
    else if (tab == "CHECKVALIDCALLS")
    {
        miscPage->setCheckCalls (util->trueOrFalse (value));
    }
    /*
    else if (tab=="PSTROTATORACTIVE"){
        interfacesWindowsPage->setSendToPSTRotator(value);
    }
    else if (tab=="PSTROTATORPORT"){
        interfacesWindowsPage->setPSTRotatorUDPServerPort(value);
    }
    else if (tab=="PSTROTATORSERVER")
    {
        interfacesWindowsPage->setPSTRotatorUDPServer(value);
    }
    */
    else if (tab=="UDPSERVER"){
        UDPPage->setUDPServer(value);
    }
    else if (tab=="UDPNETWORKINTERFACE"){
        UDPPage->setNetworkInterface(value);
    }
    else if (tab=="UDPSERVERPORT"){
        UDPPage->setUDPServerPort(value);
    }
    else if (tab=="LOGFROMWSJTX")
    {
        UDPPage->setLogFromWSJTx(value);
    }
    else if (tab=="LOGAUTOFROMWSJTX")
    {
        UDPPage->setAutoLogFromWSJTx(value);
    }
    else if (tab=="REALTIMEFROMWSJTX")
    {
        UDPPage->setReaDataFromWSJTx(value);
    }
    else if (tab=="INFOTIMEOUT")
    {
        UDPPage->setTimeout(value);
    }
    else if (tab =="NAME")
    {
        userDataPage->setName(value);
    }
    else if (tab =="ADDRESS1")
    {
        userDataPage->setAddress1(value);
    }
    else if (tab =="ADDRESS2")
    {
        userDataPage->setAddress2(value);
    }
    else if (tab =="ADDRESS3")
    {
        userDataPage->setAddress3(value);
    }
    else if (tab =="ADDRESS4")
    {
        userDataPage->setAddress4(value);
    }
    else if (tab =="CITY")
    {
        userDataPage->setCity(value);
    }
    else if (tab =="ZIPCODE")
    {
        userDataPage->setZipCode(value);
    }
    else if (tab =="PROVINCESTATE")
    {
        userDataPage->setProvince(value);
    }
    else if (tab =="COUNTRY")
    {
        userDataPage->setCountry(value);
    }
    else if (tab =="POWER")
    {
        userDataPage->setPower(value);
    }
    else if (tab =="RIG1")
    {
        userDataPage->setRig1(value);
    }
    else if (tab =="RIG2")
    {
        userDataPage->setRig2(value);
    }
    else if (tab =="RIG3")
    {
        userDataPage->setRig3(value);
    }

    else if (tab =="ANTENNA1")
    {
        userDataPage->setAntenna1(value);
    }
    else if (tab =="ANTENNA2")
    {
        userDataPage->setAntenna2(value);
    }
    else if (tab =="ANTENNA3")
    {
        userDataPage->setAntenna3(value);
    }
    else if (tab =="STATIONLOCATOR"){

        if ( locator->isValidLocator(value) )
        {
            userDataPage->setStationLocator(value);
        }
    }else if (tab =="DXCLUSTERSHOWHF"){
        dxClusterPage->setShowHFQCheckbox(value);
    }else if (tab =="DXCLUSTERSHOWVHF"){
        dxClusterPage->setShowVHFQCheckbox(value);
    }else if (tab =="DXCLUSTERSHOWWARC"){
        dxClusterPage->setShowWARCQCheckbox(value);
    }else if (tab =="DXCLUSTERSHOWWORKED"){
        dxClusterPage->setShowWorkedQCheckbox(value);
    }else if (tab =="DXCLUSTERSHOWCONFIRMED"){
        dxClusterPage->setShowConfirmedQCheckbox(value);
    }else if (tab =="DXCLUSTERSHOWANN"){
        dxClusterPage->setShowANNQCheckbox(value);
    }else if (tab =="DXCLUSTERSHOWWWV"){
        dxClusterPage->setShowWWVQCheckbox(value);
    }else if (tab =="DXCLUSTERSHOWWCY"){
        dxClusterPage->setShowWCYQCheckbox(value);
    }else if(tab =="DXCLUSTERSERVERPORT"){
        dxClusterServers << value;
           //qDebug() << "SetupDialog::processConfigLine: dxClusterServers: " << dxClusterServers.last() << QT_ENDL;
    }else if (tab  =="DXCLUSTERSERVERTOUSE"){
        dxClusterServerToUse=value;
    }
    else if (tab  =="DXCLUSTERSAVE"){
        dxClusterPage->setSaveActivityQCheckbox(value);
    }
    else if (tab  =="DXCLUSTERSENDTOMAP"){
        dxClusterPage->setSendSpotstoMap(value);
    }
    else if(tab =="NEWONECOLOR"){
        colorsPage->setNewOneColor(value);
    }else if(tab =="NEEDEDCOLOR"){
        colorsPage->setNeededColor(value);
    }else if(tab =="WORKEDCOLOR"){
        colorsPage->setWorkedColor(value);
    }else if(tab =="CONFIRMEDCOLOR"){
        colorsPage->setConfirmedColor(value);
    }else if(tab =="DEFAULTCOLOR"){
        colorsPage->setDefaultColor(value);
          //qDebug() << "SetupDialog::processConfigLine: DEFAULTCOLOR: " << value << QT_ENDL;
    }else if(tab =="DARKMODE"){
        colorsPage->setDarkMode(value);
    }else if(tab =="HAMLIBRIGTYPE"){
          //qDebug() << "SetupDialog::processConfigLine: Before HAMLIBRIGTYPE: " << value << QT_ENDL;
        hamlibPage->setRigType(value);
          //qDebug() << "SetupDialog::processConfigLine: After HAMLIBRIGTYPE: " << value << QT_ENDL;
    }else if(tab =="HAMLIBSERIALPORT"){
          //qDebug() << "SetupDialog::processConfigLine: HAMLIBSERIALPORT: " << value << QT_ENDL;
        hamlibPage->setSerialPort(value);
    }else if(tab =="HAMLIBSERIALBAUDS"){
          //qDebug() << "SetupDialog::processConfigLine: HAMLIBSERIALBAUDS: " << value << QT_ENDL;
        hamlibPage->setSerialSpeed(value.toInt());
    }else if(tab =="HAMLIB"){
          //qDebug() << "SetupDialog::processConfigLine: HAMLIB: " << value << QT_ENDL;
        hamlibPage->setActive(value);
    }else if(tab=="HAMLIBREADONLY"){
        hamlibPage->setReadOnly(value);
    }else if(tab =="HAMLIBSERIALDATABITS"){
        //qDebug() << "SetupDialog::processConfigLine: HAMLIBSERIALDATABITS: " << value << QT_ENDL;
        hamlibPage->setDataBits(value.toInt ());
    }else if(tab =="HAMLIBSERIALSTOPBITS"){
          //qDebug() << "SetupDialog::processConfigLine: HAMLIBSERIALSTOPBITS: " << value << QT_ENDL;
        hamlibPage->setStopBits(value);
    }else if(tab =="HAMLIBSERIALFLOWCONTROL"){
          //qDebug() << "SetupDialog::processConfigLine: HAMLIBSERIALFLOWCONTROL: " << value << QT_ENDL;
        hamlibPage->setFlowControl(value);
    }else if(tab =="HAMLIBSERIALPARITY"){
          //qDebug() << "SetupDialog::processConfigLine: HAMLIBSERIALPARITY: " << value << QT_ENDL;
        hamlibPage->setParity(value);
    }else if(tab =="HAMLIBSERIALRTS"){
          //qDebug() << "SetupDialog::processConfigLine: HAMLIBSERIALRTS: " << value << QT_ENDL;
        //hamlibPage->setRTS(value);
    }else if(tab =="HAMLIBSERIALDTR"){
          //qDebug() << "SetupDialog::processConfigLine: HAMLIBSERIALDTR: " << value << QT_ENDL;
        //hamlibPage->setDTR(value);
    }else if (tab == "HAMLIBRIGPOLLRATE"){
        hamlibPage->setPollingInterval(value.toInt());
    }else if (tab == "HAMLIBNETADDRESS"){
        hamlibPage->setRadioNetworkAddress (value);
    }else if (tab == "HAMLIBNETPORT"){
        hamlibPage->setRadioNetworkPort (value.toInt ());
    }else if(tab =="SELECTEDLOG"){
           //qDebug() << "SetupDialog::processConfigLine: SELECTEDLOG: " << value << QT_ENDL;
        i = value.toInt();

        if (dataProxy->doesThisLogExist(i))
        {
               //qDebug() << "SetupDialog::processConfigLine: dataProxy->doesThisLogExist TRUE" << QT_ENDL;
        }
        else
        {
               //qDebug() << "SetupDialog::processConfigLine: dataProxy->doesThisLogExist FALSE" << QT_ENDL;
            i = 0;
            while(!dataProxy->doesThisLogExist(i))
            {
                i++;
            }
        }
        logsPage->setSelectedLog(i);
           //qDebug() << "SetupDialog::processConfigLine: dataProxy->doesThisLogExist END" << QT_ENDL;
    }else if(tab =="CLUBLOGACTIVE"){
        eLogPage->setClubLogActive(util->trueOrFalse(value));
    }
    else if(tab =="CLUBLOGREALTIME"){
        //clubLogPage->setClubLogRealTime(value);
        eLogPage->setClubLogRealTime(util->trueOrFalse(value));
    }
    else if(tab =="CLUBLOGPASS"){
        //clubLogPage->setPassword(value);
        eLogPage->setClubLogPassword(value);
    }
    else if(tab =="CLUBLOGEMAIL"){
        //clubLogPage->setEmail(value);
        eLogPage->setClubLogEmail(value);
    }
    else if(tab =="EQSLACTIVE"){
        //eQSLPage->setActive(value);
        eLogPage->setEQSLActive(util->trueOrFalse(value));
    }
    else if(tab =="EQSLCALL"){
        //eQSLPage->setCallsign(value);
        eLogPage->setEQSLUser(value);
    }
    else if(tab =="EQSLPASS"){
        //eQSLPage->setPassword(value);
        eLogPage->setEQSLPassword(value);
    }
    else if(tab =="QRZCOMACTIVE"){
        //eQSLPage->setActive(value);
        eLogPage->setQRZCOMActive(util->trueOrFalse(value));
    }
    else if(tab =="QRZCOMSUBSCRIBER"){
        eLogPage->setQRZCOMSubscriber(util->trueOrFalse (value));
    }
    else if(tab =="QRZCOMUSER"){
        eLogPage->setQRZCOMUser(value);
    }
    else if(tab =="QRZCOMAUTO"){
        eLogPage->setQRZCOMAutoCheck(util->trueOrFalse(value));
    }
    else if(tab =="QRZCOMPASS"){
        eLogPage->setQRZCOMPassword(value);
    }
    else if(tab =="QRZCOMLOGBOOKKEY"){
        eLogPage->setQRZCOMLogBookKEY(value);
    }
    else if(tab =="LOTWACTIVE"){
        eLogPage->setLoTWActive(util->trueOrFalse(value));
    }
    else if(tab =="LOTWPATH"){
        eLogPage->setTQSLPath(value);
    }
    else if(tab =="LOTWUSER"){
        eLogPage->setLoTWUser(value);
    }
    else if(tab =="LOTWPASS"){
        eLogPage->setLoTWPass(value);
    }
    else if(tab =="MAINWINDOWSIZE"){
        QStringList values;
        values.clear();
        values << value.split("x");
        if ((values.at(0).toInt()>0) && (values.at(1).toInt()>0))
        {
            windowSize = value;
        }
    }
    else if(tab =="DELETEALWAYSADIFILE"){
            miscPage->setDeleteAlwaysAdiFile(util->trueOrFalse(value));
        }
    else if (tab == "LATESTBACKUP"){
        if (value.length()>0)
        {
            latestBackup = value;
        }
        else
        {
            latestBackup = QString();
        }
    //s.append("LatestBackup=" + (QDateTime::currentDateTime()).toString("yyyyMMdd-hhmmss") + ";\n" );
    }
    else
    {
           //qDebug() << "SetupDialog::processConfigLine: NONE: " << QT_ENDL;
    }

    // Lines are: Option = value;

       //qDebug() << "SetupDialog::processConfigLine: END "  << QT_ENDL;
    logEvent(Q_FUNC_INFO, "END", Debug);
    return true;
}

void SetupDialog::readActiveBands (const QString &actives)
{ // Checks a "10m, 12m" QString, checks if  they are valid bands and import to the
    // bands used in the program
      //qDebug() << "SetupDialog::readActiveBands: " << actives << QT_ENDL;

    logEvent(Q_FUNC_INFO, "Start", Debug);
    bool atLeastOne = false;

    QStringList values = actives.split(", ", QT_SKIP);
    QStringList _abands;

    for (int i = 0; i < values.size() ; i++)
    {
        if (isValidBand(values.at(i)))
        {
            if (!atLeastOne)
            {
                   //qDebug() << "SetupDialog::readActiveBands (at least One!): " << values.at(i) << QT_ENDL;
                atLeastOne = true;
                _abands.clear();
            }

            _abands << values.at(i);
               //qDebug() << "SetupDialog::readActiveBands: " << values.at(i) << QT_ENDL;
        }
    }

    bands.clear();
    //_abands.removeDuplicates();

    bands << dataProxy->getBandsInLog(-1);
    bands << _abands;
    bands.removeDuplicates();
    //qDebug() << Q_FUNC_INFO << " - END";
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::readActiveModes (const QString &actives)
{
       //qDebug() << "SetupDialog::readActiveModes: " << actives << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);

    bool atLeastOne = false;
    QStringList _amodes;//, _backModes;
    // _backModes.clear();
    // _backModes << modes;
    QStringList values = actives.split(", ", QT_SKIP);
    values.removeDuplicates();

    for (int i = 0; i < values.size() ; i++)
    {
        if (isValidMode(values.at(i)))
        {
            if (!atLeastOne)
            {
                atLeastOne = true;
                _amodes.clear();
            }
            _amodes << values.at(i);
        }
    }

    modes.clear();
    modes = dataProxy->getModesInLog(-1);
    modes << _amodes;
    modes.removeDuplicates();
    logEvent(Q_FUNC_INFO, "END", Debug);
       //qDebug() << "SetupDialog::readActiveModes: " << modes.join(" / ") << QT_ENDL;
}

bool SetupDialog::isValidBand (const QString &b)
{
       //qDebug() << "SetupDialog::isValidBand: "<< b << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    QString stringQuery = QString("SELECT id FROM band WHERE name='%1'").arg(b);
    QSqlQuery query(stringQuery);
    query.next();
    logEvent(Q_FUNC_INFO, "END", Debug);
    return query.isValid();
}
bool SetupDialog::isValidMode (const QString &b)
{
       //qDebug() << "SetupDialog::isValidMode: " << b << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    QString stringQuery = QString("SELECT id FROM mode WHERE name='%1'").arg(b);
    QSqlQuery query(stringQuery);
    query.next();
    logEvent(Q_FUNC_INFO, "END", Debug);
    return query.isValid();
}

void SetupDialog::setDefaults()
{
       //qDebug() << "SetupDialog::setDefaults" << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    miscPage->setRealTime("TRUE");
    miscPage->setUTCTime("TRUE");
    miscPage->setImperial("FALSE"); //Metric system is the default
    miscPage->setAlwaysADIF("FALSE");
    miscPage->setSendQSLWhenRec("TRUE");
    miscPage->setShowStationCallSignInSearch("TRUE");
    miscPage->setCheckNewVersions("TRUE");
    miscPage->setReportInfo("FALSE");
    miscPage->setDXMarathon("FALSE");
    miscPage->setDebugLogLevel(util->getDebugLevels().at(0));
    //miscPage->setLogSort("FALSE");
    miscPage->setSetEQSLByDefault("TRUE");
    miscPage->setCheckCalls (true);

    UDPPage->setUDPServer("FALSE");
    UDPPage->setUDPServerPort("2237");
    UDPPage->setTimeout("2000");
    UDPPage->setLogFromWSJTx("FALSE");
    UDPPage->setReaDataFromWSJTx("FALSE");
    UDPPage->setAutoLogFromWSJTx("FALSE");
    //interfacesWindowsPage->setSendToPSTRotator("FALSE");
    //interfacesWindowsPage->setPSTRotatorUDPServer("locahost");
    //interfacesWindowsPage->setPSTRotatorUDPServerPort("12040");

    dxClusterPage->setShowHFQCheckbox("TRUE");
    dxClusterPage->setShowVHFQCheckbox("TRUE");
    dxClusterPage->setShowWARCQCheckbox("TRUE");
    dxClusterPage->setShowWorkedQCheckbox("TRUE");
    dxClusterPage->setShowConfirmedQCheckbox("TRUE");
    dxClusterPage->setShowANNQCheckbox("TRUE");
    dxClusterPage->setShowWWVQCheckbox("TRUE");
    dxClusterPage->setShowWCYQCheckbox("TRUE");
    dxClusterServers.clear();
    dxClusterServers.append("dxfun.com:8000");
    dxClusterServerToUse = "dxfun.com:8000";

    if (modes.isEmpty())
    {
        modes << "SSB" << "CW" << "RTTY";
        modes.removeDuplicates();
    }

    if (bands.isEmpty())
    {
        bands << "10M" << "12M" << "15M" << "17M" << "20M" << "40M" << "80M" << "160M";
        bands.removeDuplicates();
    }
    logEvent(Q_FUNC_INFO, "END", Debug);
}

QString SetupDialog::checkAndFixASCIIinADIF(const QString &_data)
{
       //qDebug() << "SetupDialog::checkAndFixASCIIinADIF " << _data << QT_ENDL;
//TODO: this function is also in the FileManager class. Maybe I should call that one and keep just one copy
    logEvent(Q_FUNC_INFO, "Start", Debug);
    ushort unicodeVal;
    QString st = _data;
    QString newString;
    newString.clear();
    for(int i=0; i < st.length(); i++)
    {
    // Get unicode VALUE into unicodeVal
        unicodeVal = (st.at(i)).unicode();
        if ((20 <= unicodeVal ) && (unicodeVal <= 126))
        {
            newString.append(st.at(i));
        }
           //qDebug() << "SetupDialog::checkAndFixunicodeinADIF: " << st.at(i) <<" = " << QString::number(unicodeVal) << QT_ENDL;
    }

    // Show into another lineEdit

    logEvent(Q_FUNC_INFO, "END", Debug);
    return newString;
}

bool SetupDialog::haveAtleastOneLog()
{
    logEvent(Q_FUNC_INFO, "Start", Debug);
    emit debugLog (Q_FUNC_INFO, "END-1", logLevel);
    return dataProxy->haveAtLeastOneLog();
    //logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::setClubLogActive(const bool _b)
{
    logEvent(Q_FUNC_INFO, "Start", Debug);
    eLogPage->setClubLogActive(_b);
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::setQRZCOMAutoCheckActive(const bool _b)
{
     eLogPage->setQRZCOMAutoCheck(_b);
}

void SetupDialog::setEQSLActive(const bool _b)
{
    eLogPage->setEQSLActive(_b);
}

void SetupDialog::checkIfNewBandOrMode()
{
      //qDebug() << "SetupDialog::checkIfNewBandOrMode: logLevel: " << QString::number(logLevel) << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    QStringList _items;

    _items.clear();
       //qDebug() << "SetupDialog::checkIfNewBandOrMode -1" << QT_ENDL;
    _items << dataProxy->getBandsInLog(-1);
       //qDebug() << "SetupDialog::checkIfNewBandOrMode -2" << QT_ENDL;
    _items << (bandModePage->getBands()).split(", ", QT_SKIP);
       //qDebug() << "SetupDialog::checkIfNewBandOrMode -3" << QT_ENDL;
    _items.removeDuplicates();
       //qDebug() << "SetupDialog::checkIfNewBandOrMode -4" << QT_ENDL;
    bandModePage->setActiveBands(_items);
       //qDebug() << "SetupDialog::checkIfNewBandOrMode -5" << QT_ENDL;

    _items.clear();
    _items << dataProxy->getModesInLog(-1);
    _items << (bandModePage->getModes()).split(", ", QT_SKIP);
    _items.removeDuplicates();
    bandModePage->setActiveModes(_items);
    logEvent(Q_FUNC_INFO, "END", Debug);
       //qDebug() << "SetupDialog::checkIfNewBandOrMode END" << QT_ENDL;
}


void SetupDialog::slotAnalyzeNewLogData(const QStringList _qs)
{
      //qDebug() << "SetupDialog::slotAnalyzeNewLogData (length=" << QString::number(_qs.length()) << ")" << QT_ENDL;
       //qDebug() << "SetupDialog::slotAnalyzeNewLogData" << QT_ENDL;
 // We receive the station callsign and operators from the logs tab
    logEvent(Q_FUNC_INFO, "Start", Debug);
    if (_qs.length()!=2)
    {
        emit debugLog (Q_FUNC_INFO, "END-1", logLevel);
        return;
    }
    userDataPage->setMainCallsign(_qs.at(0));
    userDataPage->setOperators(_qs.at(1));
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::slotSetStationCallSign(const QString &_p)
{
       //qDebug() << "SetupDialog::slotSetStationCallSign: " << _p << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    logsPage->setDefaultStationCallsign(_p);
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::slotSetOperators(const QString &_p)
{
       //qDebug() << "SetupDialog::slotSetOperators: " << _p << QT_ENDL;
    logEvent(Q_FUNC_INFO, "Start", Debug);
    logsPage->setDefaultOperators(_p);
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::setLogLevel(const DebugLogLevel _sev)
{
    logLevel = _sev;
    miscPage->setDebugLogLevel(util->debugLevelToString(_sev));
}

void SetupDialog::slotQueryErrorManagement(QString functionFailed, QString errorCodeS, QString nativeError, QString failedQuery)
{   logEvent(Q_FUNC_INFO, "Start", Debug);
    emit queryError(functionFailed, errorCodeS, nativeError, failedQuery);
    logEvent(Q_FUNC_INFO, "END", Debug);
}

void SetupDialog::slotFocusOK()
{
    okButton->setFocus(Qt::OtherFocusReason);
}

void SetupDialog::showEvent(QShowEvent *event)
{
    //qDebug() << Q_FUNC_INFO << QT_ENDL;
    //qDebug() << Q_FUNC_INFO << " - selectedLog: " << QString::number(logsPage->getSelectedLog()) << QT_ENDL;
    QWidget::showEvent(event);
    userDataPage->setStationFocus();
}

void SetupDialog::logEvent(const QString &_func, const QString &_msg,  DebugLogLevel _level)
{
    if (logLevel<=_level)
        emit debugLog (_func, _msg, _level);
}
