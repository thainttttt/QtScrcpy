#include <QDebug>

#include "form.h"
#include "ui_form.h"

Form::Form(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Form)
{
    ui->setupUi(this);
    ui->horizontalLayout->setStretch(1, 1);
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
