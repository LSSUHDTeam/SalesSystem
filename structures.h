#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QWidget>
#include "globals.h"

struct receiptStruct
{
    QString id, dateCreated, seller, firstName, lastName,
    itemsSold, totalPrice, description ,deptCode, transactionType, transactionCode, itemsSoldRaw;

    void operator =(struct receiptStruct &x)
    {
        this->id = x.id;
        this->dateCreated = x.dateCreated;
        this->seller = x.seller;
        this->firstName = x.firstName;
        this->lastName = x.lastName;
        this->itemsSold = x.itemsSold;
        this->totalPrice = x.totalPrice;
        this->description = x.description;
        this->deptCode = x.deptCode;
        this->transactionType = x.transactionType;
        this->transactionCode = x.transactionCode;
    }
};

struct inventoryStruct
{
    QString inventoryId, itemName, description,deal;
    double price;
    int quantity;
};

struct counterStruct
{
    int departmentCount, departmentIndex;
    int receiptCount, inventoryCount;
    int receiptIndex, inventoryIndex;
};

struct informationStruct
{
    double salesTax;
    QStringList codes, types;
};

struct fopal
{
    QString  fund, organization, account, program;
};

struct departmentInformation
{
    QString id, name;
    int previousFopals;
    struct fopal deptFopal;
    struct fopal previousFopalList[MAX_PREVIOUS_FOPALS];
    QString extra;
};

struct saleInformaton
{
    // For tacking which button is clicked in quick sales
    unsigned saleId;
    QString inventoryId;
    double price;
    int qtySold;
    QString deal;
};

struct saleObject
{
    QString inventoryId;
    double price;

    void operator =(struct saleInformaton &x)
    {
        this->inventoryId = x.inventoryId;
        this->price = x.price;
    }

    void operator =(struct saleObject &x)
    {
        this->inventoryId = x.inventoryId;
        this->price = x.price;
    }

    bool operator ==(struct saleObject &x)
    {
        if( this->inventoryId == x.inventoryId )
            return true;
        else
            return false;
    }
};

struct dailySale
{
    int totalSales;
    double runningTotal;
    unsigned qsoIndex;
    struct saleInformaton quickSaleObjects[NUMBER_OF_QUICKSALES];
    struct saleObject todaysSales[MAX_DAILY_SALES];

};

struct ClosingEntry
{
    QString id, startDate, endDate, calculatedAmount, drawerAmount,
            comments;
};

/*
            ENABLE PREVIOUS FOPAL LIMIT IN DB.. MAKE SO IT TRIMS OLDEST ONCE N NUMBER IS REACHED

*/

struct dataStructures
{
    struct receiptStruct receipts[MAX_RECEIPTS_PER_SELECT];
    struct inventoryStruct inventory[MAX_INVENTORY_ITEMS];
    struct counterStruct counters;
    struct informationStruct saleInformation;
    struct departmentInformation departments[MAX_DEPARTMENTS];
};

typedef dataStructures _structs;

/*      Common Routines     */

QString subroutine_filterText(QString);

#endif // STRUCTURES_H
