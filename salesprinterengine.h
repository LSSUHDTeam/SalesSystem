#ifndef SALESPRINTERENGINE_H
#define SALESPRINTERENGINE_H

#include <QWidget>
#include <QDebug>
#include <QApplication>
#include "globals.h"
#include <QPrinter>
#include <QPainter>

class SalesPrinterEngine : public QWidget
{
    Q_OBJECT
public:
    explicit SalesPrinterEngine(unsigned setup, QWidget *parent = 0);

signals:

public slots:

    void printData(QStringList);

private:

    unsigned instanceType;


    void printStandardReceipt(QStringList);

};

#endif // SALESPRINTERENGINE_H
