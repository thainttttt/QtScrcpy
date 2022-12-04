#include <QDebug>
#include <QFile>
#include <QFileDialog>
#include <QKeyEvent>
#include <QTime>
#include <QTimer>
#include <QDialogButtonBox>

#include "config.h"
#include "dialog.h"
#include "ui_dialog.h"
#include "videoform.h"
#include "../groupcontroller/groupcontroller.h"

#include "ccombobox.h"

QString s_keyMapPath = "";
#define GROUP_BACKUP "backup.txt"

const QString &getKeyMapPath()
{
    if (s_keyMapPath.isEmpty()) {
        s_keyMapPath = QString::fromLocal8Bit(qgetenv("QTSCRCPY_KEYMAP_PATH"));
        QFileInfo fileInfo(s_keyMapPath);
        if (s_keyMapPath.isEmpty() || !fileInfo.isDir()) {
            s_keyMapPath = QCoreApplication::applicationDirPath() + "/keymap";
        }
    }
    return s_keyMapPath;
}

QMap<QString, QString> loadBackup() {
    QMap<QString, QString> ret;
    qInfo() << "================" << QCoreApplication::applicationDirPath() + "/" + GROUP_BACKUP;
    QFile inputFile(QCoreApplication::applicationDirPath() + "/" + GROUP_BACKUP);
    if (inputFile.open(QIODevice::ReadOnly))
    {
    QTextStream in(&inputFile);
    while (!in.atEnd())
    {
        QString line = in.readLine();
        QStringList list = line.split("|");
        ret[list[1]] = list[0];
    }
    inputFile.close();
    }

    return ret;
}

