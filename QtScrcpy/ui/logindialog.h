#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QNetworkAccessManager>

#define PRODUCT_NAME "remote_phone"

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QString _machineCode, QWidget *parent = nullptr);
    ~LoginDialog();
    bool verifyLicense();

private:
    void on_loginBtn_clicked();
    void on_quitBtn_clicked();

private:
    Ui::LoginDialog *ui;
    QString machineCode;
    QNetworkAccessManager *mgr;
    QNetworkReply *reply;
};

#endif // LOGINDIALOG_H
