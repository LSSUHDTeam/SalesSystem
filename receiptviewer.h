#ifndef RECEIPTVIEWER_H
#define RECEIPTVIEWER_H

#include <QDialog>
#include "globals.h"
#include "structures.h"
#include "salesprinterengine.h"

namespace Ui {
class ReceiptViewer;
}

class ReceiptViewer : public QDialog
{
    Q_OBJECT

public:
    explicit ReceiptViewer(struct receiptStruct,QWidget *parent = 0);
    ~ReceiptViewer();

private slots:
    void on_pushButton_clicked();

    void on_printButton_clicked();

private:
    Ui::ReceiptViewer *ui;

    struct receiptStruct currentReceipt;
};

#endif // RECEIPTVIEWER_H
