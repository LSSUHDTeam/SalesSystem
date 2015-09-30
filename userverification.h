#ifndef USERVERIFICATION_H
#define USERVERIFICATION_H

#include <QDialog>
#include "globals.h"
#include "socketclient.h"
#include <QCryptographicHash>

namespace Ui {
class UserVerification;
}

class UserVerification : public QDialog
{
    Q_OBJECT

signals:
    void accessGranted();

public:
    explicit UserVerification(QWidget *parent = 0);
    ~UserVerification();

public slots:
    void processData(QString);

private slots:
    void on_loginButton_clicked();

private:
    Ui::UserVerification *ui;
    socketClient socketConn;
};

#endif // USERVERIFICATION_H