Dialog::Dialog(QWidget *parent) : QWidget(parent), ui(new Ui::Widget)
{
    ui->setupUi(this);
    initUI();
    defaultGroup = "UnGroup";
    allGroup = "All";
    ui->formRowEdit->setValidator(new QIntValidator(1, 10, this));
    setWindowFlags(Qt::WindowStaysOnTopHint);

    // setup tabwidget
    QWidget *pTabCornerWidget = new QWidget(this);
	QPushButton *pButton = new QPushButton(pTabCornerWidget);
	pButton->setText("+");
	pButton->setMaximumSize(QSize(25, 25));
	QHBoxLayout *pHLayout = new QHBoxLayout(pTabCornerWidget);
	pHLayout->addWidget(pButton);
	ui->groupTabPhone->setCornerWidget(pTabCornerWidget, Qt::TopRightCorner);
    ui->groupTabPhone->setTabsClosable(true);
    addNewTab(allGroup);
    addNewTab(defaultGroup);
    enabledGroup = defaultGroup;

    QPushButton *refreshBtn = new QPushButton(this);
	refreshBtn->setText("Refresh");
    ui->verticalLayout_4->addWidget(refreshBtn);

    updateBootConfig(true);

    backup = loadBackup();
    qInfo() << "=============backup data=============" << backup;

    // signal
    connect(ui->groupTabPhone, &QTabWidget::tabBarDoubleClicked, this, &Dialog::onTabBarDoubleClicked);
    connect(ui->groupTabPhone, &QTabWidget::tabCloseRequested, this, &Dialog::closeCurrentTab);
    connect(pButton, &QPushButton::released, this, &Dialog::onAddButtonClick);
    connect(refreshBtn, &QPushButton::released, this, &Dialog::onRefreshBtnClick);

    // Show Form
    form = new Form(ui->formRowEdit->text().trimmed().toUInt());
    form->show();
    form->showMaximized();
    form->setWindowTitle("ClickFarm.vn - Group Remote Control");

    on_useSingleModeCheck_clicked();
    on_updateDevice_clicked();

    connect(&m_autoUpdatetimer, &QTimer::timeout, this, &Dialog::on_updateDevice_clicked);
    if (ui->autoUpdatecheckBox->isChecked()) {
        m_autoUpdatetimer.start(2000);
    }

    connect(&m_adb, &qsc::AdbProcess::adbProcessResult, this, [this](qsc::AdbProcess::ADB_EXEC_RESULT processResult) {
        QString log = "";
        bool newLine = true;
        QStringList args = m_adb.arguments();

        switch (processResult) {
        case qsc::AdbProcess::AER_ERROR_START:
            break;
        case qsc::AdbProcess::AER_SUCCESS_START:
            log = "adb run";
            newLine = false;
            break;
        case qsc::AdbProcess::AER_ERROR_EXEC:
            //log = m_adb.getErrorOut();
            if (args.contains("ifconfig") && args.contains("wlan0")) {
                getIPbyIp();
            }
            break;
        case qsc::AdbProcess::AER_ERROR_MISSING_BINARY:
            log = "adb not found";
            break;
        case qsc::AdbProcess::AER_SUCCESS_EXEC:
            //log = m_adb.getStdOut();
            if (args.contains("devices")) {
                QStringList devices = m_adb.getDevicesSerialFromStdOut();
                ui->serialBox->clear();
                // ui->connectedPhoneList->clear();
                for (auto &item : devices) {
                    ui->serialBox->addItem(item);

                    // update timeout
                    int count = loadCount.value(item, 1);
                    // if cannot load serial in 3 times, continue
                    if (count > 3) {
                        continue;
                    }
                    loadCount[item] = count + 1;
                    if(form->videoForms.find(item.toStdString()) == form->videoForms.end()) {
                        ui->serialBox->setCurrentText(item);
                        on_startServerBtn_clicked();
                        break;
                    }
                }

            } else if (args.contains("show") && args.contains("wlan0")) {
                QString ip = m_adb.getDeviceIPFromStdOut();
                if (ip.isEmpty()) {
                    log = "ip not find, connect to wifi?";
                    break;
                }
                ui->deviceIpEdt->setText(ip);
            } else if (args.contains("ifconfig") && args.contains("wlan0")) {
                QString ip = m_adb.getDeviceIPFromStdOut();
                if (ip.isEmpty()) {
                    log = "ip not find, connect to wifi?";
                    break;
                }
                ui->deviceIpEdt->setText(ip);
            } else if (args.contains("ip -o a")) {
                QString ip = m_adb.getDeviceIPByIpFromStdOut();
                if (ip.isEmpty()) {
                    log = "ip not find, connect to wifi?";
                    break;
                }
                ui->deviceIpEdt->setText(ip);
            }
            break;
        }
        if (!log.isEmpty()) {
            outLog(log, newLine);
        }
    });

    m_hideIcon = new QSystemTrayIcon(this);
    m_hideIcon->setIcon(QIcon(":/image/tray/logo.png"));
    m_menu = new QMenu(this);
    m_quit = new QAction(this);
    m_showWindow = new QAction(this);
    m_showWindow->setText(tr("show"));
    m_quit->setText(tr("quit"));
    m_menu->addAction(m_showWindow);
    m_menu->addAction(m_quit);
    m_hideIcon->setContextMenu(m_menu);
    m_hideIcon->show();
    connect(m_showWindow, &QAction::triggered, this, &Dialog::show);
    connect(m_quit, &QAction::triggered, this, [this]() {
        m_hideIcon->hide();
        qApp->quit();
    });
    connect(m_hideIcon, &QSystemTrayIcon::activated, this, &Dialog::slotActivated);

    connect(&qsc::IDeviceManage::getInstance(), &qsc::IDeviceManage::deviceConnected, this, &Dialog::onDeviceConnected);
    connect(&qsc::IDeviceManage::getInstance(), &qsc::IDeviceManage::deviceDisconnected, this, &Dialog::onDeviceDisconnected);
}

Dialog::~Dialog()
{
    qDebug() << "~Dialog()";
    updateBootConfig(false);
    qsc::IDeviceManage::getInstance().disconnectAllDevice();
    delete ui;
}

void Dialog::initUI()
{
    setAttribute(Qt::WA_DeleteOnClose);
    //setWindowFlags(windowFlags() | Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint | Qt::CustomizeWindowHint);

    setWindowTitle("ClickFarm.vn");

    ui->bitRateEdit->setValidator(new QIntValidator(1, 99999, this));

    ui->maxSizeBox->addItem("640");
    ui->maxSizeBox->addItem("720");
    ui->maxSizeBox->addItem("1080");
    ui->maxSizeBox->addItem("1280");
    ui->maxSizeBox->addItem("1920");
    ui->maxSizeBox->addItem(tr("original"));

    ui->formatBox->addItem("mp4");
    ui->formatBox->addItem("mkv");

    ui->lockOrientationBox->addItem(tr("no lock"));
    ui->lockOrientationBox->addItem("0");
    ui->lockOrientationBox->addItem("90");
    ui->lockOrientationBox->addItem("180");
    ui->lockOrientationBox->addItem("270");
    ui->lockOrientationBox->setCurrentIndex(0);
}

