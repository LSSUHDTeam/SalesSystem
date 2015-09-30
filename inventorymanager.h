#ifndef INVENTORYMANAGER_H
#define INVENTORYMANAGER_H

#include <QDialog>
#include <QDebug>
#include "globals.h"
#include "structures.h"
#include "socketclient.h"

namespace Ui {
class InventoryManager;
}

class InventoryManager : public QDialog
{
    Q_OBJECT

signals:
    void forceUpdate(int, QString);

public:
    explicit InventoryManager(_structs * data,QWidget *parent = 0);
    ~InventoryManager();

private slots:
    void on_checkBox_toggled(bool checked);

    void on_cancelButton_clicked();

    void on_addItemButton_clicked();

    void on_clearButton_clicked();

    void processData(QString);

    void on_editDealCheckBox_toggled(bool checked);

    void on_loadButton_clicked();

    void on_cancelEditButton_clicked();

    void on_deleteItemButton_clicked();

    void on_saveAddButton_clicked();

    void on_editCloseButton_clicked();

private:
    Ui::InventoryManager *ui;

    unsigned processSwitch;
    _structs * dataStructs;
    socketClient socketConn;
    bool itemAlreadyExists();
    unsigned priceFieldIsValid();
    unsigned dealIsValid();
    void callServerToAddItem();

    // Edit Page

    void loadEditPage();
    unsigned editPriceFieldIsValid();
    unsigned editDealIsValid();
    void callServerToUpdateItem();
};

#endif // INVENTORYMANAGER_H
