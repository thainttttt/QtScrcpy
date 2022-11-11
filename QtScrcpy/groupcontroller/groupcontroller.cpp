#include <QPointer>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include "groupcontroller.h"
#include "videoform.h"

GroupController::GroupController(QObject *parent) : QObject(parent)
{

}

bool GroupController::isHost(const QString &serial)
{
    auto data = qsc::IDeviceManage::getInstance().getDevice(serial)->getUserData();
    if (!data) {
        return true;
    }

    return static_cast<VideoForm*>(data)->isHost();
}

QSize GroupController::getFrameSize(const QString &serial)
{
    auto data = qsc::IDeviceManage::getInstance().getDevice(serial)->getUserData();
    if (!data) {
        return QSize();
    }

    return static_cast<VideoForm*>(data)->frameSize();
}

GroupController &GroupController::instance()
{
    static GroupController gc;
    return gc;
}

void GroupController::updateDeviceState(const QString &serial)
{
    if (!m_devices.contains(serial)) {
        return;
    }

    auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
    if (!device) {
        return;
    }

    if (isHost(serial)) {
        device->registerDeviceObserver(this);
    } else {
        device->deRegisterDeviceObserver(this);
    }
}

void GroupController::addDevice(const QString &serial)
{
    if (m_devices.contains(serial)) {
        return;
    }

    m_devices.append(serial);
}

void GroupController::removeDevice(const QString &serial)
{
    if (!m_devices.contains(serial)) {
        return;
    }

    m_devices.removeOne(serial);

    auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
    if (!device) {
        return;
    }

    if (isHost(serial)) {
        device->deRegisterDeviceObserver(this);
    }
}

void GroupController::mouseEvent(const QMouseEvent *from, const QSize &frameSize, const QSize &showSize)
{
    Q_UNUSED(frameSize);
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->mouseEvent(from, getFrameSize(serial), showSize);
    }
}

void GroupController::wheelEvent(const QWheelEvent *from, const QSize &frameSize, const QSize &showSize)
{
    Q_UNUSED(frameSize);
    std::vector<QFuture<void> > threads;
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        QFuture<void> thread = QtConcurrent::run([=]() {
            device->wheelEvent(from, getFrameSize(serial), showSize);
        });
        threads.push_back(thread);
    }

    for (auto &thread : threads) thread.waitForFinished();
}

void GroupController::keyEvent(const QKeyEvent *from, const QSize &frameSize, const QSize &showSize)
{
    Q_UNUSED(frameSize);
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->keyEvent(from, getFrameSize(serial), showSize);
    }
}

void GroupController::postGoBack()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postGoBack();
    }
}

void GroupController::postGoHome()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postGoHome();
    }
}

void GroupController::postGoMenu()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postGoMenu();
    }
}

void GroupController::postAppSwitch()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postAppSwitch();
    }
}

void GroupController::postPower()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postPower();
    }
}

void GroupController::postVolumeUp()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postVolumeUp();
    }
}

void GroupController::postVolumeDown()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postVolumeDown();
    }
}

void GroupController::postCopy()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postCopy();
    }
}

void GroupController::postCut()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postCut();
    }
}

void GroupController::setScreenPowerMode(bool open)
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->setScreenPowerMode(open);
    }
}

void GroupController::expandNotificationPanel()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->expandNotificationPanel();
    }
}

void GroupController::collapsePanel()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->collapsePanel();
    }
}

void GroupController::postBackOrScreenOn(bool down)
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postBackOrScreenOn(down);
    }
}

void GroupController::postTextInput(QString &text)
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->postTextInput(text);
    }
}

void GroupController::requestDeviceClipboard()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->requestDeviceClipboard();
    }
}

void GroupController::setDeviceClipboard(bool pause)
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->setDeviceClipboard(pause);
    }
}

void GroupController::clipboardPaste()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->clipboardPaste();
    }
}

void GroupController::pushFileRequest(const QString &file, const QString &devicePath)
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->pushFileRequest(file, devicePath);
    }
}

void GroupController::installApkRequest(const QString &apkFile)
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->installApkRequest(apkFile);
    }
}

void GroupController::screenshot()
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->screenshot();
    }
}

void GroupController::showTouch(bool show)
{
    for (const auto& serial : m_devices) {
        if (true == isHost(serial)) {
            continue;
        }
        auto device = qsc::IDeviceManage::getInstance().getDevice(serial);
        if (!device || !device->enableGroup) {
            continue;
        }

        device->showTouch(show);
    }
}