void Dialog::updateBootConfig(bool toView)
{
    if (toView) {
        UserBootConfig config = Config::getInstance().getUserBootConfig();

        if (config.bitRate == 0) {
            ui->bitRateBox->setCurrentText("Mbps");
        } else if (config.bitRate % 1000000 == 0) {
            ui->bitRateEdit->setText(QString::number(config.bitRate / 1000000));
            ui->bitRateBox->setCurrentText("Mbps");
        } else {
            ui->bitRateEdit->setText(QString::number(config.bitRate / 1000));
            ui->bitRateBox->setCurrentText("Kbps");
        }

        ui->maxSizeBox->setCurrentIndex(config.maxSizeIndex);
        ui->formatBox->setCurrentIndex(config.recordFormatIndex);
        ui->recordPathEdt->setText(config.recordPath);
        ui->lockOrientationBox->setCurrentIndex(config.lockOrientationIndex);
        ui->framelessCheck->setChecked(config.framelessWindow);
        ui->recordScreenCheck->setChecked(config.recordScreen);
        ui->notDisplayCheck->setChecked(config.recordBackground);
        ui->useReverseCheck->setChecked(config.reverseConnect);
        ui->fpsCheck->setChecked(config.showFPS);
        ui->alwaysTopCheck->setChecked(config.windowOnTop);
        ui->closeScreenCheck->setChecked(config.autoOffScreen);
        ui->stayAwakeCheck->setChecked(config.keepAlive);
        ui->useSingleModeCheck->setChecked(config.simpleMode);
        ui->autoUpdatecheckBox->setChecked(config.autoUpdateDevice);
        ui->formRowEdit->setText(config.formRow);
    } else {
        UserBootConfig config;

        config.bitRate = getBitRate();
        config.maxSizeIndex = ui->maxSizeBox->currentIndex();
        config.recordFormatIndex = ui->formatBox->currentIndex();
        config.recordPath = ui->recordPathEdt->text();
        config.lockOrientationIndex = ui->lockOrientationBox->currentIndex();
        config.recordScreen = ui->recordScreenCheck->isChecked();
        config.recordBackground = ui->notDisplayCheck->isChecked();
        config.reverseConnect = ui->useReverseCheck->isChecked();
        config.showFPS = ui->fpsCheck->isChecked();
        config.windowOnTop = ui->alwaysTopCheck->isChecked();
        config.autoOffScreen = ui->closeScreenCheck->isChecked();
        config.framelessWindow = ui->framelessCheck->isChecked();
        config.keepAlive = ui->stayAwakeCheck->isChecked();
        config.simpleMode = ui->useSingleModeCheck->isChecked();
        config.autoUpdateDevice = ui->autoUpdatecheckBox->isChecked();
        config.formRow = ui->formRowEdit->text();
        Config::getInstance().setUserBootConfig(config);
    }
}

void Dialog::execAdbCmd()
{
    if (checkAdbRun()) {
        return;
    }
    QString cmd = ui->adbCommandEdt->text().trimmed();
    outLog("adb " + cmd, false);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    m_adb.execute(ui->serialBox->currentText().trimmed(), cmd.split(" ", Qt::SkipEmptyParts));
#else
    m_adb.execute(ui->serialBox->currentText().trimmed(), cmd.split(" ", QString::SkipEmptyParts));
#endif
}

void Dialog::delayMs(int ms)
{
    QTime dieTime = QTime::currentTime().addMSecs(ms);

    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}

QString Dialog::getGameScript(const QString &fileName)
{
    if (fileName.isEmpty()) {
        return "";
    }

    QFile loadFile(getKeyMapPath() + "/" + fileName);
    if (!loadFile.open(QIODevice::ReadOnly)) {
        outLog("open file failed:" + fileName, true);
        return "";
    }

    QString ret = loadFile.readAll();
    loadFile.close();
    return ret;
}

void Dialog::slotActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::Trigger:
#ifdef Q_OS_WIN32
        this->show();
#endif
        break;
    default:
        break;
    }
}

void Dialog::closeEvent(QCloseEvent *event)
{
    this->hide();
    m_hideIcon->showMessage(tr("Notice"),
                            tr("Hidden here!"),
                            QSystemTrayIcon::Information,
                            3000);
    event->ignore();
}

void Dialog::on_updateDevice_clicked()
{
    if (checkAdbRun()) {
        return;
    }
    outLog("update devices...", false);
    m_adb.execute("", QStringList() << "devices");
}

