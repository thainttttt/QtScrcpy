#include <QMessageBox>
#include <QDialog>
#include <QNetworkRequest>
#include <QJsonObject>
#include <QJsonDocument>
#include <QNetworkReply>
#include <QJsonArray>
#include <QDateTime>
#include <QTimer>

#include "logindialog.h"
#include "ui_logindialog.h"

LoginDialog::LoginDialog(QString _machineCode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    machineCode = _machineCode;
    mgr = new QNetworkAccessManager(this);

    ui->setupUi(this);
    setWindowTitle(tr("Enter a license key"));
    ui->pwdLineEdit->setFocus();

    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &LoginDialog::on_loginBtn_clicked);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &LoginDialog::on_quitBtn_clicked);

}

LoginDialog::~LoginDialog()
{
    delete ui;
}

bool LoginDialog::verifyLicense() {
    QNetworkRequest request(QUrl("https://license.lalasoft.vn/api/checkmachine"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // init data
    QJsonObject obj_data;
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    QString hash = QString(QCryptographicHash::hash(
        (QString("liclalasoft") + machineCode + QString::number(time)).toUtf8(),
        QCryptographicHash::Md5
    ).toHex());
    qDebug() << "Hashhhh" << hash << time;

    // update data
    obj_data["machine_code"] = machineCode;
    obj_data["code"] = QJsonValue::Null;
    obj_data["type_product"] = QJsonValue::Null;
    QJsonObject obj;
    obj["hash"] = hash;
    obj["time"] = time;
    obj["data"] = obj_data;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();

    // send request and wait for response
    QEventLoop loop;
    QTimer timer;    
    timer.setSingleShot(true);
    reply = mgr->post(request, data);
    connect(&timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    connect(reply , SIGNAL(finished()), &loop, SLOT(quit()));
    timer.start(3000);   // 3 secs. timeout
    loop.exec();

    bool accept = false;
    if(timer.isActive()) {
        timer.stop();
        if(reply->error() == QNetworkReply::NoError){
            QString contents = QString::fromUtf8(reply->readAll());
            qDebug() << "contents: " << contents;
            QJsonDocument jsonResponse = QJsonDocument::fromJson(contents.toUtf8());
            QJsonObject jsonObject = jsonResponse.object();
            QJsonArray jsonArray = jsonObject["data"].toArray();

            foreach (const QJsonValue & value, jsonArray) {
                QJsonObject obj = value.toObject();
                if (obj["type_product"].toString() == QString::fromUtf8(PRODUCT_NAME)
                && obj["status"].toBool()) {
                    accept = true;
                    break;
                }
            }
        }
        else {
            QString err = reply->errorString();
            qDebug() << "Error: " << err;
        }
    } else {
        // timeout
        disconnect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
        reply->abort();
    }
    reply->deleteLater();
    if (!accept) qDebug() << "The license is invalid.";
    return accept;
}

void LoginDialog::on_loginBtn_clicked()
{
    QNetworkRequest request(QUrl("https://license.lalasoft.vn/api/code"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // init data
    qint64 time = QDateTime::currentMSecsSinceEpoch();
    QString key = ui->pwdLineEdit->text();
    QString productName = QString::fromUtf8(PRODUCT_NAME);
    QString hash = QString(QCryptographicHash::hash(
        (QString("liclalasoft") + machineCode + key + productName + QString::number(time)).toUtf8(),
        QCryptographicHash::Md5
    ).toHex());

    // update data
    QJsonObject obj_data;
    obj_data["machine_code"] = machineCode;
    obj_data["code"] = key;
    obj_data["type_product"] = productName;
    QJsonObject obj;
    obj["hash"] = hash;
    obj["time"] = time;
    obj["data"] = obj_data;
    QJsonDocument doc(obj);
    QByteArray data = doc.toJson();
    reply = mgr->post(request, data);
    qDebug() << "doc" << doc;

    // process response
    QObject::connect(reply, &QNetworkReply::finished, this, [this](){
        if(reply->error() == QNetworkReply::NoError){
            QString contents = QString::fromUtf8(reply->readAll());
            qDebug() << "contents: " << contents;
            QJsonDocument jsonResponse = QJsonDocument::fromJson(contents.toUtf8());
            QJsonObject jsonObject = jsonResponse.object();

            bool jsonArray = jsonObject["status"].toBool();
            Q_UNUSED(jsonArray);
            if (!jsonArray) {
            // if (0) {
                QMessageBox::warning(this, "License is invalid", "Please make sure you have a valid license.", QMessageBox::Ok);
                ui->pwdLineEdit->clear();
                ui->pwdLineEdit->setFocus();
            } else QDialog::accept();
        }
        else {
            QMessageBox::warning(this, "Cannot verify license", "Please check your internet connection and try again.", QMessageBox::Ok);
            ui->pwdLineEdit->clear();
            ui->pwdLineEdit->setFocus();

            QString err = reply->errorString();
            qDebug() << "Error: " << err;
        }
        reply->deleteLater();
    });
}

void LoginDialog::on_quitBtn_clicked()
{
    QDialog::reject();
}
