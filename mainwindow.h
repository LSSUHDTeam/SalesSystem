#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QTimer>
#include <QSound>
#include "QDebug"
#include "globals.h"
#include <QDateTime>
#include <QMainWindow>
#include "structures.h"
#include "salewindow.h"
#include "socketclient.h"
#include "reportmanager.h"
#include "librarymanager.h"
#include "userverification.h"
#include "inventorymanager.h"
#include "transactionviewer.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void processData(QString);
    void requestUpdate(int, QString);

private slots:
    void on_saleButton_clicked();

    void on_OneGreenScantron_clicked();

    void on_FiveGreenScantrons_clicked();

    void on_BlueBook_clicked();

    void on_OneRedScantron_clicked();

    void on_FiveRedScantrons_clicked();

    void on_OneFolder_clicked();

    void on_OnePencil_clicked();

    void on_OneMPencil_clicked();

    void on_OnePen_clicked();

    void on_OneFiveDollarEarBuds_clicked();

    void on_OneThreeDollarEarbuds_clicked();

    void on_UndoLastSale_clicked();

    void checkForDateChange();

    void on_lookupTransactionButton_clicked();

    void on_reportButton_clicked();

    void user_granted_access();

    void on_inventoryManagementButton_clicked();

    void closeEvent(QCloseEvent *);

    void on_UndoLastSale_2_clicked();

    void on_Sharpie_clicked();

    void on_Eraser_clicked();

    void on_DryEraseMarker_clicked();

    void on_Highlighter_clicked();

    void on_PaddedEnvelope_clicked();

    void on_TapeFlags_clicked();

    void on_PenInk_clicked();

    void on_PencilRefill_clicked();

    void on_LaregeColoredIndexCards_clicked();

    void on_SmallColoredIndexCards_clicked();

    void on_LargeRegularIndexCards_clicked();

    void on_SmallRegularIndexCards_clicked();

    void on_UndoLastSale_3_clicked();

    void on_actionBook_Charge_triggered();

private:
    Ui::MainWindow *ui;

    unsigned access;
    bool purgeData;
    bool firstCheck;
    _structs dataStructs;
    socketClient socketConn;

    void doUpdateChain();
    void purgeStructures(int);
    void populateInventory(QStringList);
    void populateReceipts(QStringList);
    void populateInformation(QString);
    void populateDepartmentInformation(QStringList);

    QTimer updateTimer;
    double runningTotal;
    bool sendingReceipt;
    QDate dateOfReference;
    struct dailySale dailySales;
    QList< QPair<unsigned, int> > salesHistory;

    void updateDailySale(unsigned);
    void calculateAndDisplayDailySales();

    void playSoundFile(unsigned);

};

#endif // MAINWINDOW_H