void Dialog::on_startServerBtn_clicked()
{
    outLog("start server...", false);

    // this is ok that "native" toUshort is 0
    quint16 videoSize = ui->maxSizeBox->currentText().trimmed().toUShort();
    qsc::DeviceParams params;
    params.serial = ui->serialBox->currentText().trimmed();
    params.maxSize = videoSize;
    params.bitRate = getBitRate();
    // on devices with Android >= 10, the capture frame rate can be limited
    params.maxFps = static_cast<quint32>(Config::getInstance().getMaxFps());
    params.closeScreen = ui->closeScreenCheck->isChecked();
    params.useReverse = ui->useReverseCheck->isChecked();
    params.display = !ui->notDisplayCheck->isChecked();
    params.renderExpiredFrames = Config::getInstance().getRenderExpiredFrames();
    params.lockVideoOrientation = ui->lockOrientationBox->currentIndex() - 1;
    params.stayAwake = ui->stayAwakeCheck->isChecked();
    params.recordPath = ui->recordPathEdt->text().trimmed();
    params.recordFileFormat = ui->formatBox->currentText().trimmed();
    params.serverLocalPath = getServerPath();
    params.serverRemotePath = Config::getInstance().getServerPath();
    params.pushFilePath = Config::getInstance().getPushFilePath();
    params.gameScript = getGameScript(ui->gameBox->currentText());
    params.serverVersion = Config::getInstance().getServerVersion();
    params.logLevel = Config::getInstance().getLogLevel();
    params.codecOptions = Config::getInstance().getCodecOptions();
    params.codecName = Config::getInstance().getCodecName();

    qsc::IDeviceManage::getInstance().connectDevice(params);
}

void Dialog::on_stopServerBtn_clicked()
{
    if (qsc::IDeviceManage::getInstance().disconnectDevice(ui->serialBox->currentText().trimmed())) {
        outLog("stop server");
    }
}

void Dialog::on_wirelessConnectBtn_clicked()
{
    if (checkAdbRun()) {
        return;
    }
    QString addr = ui->deviceIpEdt->text().trimmed();
    if (!ui->devicePortEdt->text().isEmpty()) {
        addr += ":";
        addr += ui->devicePortEdt->text().trimmed();
    } else if (!ui->devicePortEdt->placeholderText().isEmpty()) {
        addr += ":";
        addr += ui->devicePortEdt->placeholderText().trimmed();
    } else {
        outLog("error: device port is null", false);
        return;
    }

    outLog("wireless connect...", false);
    QStringList adbArgs;
    adbArgs << "connect";
    adbArgs << addr;
    m_adb.execute("", adbArgs);
}

void Dialog::on_startAdbdBtn_clicked()
{
    if (checkAdbRun()) {
        return;
    }
    outLog("start devices adbd...", false);
    // adb tcpip 5555
    QStringList adbArgs;
    adbArgs << "tcpip";
    adbArgs << "5555";
    m_adb.execute(ui->serialBox->currentText().trimmed(), adbArgs);
}

void Dialog::outLog(const QString &log, bool newLine)
{
    // avoid sub thread update ui
    QString backLog = log;
    QTimer::singleShot(0, this, [this, backLog, newLine]() {
        ui->outEdit->append(backLog);
        if (newLine) {
            ui->outEdit->append("<br/>");
        }
    });
}

bool Dialog::filterLog(const QString &log)
{
    if (log.contains("app_proces")) {
        return true;
    }
    if (log.contains("Unable to set geometry")) {
        return true;
    }
    return false;
}

bool Dialog::checkAdbRun()
{
    if (m_adb.isRuning()) {
        outLog("wait for the end of the current command to run");
    }
    return m_adb.isRuning();
}

void Dialog::on_getIPBtn_clicked()
{
    if (checkAdbRun()) {
        return;
    }

    outLog("get ip...", false);
    // adb -s P7C0218510000537 shell ifconfig wlan0
    // or
    // adb -s P7C0218510000537 shell ip -f inet addr show wlan0
    QStringList adbArgs;
#if 0
    adbArgs << "shell";
    adbArgs << "ip";
    adbArgs << "-f";
    adbArgs << "inet";
    adbArgs << "addr";
    adbArgs << "show";
    adbArgs << "wlan0";
#else
    adbArgs << "shell";
    adbArgs << "ifconfig";
    adbArgs << "wlan0";
#endif
    m_adb.execute(ui->serialBox->currentText().trimmed(), adbArgs);
}

