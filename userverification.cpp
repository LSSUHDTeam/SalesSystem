#include "userverification.h"
#include "ui_userverification.h"

UserVerification::UserVerification(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::UserVerification)
{
    ui->setupUi(this);
    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));
    ui->passwordField->setEchoMode(QLineEdit::Password);
}

UserVerification::~UserVerification()
{
    delete ui;
}

void UserVerification::processData(QString data)
{
    switch(data.toInt())
    {
    case 1:
        emit accessGranted();
        this->close();
        break;
    case 2:
        ui->infoLabel->setText("Incorrect Password");
        break;
    case 3:
        ui->infoLabel->setText("Unknown User");
        break;
    default:
        break;
    }
}

void UserVerification::on_loginButton_clicked()
{
    QString md5Pass = QString(QCryptographicHash::hash((ui->passwordField->text().toUtf8()),QCryptographicHash::Md5).toHex());
    QString information = ui->usernameField->text() + QUERY_DELIMITER + md5Pass;
    socketConn.SubmitQuery(INFORMATION_QUERY,2,0,information);
}
