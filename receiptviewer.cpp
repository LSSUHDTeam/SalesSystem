#include "receiptviewer.h"
#include "ui_receiptviewer.h"

ReceiptViewer::ReceiptViewer(struct receiptStruct receipt, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReceiptViewer)
{
    ui->setupUi(this);

    currentReceipt = receipt;

    ui->RecIdLabel->setText(currentReceipt.id);
    ui->sellerLabel->setText(currentReceipt.seller);
    ui->dateCreatedLabel->setText(currentReceipt.dateCreated);
    ui->firstNameLabel->setText(currentReceipt.firstName);
    ui->lastNameLabel->setText(currentReceipt.lastName);
    ui->totalPriceLabel->setText(currentReceipt.totalPrice);
    ui->deptCodeLabel->setText(currentReceipt.deptCode);
    ui->typeLabel->setText(currentReceipt.transactionType);
    ui->codeLabel->setText(currentReceipt.transactionCode);

    foreach(QString item, currentReceipt.itemsSold.split(LIST_DELIMITER))
    {
        ui->itemsSold->insertPlainText("\n ");
        ui->itemsSold->insertPlainText(item);
    }

    if( currentReceipt.description == EMPTY_DB_ENTRY )
    {
        ui->description->insertPlainText("No desription given for this receipt.");
    }
    else
    {
        ui->description->insertPlainText(currentReceipt.description);
    }
}

ReceiptViewer::~ReceiptViewer()
{
    delete ui;
}

void ReceiptViewer::on_pushButton_clicked()
{
    this->close();
}

void ReceiptViewer::on_printButton_clicked()
{
    QStringList data;
    data.append(currentReceipt.dateCreated);
    data.append(currentReceipt.seller);
    data.append(currentReceipt.firstName);
    data.append(currentReceipt.lastName);

    QString x = "";
    foreach(QString item, currentReceipt.itemsSold.split(LIST_DELIMITER))
    {
        QStringList s = item.split("x");
        x += (s[0] + " Qty: " + s[1] + "\n");
    }
    data.append(x);

    data.append(currentReceipt.totalPrice);
    data.append(currentReceipt.description);
    data.append(currentReceipt.deptCode);
    data.append(currentReceipt.transactionType);
    data.append(currentReceipt.transactionCode);
    data.append(currentReceipt.id);

    SalesPrinterEngine printerEngine(PRINTER_RECEIPT_STANDARD, this);
    printerEngine.printData(data);
}
