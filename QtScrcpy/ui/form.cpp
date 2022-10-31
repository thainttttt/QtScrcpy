#include <QDebug>
#include <QShortcut>

#include "form.h"
#include "ui_form.h"

Form::Form(int maxRow, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    _maxRow = maxRow;
    ui->setupUi(this);
    ui->horizontalLayout->setStretch(1, 1);
    // installShortcut();

    // setDeviceClipboard
    auto shortcut = new QShortcut(QKeySequence("Ctrl+Shift+v"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        if (currentForm.isEmpty()) return;
        auto device = qsc::IDeviceManage::getInstance().getDevice(currentForm);
        if (!device) {
            return;
        }
        emit device->clipboardPaste();
    });
}

Form::~Form()
{
    delete ui;
}

void Form::unsetMainForm() {
    if (ui->horizontalLayout->count() == 2) {
        ui->horizontalLayout->takeAt(0)->widget();
    }
}

void Form::setMainForm(QWidget *w)
{
    // remove item from grid widget
    int idx = ui->gridLayout_2->indexOf(w);
    QLayoutItem *child;
    child = ui->gridLayout_2->takeAt(idx);
    delete child;   // delete the layout item

    ui->horizontalLayout->insertWidget(0, w);

    // reset
    std::vector<QLayoutItem*> tmpItems;
    tmpItems.reserve(ui->gridLayout_2->count());
    while ((child=ui->gridLayout_2->takeAt(0)) != nullptr) {
        tmpItems.push_back(child);
    }
    for (auto& item : tmpItems) {
        addForm(item->widget());
    }
}

void Form::addForm(QWidget *w) {
    if (!w) {
        return;
    }
    w->setParent(this);
    w->show();

    int col = ui->gridLayout_2->count() / _maxRow;
    int row = ui->gridLayout_2->count() % _maxRow;

    ui->gridLayout_2->setColumnStretch(col, 0);
    ui->gridLayout_2->setColumnStretch(col+1, 1);
    ui->gridLayout_2->addWidget(w, row, col, 1, 1);
}

void Form::resetForm() {
    QLayoutItem *child;
    while ((child=ui->gridLayout_2->takeAt(0)) != nullptr) {
        child->widget()->hide();
        delete child;
    }

    unsetMainForm();
    if (!mainSerial.isEmpty()) {
        auto oldMainForm = videoForms[mainSerial.toStdString()];
        if (oldMainForm->isHost())
            oldMainForm->updateGroupState();
        oldMainForm->hide();
        mainSerial.clear();
    }
}

void Form::setRow(int row) {
    _maxRow = row;
}

void Form::setCurrentForm(QString &serial) {
    currentForm = serial;
}

void Form::keyPressEvent(QKeyEvent *event) {
    if (currentForm.isEmpty()) return;

    // QMouseEvent event(QEvent::MouseButtonPress, pos, 0, 0, 0);
    // QKeyPressEvent *event2 = static_cast<QKeyPressEvent>

    QApplication::sendEvent(videoForms[currentForm.toStdString()], event);
}

void Form::keyReleaseEvent(QKeyEvent *event) {
    if (currentForm.isEmpty()) return;

    QApplication::sendEvent(videoForms[currentForm.toStdString()], event);
}

bool Form::updateMainForm(QString& serial) {
    unsetMainForm();

    if (!mainSerial.isEmpty()) {
        auto oldMainForm = videoForms[mainSerial.toStdString()];
        if (oldMainForm->isHost())
            oldMainForm->updateGroupState();
        addForm(oldMainForm);

        if (mainSerial == serial) {
            mainSerial.clear();
            return false;
        }
    }
    auto mainForm = videoForms[serial.toStdString()];
    setMainForm(mainForm);
    mainForm->updateGroupState();

    mainSerial = serial;

    return true;
}

void Form::installShortcut() {
    QShortcut *shortcut = nullptr;

    // setDeviceClipboard
    shortcut = new QShortcut(QKeySequence("Ctrl+Shift+v"), this);
    shortcut->setAutoRepeat(false);
    connect(shortcut, &QShortcut::activated, this, [this]() {
        qInfo() << "aaaaaaaaaaaaaaaa";
        auto device = qsc::IDeviceManage::getInstance().getDevice(currentForm);
        if (!device) {
            return;
        }
        emit device->clipboardPaste();
    });
}
