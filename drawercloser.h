#ifndef DRAWERCLOSER_H
#define DRAWERCLOSER_H

#include <QDialog>
#include <QDateTime>
#include <QFileDialog>
#include "structures.h"
#include "socketclient.h"
#include "receiptviewer.h"
#include "receiptviewer.h"
#include "messagewindow.h"


namespace Ui {
class DrawerCloser;
}

class DrawerCloser : public QDialog
{
    Q_OBJECT

public:
    explicit DrawerCloser(_structs * dataStructs, QWidget *parent = 0);
    ~DrawerCloser();

public slots:
    void processData(QString);
    void doUpdate();

private slots:

    void on_viewDetails_clicked();

    void on_closeDrawer_clicked();

    void on_receiptTable_clicked(const QModelIndex &index);

    void on_comments_textChanged();


    void on_removeCloseButton_clicked();

private:
    Ui::DrawerCloser *ui;
    _structs * dataStructs;


    socketClient socketConn;
    QDateTime closeStartDateTime, closeEndDateTime;
    double drawerAmount;
    double checkAmount;
    double chargebackAmount;
    double subtotalInDrawer;

    unsigned receiptCount;
    struct receiptStruct _receipts[MAX_RECEIPTS_PER_SELECT];
    unsigned populationSwitch;

    bool textEntrySwitch;
    QStringList reservedCharachters;

    QString filteredText(QString text);
    void populateClosingData();
    void purgeReceipts();
    void updateWindow();


    bool amountFoundIsValid();
};

#endif // DRAWERCLOSER_H
