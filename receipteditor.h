#ifndef RECEIPTEDITOR_H
#define RECEIPTEDITOR_H

#include <QHash>
#include <QDialog>
#include <QDebug>
#include <qmath.h>
#include "globals.h"
#include "structures.h"
#include "socketclient.h"
#include "messagewindow.h"

namespace Ui {
class ReceiptEditor;
}

class ReceiptEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ReceiptEditor(_structs * data,
                           struct receiptStruct* receipt,
                           QWidget *parent = 0);
    ~ReceiptEditor();

signals:
    void forceUpdate();

public slots:
    void messageWindowReturn(bool);

private slots:
    void on_availableItems_clicked(const QModelIndex &index);

    void on_itemsSold_clicked(const QModelIndex &index);

    void on_addItemButton_clicked();

    void on_removeItemButton_clicked();

    void on_availableItems_currentRowChanged(int currentRow);

    void on_itemsSold_currentRowChanged(int currentRow);

    void on_cancelButton_clicked();

    void on_doneButton_clicked();

    void on_voidReceiptButton_clicked();

private:
    Ui::ReceiptEditor *ui;

    bool halt;
    bool saving;
    bool voiding;
    unsigned messageReturnSwitch;
    double currentTotal;
    _structs * dataStructs;
    socketClient socketConn;
    QStringList availableItemsList;
    QStringList reservedCharachters;
    QHash<QString, int> currentSale;
    struct receiptStruct * _receipt;

    void populateWindow();
    void updateSoldItems();
    void calculateAndSetPrice();
    void updateAvailableItems();
    void generateMessageBox(QStringList);
    QString filteredText(QString text);



};

#endif // RECEIPTEDITOR_H
