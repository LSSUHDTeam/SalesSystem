#ifndef SALEWINDOW_H
#define SALEWINDOW_H

#include <QHash>
#include "QDebug"
#include <QDialog>
#include <qmath.h>
#include "globals.h"
#include <QDateTime>
#include "structures.h"
#include "socketclient.h"
#include "messagewindow.h"
#include "salesprinterengine.h"

namespace Ui {
class SaleWindow;
}

class SaleWindow : public QDialog
{
    Q_OBJECT

signals:
    void forceUpdate(int, QString);

public:
    explicit SaleWindow(_structs * _dataStructs, QWidget *parent = 0);
    ~SaleWindow();

public slots:

    void messageWindowReturn(bool);

    void processData(QString);

private slots:
    void on_availableItems_clicked(const QModelIndex &index);

    void on_addItemButton_clicked();

    void on_itemsSold_clicked(const QModelIndex &index);

    void on_removeItemButton_clicked();

    void on_finishOrderButton_clicked();

    void on_availableItems_currentRowChanged(int currentRow);

    void on_cancelButton_clicked();

    void on_itemsSold_currentRowChanged(int currentRow);

private:
    Ui::SaleWindow *ui;

    _structs * dataStructs;

    void updateWindow();
    void updateAvailableItems();
    void updateSoldItems();
    void calculateAndSetPrice();
    void generateMessageBox(QStringList);
    void printOutReceipt(QStringList);
    QString filteredText(QString);

    QHash<QString, int> currentSale;
    QStringList availableItemsList;
    QStringList reservedCharachters;
    socketClient socketConn;
    double currentTotal;
    QString receiptId;
    bool saving;
    bool halt;

};

#endif // SALEWINDOW_H
