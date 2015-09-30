#include "librarymanager.h"
#include "ui_librarymanager.h"

LibraryManager::LibraryManager(_structs * _dataStructs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LibraryManager)
{
    ui->setupUi(this);

    dataStructs = _dataStructs;
}

LibraryManager::~LibraryManager()
{
    delete ui;
}

void LibraryManager::on_receiptChargeBackButton_clicked()
{
    BookReceipts * bookReceiptWindow = new BookReceipts(dataStructs ,this);
    bookReceiptWindow->show();
}