void Dialog::getIPbyIp()
{
    if (checkAdbRun()) {
        return;
    }

    QStringList adbArgs;
    adbArgs << "shell";
    adbArgs << "ip -o a";

    m_adb.execute(ui->serialBox->currentText().trimmed(), adbArgs);
}

void Dialog::onDeviceConnected(bool success, const QString &serial, const QString &deviceName, const QSize &size)
{
    Q_UNUSED(deviceName);
    Q_UNUSED(size);
    if (!success) {
        return;
    }

    auto videoForm = new VideoForm(ui->framelessCheck->isChecked(), Config::getInstance().getSkin());
    videoForm->setSerial(serial);

    qsc::IDeviceManage::getInstance().getDevice(serial)->setUserData(static_cast<void*>(videoForm));
    qsc::IDeviceManage::getInstance().getDevice(serial)->registerDeviceObserver(videoForm);

    videoForm->showFPS(ui->fpsCheck->isChecked());
    if (ui->alwaysTopCheck->isChecked()) {
        videoForm->staysOnTop();
    }

    // must be show before updateShowSize
    // videoForm->show();
    // QString name = Config::getInstance().getNickName(serial);
    // if (name.isEmpty()) {
    //     name = Config::getInstance().getTitle();
    // }
    // videoForm->setWindowTitle(name + "-" + serial);
    // videoForm->updateShowSize(size);
    // videoForm->updateShowSize(QSize(25, 50));
    // videoForm->resize(50, 100);

    // bool deviceVer = size.height() > size.width();
    // QRect rc = Config::getInstance().getRect(serial);
    // bool rcVer = rc.height() > rc.width();
    // // same width/height rate
    // if (rc.isValid() && (deviceVer == rcVer)) {
    //     // mark: resize is for fix setGeometry magneticwidget bug
    //     videoForm->resize(rc.size());
    //     videoForm->setGeometry(rc);
    // }

    GroupController::instance().addDevice(serial);

    form->videoForms[serial.toStdString()] = videoForm;
    
    // restore from backup automatically
    QString group = defaultGroup;
    QListWidget* connectedPhoneList = (QListWidget*) ui->groupTabPhone->widget(1);  // default id=1 (defaultGroup)
    if (backup.contains(serial)) {
        group = backup.value(serial);
        bool newGroup = true;
        for (int k=1; k<ui->groupTabPhone->count(); k++) {
            if (ui->groupTabPhone->tabText(k) == group) {
                newGroup = false;
                connectedPhoneList = (QListWidget*) ui->groupTabPhone->widget(k);
                break;
            }
        }
        if (newGroup) {
            addNewTab(group);
            connectedPhoneList = (QListWidget*) ui->groupTabPhone->widget(ui->groupTabPhone->count()-1);
        }
    }

    // add item to connectedPhoneList
    newTabItem(connectedPhoneList, false, serial, group);

    // add item to allList
    newTabItem((QListWidget*) ui->groupTabPhone->widget(0), true, serial, group);

    // if group is enabled, show in form
    if (group == enabledGroup) form->addForm(videoForm);
}

void Dialog::onDeviceDisconnected(QString serial)
{
    GroupController::instance().removeDevice(serial);
    auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
    if (!device) {
        return;
    }
    auto data = device->getUserData();
    if (data) {
        VideoForm* vf = static_cast<VideoForm*>(data);
        qsc::IDeviceManage::getInstance().getDevice(serial)->deRegisterDeviceObserver(vf);
        vf->close();
        vf->deleteLater();
    }

    form->videoForms.erase(serial.toStdString());
    onRefreshBtnClick();
}

void Dialog::on_wirelessDisConnectBtn_clicked()
{
    if (checkAdbRun()) {
        return;
    }
    QString addr = ui->deviceIpEdt->text().trimmed();
    outLog("wireless disconnect...", false);
    QStringList adbArgs;
    adbArgs << "disconnect";
    adbArgs << addr;
    m_adb.execute("", adbArgs);
}

void Dialog::on_selectRecordPathBtn_clicked()
{
    QFileDialog::Options options = QFileDialog::DontResolveSymlinks | QFileDialog::ShowDirsOnly;
    QString directory = QFileDialog::getExistingDirectory(this, tr("select path"), "", options);
    ui->recordPathEdt->setText(directory);
}

void Dialog::on_recordPathEdt_textChanged(const QString &arg1)
{
    ui->recordPathEdt->setToolTip(arg1.trimmed());
    ui->notDisplayCheck->setCheckable(!arg1.trimmed().isEmpty());
}

