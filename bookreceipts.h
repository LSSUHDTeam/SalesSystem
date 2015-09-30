#ifndef BOOKRECEIPTS_H
#define BOOKRECEIPTS_H

#include <QDialog>
#include <QDateTime>
#include "globals.h"
#include "structures.h"
#include "socketclient.h"
#include "salesprinterengine.h"
#include "messagewindow.h"

namespace Ui {
class BookReceipts;
}

class BookReceipts : public QDialog
{
    Q_OBJECT

public:
    explicit BookReceipts(_structs * _dataStructs, QWidget *parent = 0);
    ~BookReceipts();

private slots:
    void on_submitButton_clicked();

    void on_cancelButton_clicked();

    void messageWindowReturn(bool);

    void processData(QString);

private:
    Ui::BookReceipts *ui;

    unsigned messageSwitch;
    bool validDataEntry();
    bool validQuantity();
    bool validPrice();
    bool validTotal();
    void generateMessageBox(QStringList);
    QString update;
};

#endif // BOOKRECEIPTS_H
