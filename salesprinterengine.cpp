#include "salesprinterengine.h"

SalesPrinterEngine::SalesPrinterEngine(unsigned setup, QWidget *parent) :
    QWidget(parent)
{
    instanceType = setup;

}

void SalesPrinterEngine::printData(QStringList data)
{
    // Broken up and determined via instance type
    switch(instanceType)
    {
    case PRINTER_RECEIPT_STANDARD:
        printStandardReceipt(data);
        break;
    default:
        break;
    }
}

void SalesPrinterEngine::printStandardReceipt(QStringList data)
{
    QString page =
    (
     "\t\t\t LSSU TECHNICAL SERVICES RECEIPT \n\n"
     " Receipt ID   \t   : " + data[10] + "\n"
     " Date Created \t : " + data[0]  + "\n"
     " Created By   \t  : " + data[1]  + "\n"
     " Department   \t : " + data[7]  + "\n"
     " Payment Type \t: " + data[8]  + "\n"
     " Code             \t : " + data[9]  + "\n"
     " First Name   \t : " + data[2]  + "\n"
     " Last Name    \t: " + data[3]  + "\n\n "
     + data[4]
     + "\n\n Total        : " + data[5] + "\n\n\n"
     + " Description  :  " + data[6]               );

    QPrinter printer;
    QPainter painter;
    painter.begin(&printer);
    painter.drawText(100, 100, 500, 500, Qt::AlignLeft|Qt::AlignTop, page);
    painter.drawText(100, 300, 500, 500, Qt::AlignLeft|Qt::AlignBottom, page);
    painter.end();
}