void Dialog::on_adbCommandBtn_clicked()
{
    execAdbCmd();
}

void Dialog::on_stopAdbBtn_clicked()
{
    m_adb.kill();
}

void Dialog::on_clearOut_clicked()
{
    ui->outEdit->clear();
}

void Dialog::on_stopAllServerBtn_clicked()
{
    qsc::IDeviceManage::getInstance().disconnectAllDevice();
}

void Dialog::on_refreshGameScriptBtn_clicked()
{
    ui->gameBox->clear();
    QDir dir(getKeyMapPath());
    if (!dir.exists()) {
        outLog("keymap directory not find", true);
        return;
    }
    dir.setFilter(QDir::Files | QDir::NoSymLinks);
    QFileInfoList list = dir.entryInfoList();
    QFileInfo fileInfo;
    int size = list.size();
    for (int i = 0; i < size; ++i) {
        fileInfo = list.at(i);
        ui->gameBox->addItem(fileInfo.fileName());
    }
}

void Dialog::on_applyScriptBtn_clicked()
{
    auto curSerial = ui->serialBox->currentText().trimmed();
    auto device = qsc::IDeviceManage::getInstance().getDevice(curSerial);
    if (!device) {
        return;
    }

    device->updateScript(getGameScript(ui->gameBox->currentText()));
}

void Dialog::on_recordScreenCheck_clicked(bool checked)
{
    if (!checked) {
        return;
    }

    QString fileDir(ui->recordPathEdt->text().trimmed());
    if (fileDir.isEmpty()) {
        qWarning() << "please select record save path!!!";
        ui->recordScreenCheck->setChecked(false);
    }
}

void Dialog::on_usbConnectBtn_clicked()
{
    on_stopAllServerBtn_clicked();
    delayMs(200);
    on_updateDevice_clicked();
    delayMs(200);

    int firstUsbDevice = findDeviceFromeSerialBox(false);
    if (-1 == firstUsbDevice) {
        qWarning() << "No use device is found!";
        return;
    }
    ui->serialBox->setCurrentIndex(firstUsbDevice);

    on_startServerBtn_clicked();
}

int Dialog::findDeviceFromeSerialBox(bool wifi)
{
    QRegExp regIP("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\:([0-9]|[1-9]\\d|[1-9]\\d{2}|[1-9]\\d{3}|[1-5]\\d{4}|6[0-4]\\d{3}|65[0-4]\\d{2}|655[0-2]\\d|6553[0-5])\\b");
    for (int i = 0; i < ui->serialBox->count(); ++i) {
        bool isWifi = regIP.exactMatch(ui->serialBox->itemText(i));
        bool found = wifi ? isWifi : !isWifi;
        if (found) {
            return i;
        }
    }

    return -1;
}

void Dialog::on_wifiConnectBtn_clicked()
{
    on_stopAllServerBtn_clicked();
    delayMs(200);

    on_updateDevice_clicked();
    delayMs(200);

    int firstUsbDevice = findDeviceFromeSerialBox(false);
    if (-1 == firstUsbDevice) {
        qWarning() << "No use device is found!";
        return;
    }
    ui->serialBox->setCurrentIndex(firstUsbDevice);

    on_getIPBtn_clicked();
    delayMs(200);

    on_startAdbdBtn_clicked();
    delayMs(1000);

    on_wirelessConnectBtn_clicked();
    delayMs(2000);

    on_updateDevice_clicked();
    delayMs(200);

    int firstWifiDevice = findDeviceFromeSerialBox(true);
    if (-1 == firstWifiDevice) {
        qWarning() << "No wifi device is found!";
        return;
    }
    ui->serialBox->setCurrentIndex(firstWifiDevice);

    on_startServerBtn_clicked();
}

// void Dialog::on_itemDoubleClicked(QListWidgetItem *item)
// {
//     Q_UNUSED(item);

//     auto listWidget = (QListWidget*) ui->groupTabPhone->currentWidget();
//     QString serial = ((QLabel*) listWidget->itemWidget(item)->layout()->itemAt(0)->widget())->text();

//     // if video form not found, return
//     if(form->videoForms.find(serial.toStdString())==form->videoForms.end()) {
//         qInfo() << "Serial=" << serial << " not found. Please try again later...";
//         return;
//     }

//     // if tabwidget not in enabledGroup, return
//     auto it = std::find(enabledGroup.begin(), enabledGroup.end(), ui->groupTabPhone->tabText(ui->groupTabPhone->currentIndex()));
//     if (it == enabledGroup.end()) return;

