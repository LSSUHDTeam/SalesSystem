#ifndef TRANSACTIONVIEWER_H
#define TRANSACTIONVIEWER_H

#include <QDate>
#include <QDebug>
#include <QDialog>
#include "globals.h"
#include <QFileDialog>
#include "structures.h"
#include "socketclient.h"
#include "receiptviewer.h"
#include "receipteditor.h"

namespace Ui {
class TransactionViewer;
}

class TransactionViewer : public QDialog
{
    Q_OBJECT

public:
    explicit TransactionViewer(_structs * data, QWidget *parent = 0);
    ~TransactionViewer();

public slots:
    void requestUpdateOnCurrentDate();

private slots:

    void processData(QString);

    void on_searchButton_clicked();

    void on_viewDetailsButton_clicked();

    void on_receiptTable_clicked(const QModelIndex &index);

    void on_closeButton_clicked();

    void on_editButton_clicked();

    void on_searchByRange_toggled(bool checked);

    void on_exportButton_clicked();

private:
    Ui::TransactionViewer *ui;

    void updateTable();
    void purgeStructures();
    void retrieveDataFromServer();

    QDate dateToLookup;
    _structs * dataStructures;
    unsigned receiptIndex, receiptCount;
    struct receiptStruct currentReceipts[MAX_RECEIPTS_PER_SELECT];

    socketClient socketConn;
};

#endif // TRANSACTIONVIEWER_H