//     form->unsetMainForm();
//     if (!mainSerial.isEmpty()) {
//         auto oldMainForm = form->videoForms[mainSerial.toStdString()];
//         oldMainForm->updateGroupState();
//         form->addForm(oldMainForm);

//         if (mainSerial == serial) {
//             mainSerial.clear();
//             return;
//         }
//     }
//     auto mainForm = form->videoForms[serial.toStdString()];
//     form->setMainForm(mainForm);
//     mainForm->updateGroupState();

//     mainSerial = serial;
// }

void Dialog::on_updateNameBtn_clicked()
{
    if (ui->serialBox->count() != 0) {
        if (ui->userNameEdt->text().isEmpty()) {
            Config::getInstance().setNickName(ui->serialBox->currentText(), "Phone");
        } else {
            Config::getInstance().setNickName(ui->serialBox->currentText(), ui->userNameEdt->text());
        }

        on_updateDevice_clicked();

        qDebug() << "Update OK!";
    } else {
        qWarning() << "No device is connected!";
    }
}

void Dialog::on_useSingleModeCheck_clicked()
{
    if (ui->useSingleModeCheck->isChecked()) {
        ui->rightWidget->hide();
    } else {
        ui->rightWidget->show();
    }

    adjustSize();
}

void Dialog::on_serialBox_currentIndexChanged(const QString &arg1)
{
    ui->userNameEdt->setText(Config::getInstance().getNickName(arg1));
}

quint32 Dialog::getBitRate()
{
    return ui->bitRateEdit->text().trimmed().toUInt() *
            (ui->bitRateBox->currentText() == QString("Mbps") ? 1000000 : 1000);
}

const QString &Dialog::getServerPath()
{
    static QString serverPath;
    if (serverPath.isEmpty()) {
        serverPath = QString::fromLocal8Bit(qgetenv("QTSCRCPY_SERVER_PATH"));
        QFileInfo fileInfo(serverPath);
        if (serverPath.isEmpty() || !fileInfo.isFile()) {
            serverPath = QCoreApplication::applicationDirPath() + "/scrcpy-server";
        }
    }
    return serverPath;
}

void Dialog::on_startAudioBtn_clicked()
{
    if (ui->serialBox->count() == 0) {
        qWarning() << "No device is connected!";
        return;
    }

    m_audioOutput.start(ui->serialBox->currentText(), 28200);
}

void Dialog::on_stopAudioBtn_clicked()
{
    m_audioOutput.stop();
}

void Dialog::on_installSndcpyBtn_clicked()
{
    if (ui->serialBox->count() == 0) {
        qWarning() << "No device is connected!";
        return;
    }
    m_audioOutput.installonly(ui->serialBox->currentText(), 28200);
}

void Dialog::on_autoUpdatecheckBox_toggled(bool checked)
{
    if (checked) {
        m_autoUpdatetimer.start(5000);
    } else {
        m_autoUpdatetimer.stop();
    }
}

void Dialog::addNewTab(QString &label) {
	QListWidget* listWidget = new QListWidget;

	ui->groupTabPhone->addTab(listWidget, label);

    connect(listWidget, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, SLOT(on_itemDoubleClicked(QListWidgetItem*)));
}

void Dialog::closeCurrentTab(int index)
{
	if (index < 2)
		return;

    auto listWidget = (QListWidget*) ui->groupTabPhone->widget(index);
    auto targetListWidget = (QListWidget*) ui->groupTabPhone->widget(1);  //Ungroup
    while (listWidget->count()>0) {
        QListWidgetItem* item = listWidget->item(0);
        QWidget* itemWidget = listWidget->itemWidget(item);
        QString targetLabel = ((QLabel*) itemWidget->layout()->itemAt(0)->widget())->text();

        newTabItem(targetListWidget, false, targetLabel, defaultGroup);

        item = listWidget->takeItem(0);
        delete item;
    }

	ui->groupTabPhone->removeTab(index);
}

void Dialog::onAddButtonClick()
{
	static int subTabId=0;
	auto label = QString::fromUtf8("Group %1").arg(subTabId);
	subTabId++;
	addNewTab(label);
}

void Dialog::onRefreshBtnClick() {
    /*
    b0: update form row
    b1.1: remove in other group
    b1.2: update from all group
    b2: update tick group
    b3: refresh form
    */
    form->setRow(ui->formRowEdit->text().trimmed().toUInt());

    // b1.1
    for (int i=1; i<ui->groupTabPhone->count(); i++) {
        auto listWidget = (QListWidget*) ui->groupTabPhone->widget(i);

        while (listWidget->count()>0) {
            QListWidgetItem* item = listWidget->item(0);
            item = listWidget->takeItem(0);
            delete item;
        }
    }

    // update enabledGroup, get currentIndex
    int currentIndex = ui->groupTabPhone->currentIndex();
    enabledGroup = ui->groupTabPhone->tabText(currentIndex);

    // b1.2
    QListWidget* allPhoneList = (QListWidget*) ui->groupTabPhone->widget(0);
    for (int j=0; j<allPhoneList->count(); j++) {
        QListWidgetItem* item = allPhoneList->item(j);
        QWidget* itemWidget = allPhoneList->itemWidget(item);
        QString targetGroup = ((CComboBox*) itemWidget->layout()->itemAt(1)->widget())->currentText();
        QString targetLabel = ((QLabel*) itemWidget->layout()->itemAt(0)->widget())->text();

        int targetTabindex = 1;  //UnGroup
        for (int k=1; k<ui->groupTabPhone->count(); k++) {
            if (ui->groupTabPhone->tabText(k) != targetGroup) continue;
            targetTabindex = k;
            break;
        }
        auto targetListWidget = (QListWidget*) ui->groupTabPhone->widget(targetTabindex);
        targetGroup = ui->groupTabPhone->tabText(targetTabindex);
        ((CComboBox*) itemWidget->layout()->itemAt(1)->widget())->setCurrentText(targetGroup);
        newTabItem(targetListWidget, false, targetLabel, targetGroup);
    }

    // refresh form
    form->resetForm();
    QFile outFile(QCoreApplication::applicationDirPath() + "/" + GROUP_BACKUP);
    outFile.open(QIODevice::WriteOnly);
    qInfo() << "========BACKUP========";
    QTextStream textStream(&outFile);
    for (int i=0; i<ui->groupTabPhone->count(); i++) {
        auto listWidget = (QListWidget*) ui->groupTabPhone->widget(i);
        QString tabGroup = ui->groupTabPhone->tabText(i);
        for (int j=0; j<listWidget->count(); j++) {
            QWidget* itemWidget = listWidget->itemWidget(listWidget->item(j));
            QString targetLabel = ((QLabel*) itemWidget->layout()->itemAt(0)->widget())->text();

            // if form is unavailable, cannot add it
            if(form->videoForms.find(targetLabel.toStdString())==form->videoForms.end()) {
                auto item = listWidget->takeItem(j);
                delete item;
                j--;
            } else {
                auto device = qsc::IDeviceManage::getInstance().getDevice(targetLabel);
                if (i != currentIndex) {
                    device->enableGroup = false;
                } else {
                    form->addForm(form->videoForms[targetLabel.toStdString()]);
                    device->enableGroup = true;
                }

                textStream << tabGroup << "|" << targetLabel << "\n";
            }
        }
    }
    outFile.close();

}

void Dialog::newTabItem(QListWidget* list, bool all, QString label, QString &group) {
    QListWidgetItem* bItemWidget = new QListWidgetItem();

    QHBoxLayout *itemlayout = new QHBoxLayout();
    QWidget *itemWidget = new QWidget();
    itemlayout->addWidget(new QLabel(label));
    auto itemListGroup = new CComboBox(ui->groupTabPhone, all);
    itemListGroup->addItem(group);
    itemlayout->addWidget(itemListGroup);
    itemlayout->setStretch(0, 1);
    itemlayout->setContentsMargins(0,0,0,0);
    itemWidget->setLayout(itemlayout);

    bItemWidget->setSizeHint(itemWidget->sizeHint());
    list->addItem(bItemWidget);
    list->setItemWidget(bItemWidget, itemWidget);
}


void Dialog::onTabBarDoubleClicked(int index) {
    if (index==0) return;

    QDialog dlg;
    QVBoxLayout la(&dlg);
    QLineEdit ed;
    la.addWidget(&ed);
    QDialogButtonBox bb(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    la.addWidget(&bb);
    dlg.setLayout(&la);
    connect(&bb, &QDialogButtonBox::accepted, [&]{
      if (!ed.text().isEmpty()) ui->groupTabPhone->setTabText(index, ed.text());
      dlg.accept();
    });
    connect(&bb, &QDialogButtonBox::rejected, [&]{ dlg.reject(); });
    dlg.exec();
}