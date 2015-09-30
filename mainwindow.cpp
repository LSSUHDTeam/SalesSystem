#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    // Ui Setup
    ui->setupUi(this);
    ui->saleButton->setEnabled(false);
    ui->lookupTransactionButton->setEnabled(false);

    // Daily sale initialization
    dailySales.qsoIndex = 0;
    dailySales.totalSales = 0;
    dailySales.runningTotal = 0.00;

    // Socket connection and initial updating of data structures
    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));

    // Keep this order.
    purgeData = false;
    firstCheck = true;
    doUpdateChain();
    firstCheck = false;

    // Updater initialization
    connect(&updateTimer, SIGNAL(timeout()), this, SLOT(checkForDateChange()));
    dateOfReference = QDate::currentDate();
    updateTimer.start(TIMER_DAILY_RECEIPTS);
    sendingReceipt = false;

    // Window settings
    this->setWindowFlags(Qt::FramelessWindowHint);
    this->setWindowFlags(Qt::WindowTitleHint);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::doUpdateChain()
{
    if( purgeData )
    {
        purgeStructures(PURGE_INVENTORY);
        purgeStructures(PURGE_RECEIPTS);
        purgeStructures(PURGE_INFORMATION);
        purgeStructures(PURGE_DEPARTMENTS);
    }
    socketConn.SubmitQuery(INVENTORY_QUERY, 0);
    socketConn.SubmitQuery(INFORMATION_QUERY, 0);
    socketConn.SubmitQuery(INFORMATION_QUERY, 1);
}

void MainWindow::processData(QString data)
{
    QStringList dataList = data.split(INCOMING_RESULT_DELIMITER);
    switch (socketConn.lastQuery.first)
    {
    case RECEIPT_QUERY:

        if( !sendingReceipt)
            populateReceipts(dataList);
        else
            sendingReceipt = false;

        break;
    case INVENTORY_QUERY:
        populateInventory(dataList);
        break;
    case INFORMATION_QUERY:
        if( socketConn.lastQuery.second == 0)
            populateInformation(data);

        if( socketConn.lastQuery.second == 1)
        {
            populateDepartmentInformation(dataList);
            ui->saleButton->setEnabled(true);
            ui->lookupTransactionButton->setEnabled(true);
        }
        break;
    default:
        break;
    }
}

void MainWindow::purgeStructures(int purgeLevel)
{
    int i;
    switch(purgeLevel)
    {
    case PURGE_INVENTORY:
    {
        for( i = 0; i < dataStructs.counters.inventoryCount; i++ )
        {
            dataStructs.inventory[i].description = "";
            dataStructs.inventory[i].inventoryId = "";
            dataStructs.inventory[i].itemName = "";
            dataStructs.inventory[i].price = 0.00;
            dataStructs.inventory[i].quantity = 0;
            dataStructs.inventory[i].deal = "";
        }
        dataStructs.counters.inventoryCount = 0;
        break;
    }
    case PURGE_RECEIPTS:
    {
        for( i = 0; i < dataStructs.counters.receiptCount; i++ )
        {
            dataStructs.receipts[i].totalPrice = "";
            dataStructs.receipts[i].seller = "";
            dataStructs.receipts[i].lastName = "";
            dataStructs.receipts[i].itemsSold = "";
            dataStructs.receipts[i].id = "";
            dataStructs.receipts[i].firstName = "";
            dataStructs.receipts[i].description = "";
            dataStructs.receipts[i].deptCode = "";
            dataStructs.receipts[i].dateCreated = "";
            dataStructs.receipts[i].transactionType = "";
            dataStructs.receipts[i].transactionCode = "";
        }
        dataStructs.counters.receiptCount = 0;
        break;
    }
    case PURGE_INFORMATION:
    {
        dataStructs.saleInformation.codes.clear();
        dataStructs.saleInformation.types.clear();
        dataStructs.saleInformation.salesTax = 0.00;
        break;
    }
    case PURGE_DEPARTMENTS:
    {
        for( i = 0; i < dataStructs.counters.departmentCount; i++ )
        {
            dataStructs.departments[i].id = "";
            dataStructs.departments[i].name = "";
            dataStructs.departments[i].deptFopal.fund = "";
            dataStructs.departments[i].deptFopal.organization = "";
            dataStructs.departments[i].deptFopal.program = "";
            dataStructs.departments[i].deptFopal.account = "";
            dataStructs.departments[i].extra = "";

            for( int j = 0; j < dataStructs.departments[i].previousFopals; j++)
            {
                dataStructs.departments[i].previousFopalList[j].fund = "";
                dataStructs.departments[i].previousFopalList[j].organization = "";
                dataStructs.departments[i].previousFopalList[j].program = "";
                dataStructs.departments[i].previousFopalList[j].account = "";
            }
            dataStructs.counters.departmentCount = 0;
            dataStructs.departments[i].previousFopals = 0;
        }
        break;
    }
    default:
        break;
    }
}

void MainWindow::populateInventory(QStringList data)
{
    dataStructs.counters.inventoryCount = 0;
    foreach(QString el, data)
    {
        QStringList cells = el.split(RESULT_MEMBER_DELIMITER);
       // if( cells[4].toInt() > 0 )
       // {
            dataStructs.inventory[dataStructs.counters.inventoryCount].inventoryId = cells[0];
            dataStructs.inventory[dataStructs.counters.inventoryCount].itemName = cells[1];
            dataStructs.inventory[dataStructs.counters.inventoryCount].description = cells[2];
            dataStructs.inventory[dataStructs.counters.inventoryCount].price = cells[3].toDouble();
            dataStructs.inventory[dataStructs.counters.inventoryCount].quantity = cells[4].toInt();

            if( cells[5] != EMPTY_DB_ENTRY )
            {
                dataStructs.inventory[dataStructs.counters.inventoryCount].deal = cells[5];
            }
            else
            {
                dataStructs.inventory[dataStructs.counters.inventoryCount].deal = EMPTY_DB_ENTRY;
            }

            // On first check populate the quicksale buttons.
            if ( firstCheck )
            {
                unsigned neg = -1;
                unsigned qsid = cells[6].toInt();
                if( qsid != neg )
                {
                    dailySales.quickSaleObjects[dailySales.qsoIndex].saleId = qsid;
                    dailySales.quickSaleObjects[dailySales.qsoIndex].inventoryId = cells[0];
                    dailySales.quickSaleObjects[dailySales.qsoIndex].price = cells[3].toDouble();
                    dailySales.quickSaleObjects[dailySales.qsoIndex].qtySold = 0;
                    dailySales.quickSaleObjects[dailySales.qsoIndex].deal = cells[5];
                    dailySales.qsoIndex++;
                }
            }
            dataStructs.counters.inventoryCount++;
        //}
    }
}

void MainWindow::populateReceipts(QStringList data)
{
    dataStructs.counters.receiptCount = 0;
    foreach(QString el, data)
    {
        QStringList cells = el.split(RESULT_MEMBER_DELIMITER);
        dataStructs.receipts[dataStructs.counters.receiptCount].id = cells[0];
        dataStructs.receipts[dataStructs.counters.receiptCount].dateCreated = cells[1];
        dataStructs.receipts[dataStructs.counters.receiptCount].seller = cells[2];
        dataStructs.receipts[dataStructs.counters.receiptCount].firstName = cells[3];
        dataStructs.receipts[dataStructs.counters.receiptCount].lastName = cells[4];
        dataStructs.receipts[dataStructs.counters.receiptCount].itemsSold = cells[5];
        dataStructs.receipts[dataStructs.counters.receiptCount].totalPrice = cells[6];
        dataStructs.receipts[dataStructs.counters.receiptCount].description = cells[7];
        dataStructs.receipts[dataStructs.counters.receiptCount].deptCode = cells[8];
        dataStructs.receipts[dataStructs.counters.receiptCount].transactionType = cells[9];
        dataStructs.receipts[dataStructs.counters.receiptCount].transactionCode = cells[10];

        dataStructs.counters.receiptCount++;
    }
}

void MainWindow::populateInformation(QString data)
{
   QStringList cells = data.split(RESULT_MEMBER_DELIMITER);
   dataStructs.saleInformation.salesTax = cells[1].toDouble();
   dataStructs.saleInformation.codes = cells[2].split(LIST_DELIMITER);
   dataStructs.saleInformation.types = cells[3].split(LIST_DELIMITER);
}

void MainWindow::populateDepartmentInformation(QStringList data)
{
    dataStructs.counters.departmentCount = 0;
  foreach(QString department, data)
    {
        QStringList cells = department.split(RESULT_MEMBER_DELIMITER);
        dataStructs.departments[dataStructs.counters.departmentCount].id = cells[0];
        dataStructs.departments[dataStructs.counters.departmentCount].name = cells[1];
        dataStructs.departments[dataStructs.counters.departmentCount].deptFopal.fund = cells[2];
        dataStructs.departments[dataStructs.counters.departmentCount].deptFopal.organization = cells[3];
        dataStructs.departments[dataStructs.counters.departmentCount].deptFopal.account = cells[4];

        if( cells.count() > 5 )
        {
            dataStructs.departments[dataStructs.counters.departmentCount].deptFopal.program = cells[5];
            if( cells.count() > 6 )
            {
                if( cells[6] == EMPTY_DB_ENTRY )
                {
                     dataStructs.departments[dataStructs.counters.departmentCount].previousFopals = 0;
                }
                else
                {
                    // If the fopal has changed, then the old fopal will be listed under column "previousFopals" in a delimited list.
                    // This else statement chops up the list and loads into the data structure under departments[index].previousFopals[index2]
                    int j = 0;
                    QStringList oldFopals = cells[6].split(FOPAL_LIST_DELIMITER);

                    if( oldFopals.count() > 0 )
                    {
                        foreach(QString fopal, oldFopals)
                        {
                            QStringList fopalCells = fopal.split(PACKAGE_DELIMITER);
                            dataStructs.departments[dataStructs.counters.departmentCount].previousFopalList[j].fund = fopalCells[0];
                            dataStructs.departments[dataStructs.counters.departmentCount].previousFopalList[j].organization = fopalCells[1];
                            dataStructs.departments[dataStructs.counters.departmentCount].previousFopalList[j].program = fopalCells[2];
                            dataStructs.departments[dataStructs.counters.departmentCount].previousFopalList[j].account = fopalCells[3];
                            j++;
                        }
                        dataStructs.departments[dataStructs.counters.departmentCount].previousFopals = j;
                    }
                }
            }
        }

        if( cells[7] != EMPTY_DB_ENTRY)
             dataStructs.departments[dataStructs.counters.departmentCount].extra = cells[7];
        else
            dataStructs.departments[dataStructs.counters.departmentCount].extra = EMPTY_DB_ENTRY;

        dataStructs.counters.departmentCount++;
    }
}

void MainWindow::requestUpdate(int type, QString receiptSpecifier = "")
{
    switch (type)
    {
    // Update Inventory
    case UPDATE_INVENTORY:
    {
        purgeStructures(PURGE_INVENTORY);
        socketConn.SubmitQuery(INVENTORY_QUERY, 0);
        break;
    }
    // Update Receipts by dateRange
    case UPDATE_RECEIPTS:
    {
        purgeStructures(PURGE_RECEIPTS);
        socketConn.SubmitQuery(RECEIPT_QUERY,0,0,receiptSpecifier);
        break;
    }
    // Update Sales Information
    case UPDATE_INFORMATION:
    {
        purgeStructures(PURGE_INFORMATION);
        socketConn.SubmitQuery(INFORMATION_QUERY, 0);
        break;
    }
    case UPDATE_DEPARTMENT_INFORMATION:
    {
        purgeStructures(PURGE_DEPARTMENTS);
        socketConn.SubmitQuery(INFORMATION_QUERY, 1);
        break;
    }
    // Update All - Erases all data and updates only sales info and inventory
    case UPDATE_ALL_AND_PURGE_RECEIPTS:
    {
        purgeData = true;
        doUpdateChain();
        break;
    }
    default:
        break;
    }
}

void MainWindow::on_saleButton_clicked()
{
    SaleWindow * newSale = new SaleWindow(&dataStructs, this);
    connect(newSale, SIGNAL(forceUpdate(int, QString)), this, SLOT(requestUpdate(int,QString)));
    newSale->setAttribute( Qt::WA_DeleteOnClose, true );
    newSale->show();
}

void MainWindow::on_lookupTransactionButton_clicked()
{
    qDebug() << " WORKING ON void MainWindow::on_lookupTransactionButton_clicked() \n"
             << " Making a lookup manager to perform different types of lookups.";


    TransactionViewer * tviewer = new TransactionViewer(&dataStructs, this);
    tviewer->setAttribute( Qt::WA_DeleteOnClose, true );
    tviewer->show();
}

/*


                QUICK SALE OPERATIONS - BUTTON CLICKS AND DAILY SALE MANAGEMENT


*/

void MainWindow::updateDailySale(unsigned id)
{
    QPair<unsigned, int> sale;

    switch (id)
    {
    // One Green Scantron sold
    case QUICKSALE_GREEN_SCANTRON:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_GREEN_SCANTRON )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->OneGreenScantronLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                ui->FiveGreenScantronsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_GREEN_SCANTRON;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_FIVE_GREEN_SCANTRONS:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            // Uses QUICKSALE_GREEN_SCANTRON because it is the same object, just higher qty
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_GREEN_SCANTRON )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].deal.split(DEAL_DELIMITER)[1].toDouble();
                dailySales.quickSaleObjects[i].qtySold+=5;

                ui->OneGreenScantronLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                ui->FiveGreenScantronsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_FIVE_GREEN_SCANTRONS;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_BLUEBOOK:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_BLUEBOOK )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->BlueBookLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_BLUEBOOK;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_RED_SCANTRON:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_RED_SCANTRON )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->OneRedScantronLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                ui->FiveRedScantronsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_RED_SCANTRON;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_FIVE_RED_SCANTRONS:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            // Uses QUICKSALE_RED_SCANTRON because it is the same object, just higher qty
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_RED_SCANTRON )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].deal.split(DEAL_DELIMITER)[1].toDouble();
                dailySales.quickSaleObjects[i].qtySold+=5;

                ui->OneRedScantronLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                ui->FiveRedScantronsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_FIVE_RED_SCANTRONS;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_FOLDER:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_FOLDER )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->OneFolderLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_FOLDER;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_REG_PENCIL:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_REG_PENCIL )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->OnePencilLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_REG_PENCIL;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_M_PENCIL:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_M_PENCIL )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->OneMPencilLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_M_PENCIL;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_PEN:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_PEN )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->OnePenLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_PEN;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_FIVE_DOLLAR_EARBUDS:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_FIVE_DOLLAR_EARBUDS )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->OneFiveDollarEarbudsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_FIVE_DOLLAR_EARBUDS;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_THREE_DOLLAR_EARBUDS:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_THREE_DOLLAR_EARBUDS )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->OneThreeDollarEarbudsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_THREE_DOLLAR_EARBUDS;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }

    // NEW ITEMS
    case QUICKSALE_SHARPIE:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_SHARPIE )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->SharpieLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_SHARPIE;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_ERASER:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_ERASER )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->EraserLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_ERASER;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_DRY_ERASE_MARKER:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_DRY_ERASE_MARKER )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->DryEraseMarkerLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_DRY_ERASE_MARKER;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_HIGHLIGHTER:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_HIGHLIGHTER )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->HighlighterLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_HIGHLIGHTER;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_PADDED_ENVELOPE:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_PADDED_ENVELOPE )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->PaddedEnvelopeLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_PADDED_ENVELOPE;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_TAPE_FLAGS:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_TAPE_FLAGS )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->TapeFlagsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_TAPE_FLAGS;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_PENCILREFIL:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_PENCILREFIL )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->PencilRefillLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_PENCILREFIL;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_PENINK:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_PENINK )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->PenInkLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_PENINK;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }



    case QUICKSALE_ICARD_CL_L:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_ICARD_CL_L )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->largeColoredIndexCardsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_ICARD_CL_L;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_ICARD_R_L:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_ICARD_R_L )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->LargeRegularIndexCardsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_ICARD_R_L;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_ICARD_CL_S:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_ICARD_CL_S )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->SmalleColoredIndexCardsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_ICARD_CL_S;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }
    case QUICKSALE_ICARD_R_S:
    {
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].saleId == QUICKSALE_ICARD_R_S )
            {
                dailySales.todaysSales[dailySales.totalSales] = dailySales.quickSaleObjects[i];
                runningTotal += dailySales.quickSaleObjects[i].price;
                dailySales.quickSaleObjects[i].qtySold++;

                ui->SmallRegularIndexCardsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));

                sale.first = QUICKSALE_ICARD_R_S;
                sale.second = dailySales.totalSales;
                salesHistory.append(sale);

                dailySales.totalSales++;
            }
        }
        break;
    }



    case QUICKSALE_UNDO:
    {
        bool removed = false;
        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            // Last element of salesHistory, first element of tuple

            unsigned id = salesHistory.last().first;

            // Groups of 5 assigned under one, with qty of 5,
            // thus they would not be found by this id.
            if( id == QUICKSALE_FIVE_RED_SCANTRONS )
                id = QUICKSALE_RED_SCANTRON;
            if( id == QUICKSALE_FIVE_GREEN_SCANTRONS )
                id = QUICKSALE_GREEN_SCANTRON;


            if( dailySales.quickSaleObjects[i].saleId == id )
            {
                if( salesHistory.last().first == QUICKSALE_FIVE_RED_SCANTRONS || salesHistory.last().first == QUICKSALE_FIVE_GREEN_SCANTRONS )
                {
                    dailySales.quickSaleObjects[i].qtySold -= 5;
                    runningTotal -= dailySales.quickSaleObjects[i].deal.split(DEAL_DELIMITER)[1].toDouble();
                }
                else
                {
                    dailySales.quickSaleObjects[i].qtySold--;
                    runningTotal -=  dailySales.quickSaleObjects[i].price;
                }

                dailySales.todaysSales[salesHistory.last().second].inventoryId = "";
                dailySales.todaysSales[salesHistory.last().second].price = 0.00;
                dailySales.totalSales--;

                removed = true;

                switch(salesHistory.last().first)
                {
                case QUICKSALE_GREEN_SCANTRON:
                    ui->OneGreenScantronLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    ui->FiveGreenScantronsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_FIVE_GREEN_SCANTRONS:
                    ui->OneGreenScantronLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    ui->FiveGreenScantronsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_BLUEBOOK:
                    ui->BlueBookLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_RED_SCANTRON:
                    ui->OneRedScantronLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    ui->FiveRedScantronsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_FIVE_RED_SCANTRONS:
                    ui->OneRedScantronLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    ui->FiveRedScantronsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_FOLDER:
                    ui->OneFolderLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_REG_PENCIL:
                    ui->OnePencilLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_M_PENCIL:
                    ui->OneMPencilLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_PEN:
                    ui->OnePenLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_FIVE_DOLLAR_EARBUDS:
                    ui->OneFiveDollarEarbudsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_THREE_DOLLAR_EARBUDS:
                    ui->OneThreeDollarEarbudsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;

                case QUICKSALE_SHARPIE:
                    ui->SharpieLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_ERASER:
                    ui->EraserLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_DRY_ERASE_MARKER:
                    ui->DryEraseMarkerLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_HIGHLIGHTER:
                    ui->HighlighterLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_PADDED_ENVELOPE:
                    ui->PaddedEnvelopeLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;

                case QUICKSALE_TAPE_FLAGS:
                    ui->TapeFlagsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_PENCILREFIL:
                    ui->PencilRefillLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_PENINK:
                    ui->PenInkLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_ICARD_CL_L:
                    ui->largeColoredIndexCardsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_ICARD_R_L:
                    ui->LargeRegularIndexCardsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_ICARD_CL_S:
                    ui->SmalleColoredIndexCardsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;
                case QUICKSALE_ICARD_R_S:
                    ui->SmallRegularIndexCardsLabel->setText(("Qty Sold : " + QString::number(dailySales.quickSaleObjects[i].qtySold) ));
                    break;

                default:
                    break;
                }
            }
        }
        if(removed)
        {
            salesHistory.takeLast();
        }
        break;
    }
    default:
        break;
    }
    calculateAndDisplayDailySales();
}

void MainWindow::calculateAndDisplayDailySales()
{
    ui->todaysTotalLabel->setText( "$ " + QString::number(runningTotal, 'f', 2));
}

void MainWindow::on_OneGreenScantron_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_GREEN_SCANTRON);
}

void MainWindow::on_FiveGreenScantrons_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_FIVE_GREEN_SCANTRONS);
}

void MainWindow::on_BlueBook_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_BLUEBOOK);
}

void MainWindow::on_OneRedScantron_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_RED_SCANTRON);
}

void MainWindow::on_FiveRedScantrons_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_FIVE_RED_SCANTRONS);
}

void MainWindow::on_OneFolder_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_FOLDER);
}

void MainWindow::on_OnePencil_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_REG_PENCIL);
}

void MainWindow::on_OneMPencil_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_M_PENCIL);
}

void MainWindow::on_OnePen_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_PEN);
}

void MainWindow::on_OneFiveDollarEarBuds_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_FIVE_DOLLAR_EARBUDS);
}

void MainWindow::on_OneThreeDollarEarbuds_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_THREE_DOLLAR_EARBUDS);
}

void MainWindow::on_Sharpie_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_SHARPIE);
}

void MainWindow::on_Eraser_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_ERASER);
}

void MainWindow::on_DryEraseMarker_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_DRY_ERASE_MARKER);
}

void MainWindow::on_Highlighter_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_HIGHLIGHTER);
}

void MainWindow::on_PaddedEnvelope_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_PADDED_ENVELOPE);
}

void MainWindow::on_TapeFlags_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_TAPE_FLAGS);
}

void MainWindow::on_PenInk_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_PENINK);
}

void MainWindow::on_PencilRefill_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_PENCILREFIL);
}

void MainWindow::on_LaregeColoredIndexCards_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_ICARD_CL_L);
}

void MainWindow::on_SmallColoredIndexCards_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_ICARD_CL_S);
}

void MainWindow::on_LargeRegularIndexCards_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_ICARD_R_L);
}

void MainWindow::on_SmallRegularIndexCards_clicked()
{
    playSoundFile(PLAY_CHOOSE_BUTTON);
    updateDailySale(QUICKSALE_ICARD_R_S);
}

void MainWindow::on_UndoLastSale_clicked()
{
    if( salesHistory.size() > 0 )
    {
        playSoundFile(PLAY_UNDO_BUTTON);
        updateDailySale(QUICKSALE_UNDO);
    }
}

void MainWindow::on_UndoLastSale_2_clicked()
{
    if( salesHistory.size() > 0 )
    {
        playSoundFile(PLAY_UNDO_BUTTON);
        updateDailySale(QUICKSALE_UNDO);
    }
}

void MainWindow::on_UndoLastSale_3_clicked()
{
    playSoundFile(PLAY_UNDO_BUTTON);
    updateDailySale(QUICKSALE_UNDO);
}

void MainWindow::checkForDateChange()
{
    QDate now = QDate::currentDate();

    if( now > dateOfReference )
    {
        if( runningTotal > 0.00 )
        {
            now = now.addDays(-1);

            QString timeStamp = now.toString(STANDARD_DATE_FORMAT_2) + " 23:59:59";

            QString update = timeStamp; update += RESULT_MEMBER_DELIMITER;
            update += "DAILY RECEIPT"; update += RESULT_MEMBER_DELIMITER;
            update += EMPTY_DB_ENTRY; update += RESULT_MEMBER_DELIMITER;
            update += EMPTY_DB_ENTRY; update += RESULT_MEMBER_DELIMITER;

            for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
            {
                if( dailySales.quickSaleObjects[i].qtySold > 0 )
                {
                    update += dailySales.quickSaleObjects[i].inventoryId;
                    update += PACKAGE_DELIMITER;
                    update += QString::number(dailySales.quickSaleObjects[i].qtySold);
                    update += LIST_DELIMITER;
                }
            }
            update = update.left(update.length()-1);
            update += RESULT_MEMBER_DELIMITER;
            update += QString::number(runningTotal, 'f', 2);
            update += RESULT_MEMBER_DELIMITER;
            update += EMPTY_DB_ENTRY;
            update += RESULT_MEMBER_DELIMITER;
            update += EMPTY_DB_ENTRY;
            update += RESULT_MEMBER_DELIMITER;
            update += EMPTY_DB_ENTRY;
            update += RESULT_MEMBER_DELIMITER;
            update += EMPTY_DB_ENTRY;
            update += RESULT_MEMBER_DELIMITER;
            update = update.left(update.length()-1);

            sendingReceipt = true;

            socketConn.SubmitQuery(RECEIPT_QUERY,1,0,update);

            // Reset Sales
            for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
            {
                dailySales.quickSaleObjects[i].qtySold = 0;
                salesHistory.clear();
            }

            for(int i=0; i< dailySales.totalSales; i++ )
            {
                dailySales.todaysSales[i].inventoryId = "";
                dailySales.todaysSales[i].price = 0.00;
            }
            dailySales.totalSales = 0;
            dailySales.runningTotal = 0.00;
            runningTotal = 0.00;

            // Reset user interface
            ui->OneGreenScantronLabel->setText(" ");
            ui->FiveGreenScantronsLabel->setText(" ");
            ui->BlueBookLabel->setText(" ");
            ui->OneRedScantronLabel->setText(" ");
            ui->FiveRedScantronsLabel->setText(" ");
            ui->OnePencilLabel->setText(" ");
            ui->OneMPencilLabel->setText(" ");
            ui->OneFiveDollarEarbudsLabel->setText(" ");
            ui->OneThreeDollarEarbudsLabel->setText(" ");
            ui->OnePenLabel->setText(" ");
            ui->OneFolderLabel->setText(" ");

            ui->SharpieLabel->setText(" ");
            ui->EraserLabel->setText(" ");
            ui->HighlighterLabel->setText(" ");
            ui->DryEraseMarkerLabel->setText(" ");
            ui->PaddedEnvelopeLabel->setText(" ");

            ui->TapeFlagsLabel->setText(" ");
            ui->PencilRefillLabel->setText(" ");
            ui->PenInkLabel->setText(" ");
            ui->largeColoredIndexCardsLabel->setText(" ");
            ui->SmalleColoredIndexCardsLabel->setText(" ");
            ui->LargeRegularIndexCardsLabel->setText(" ");
            ui->SmallRegularIndexCardsLabel->setText(" ");

            ui->todaysTotalLabel->setText("$ 0.00");

            // Update inventory
            requestUpdate(UPDATE_INVENTORY);
        }
        // If the date changed, but nothing sold
        // still need to increment the date of reference
        dateOfReference = now;
    }
    // Reset timer
    updateTimer.start(TIMER_DAILY_RECEIPTS);
}


/*


                REPORTS


*/

void MainWindow::on_reportButton_clicked()
{
    access = 0;
    UserVerification * uv = new UserVerification(this);
    connect(uv, SIGNAL(accessGranted()), this, SLOT(user_granted_access()));
    uv->setAttribute( Qt::WA_DeleteOnClose, true );
    uv->show();
}


/*


                INVENTORY MANAGEMENT


*/

void MainWindow::on_inventoryManagementButton_clicked()
{
    access = 1;
    UserVerification * uv = new UserVerification(this);
    connect(uv, SIGNAL(accessGranted()), this, SLOT(user_granted_access()));
    uv->setAttribute( Qt::WA_DeleteOnClose, true );
    uv->show();
}


void MainWindow::user_granted_access()
{
    switch(access)
    {
    case 0:
    {
        ReportManager *rm = new ReportManager(&dataStructs, this);
        rm->setAttribute( Qt::WA_DeleteOnClose, true );
        rm->show();
        break;
    }
    case 1:
    {
        InventoryManager *im = new InventoryManager(&dataStructs, this);
        connect(im, SIGNAL(forceUpdate(int,QString)), this, SLOT(requestUpdate(int,QString)));
        im->setAttribute( Qt::WA_DeleteOnClose, true );
        im->show();
        break;
    }
    default:
        break;
    }

}

void MainWindow::closeEvent(QCloseEvent *)
{
    if( runningTotal > 0.00 )
    {
        // If the program is shut down before it has a chance to update, write to file.

        QDateTime now = QDateTime::currentDateTime();
        QString update = now.toString(STANDARD_DATE_FORMAT); update += LIST_DELIMITER;
        update += "DAILY RECEIPT"; update += LIST_DELIMITER;

        for(int i =0; i < NUMBER_OF_QUICKSALES; i++)
        {
            if( dailySales.quickSaleObjects[i].qtySold > 0 )
            {
                update += dailySales.quickSaleObjects[i].inventoryId;
                update += PACKAGE_DELIMITER;
                update += QString::number(dailySales.quickSaleObjects[i].qtySold);
                update += LIST_DELIMITER;
            }
        }
        update = update.left(update.length()-1);
        update += LIST_DELIMITER;
        update += QString::number(runningTotal, 'f', 2);

        QString dateTime = QDateTime::currentDateTime().toString(STANDARD_DATE_FORMAT_2);
        QString logFile = RECEIPT_FILE + dateTime + ".csv";

        QFile theFile(logFile);
        if( QFile::exists(logFile))
        {
            if( theFile.open(QFile::Append) )
            {
                QTextStream stream(&theFile);
                stream << "\n\n"
                       << update;
            }
        }
        else
        {
            if( theFile.open(QFile::WriteOnly) )
            {
                QTextStream stream(&theFile);
                stream << "\n"
                       << update;
            }
        }
    }
}

void MainWindow::on_actionBook_Charge_triggered()
{
    LibraryManager * newLibWindow = new LibraryManager(&dataStructs, this);
    newLibWindow->setAttribute( Qt::WA_DeleteOnClose, true );
    newLibWindow->show();

}

void MainWindow::playSoundFile(unsigned x)
{
    QString sound = BUTTON_SOUND_FOLDER;
    QSound * alert;
    switch(x)
    {
        case 1:
        {
            sound += BUTTON_CHOOSE;
            break;
        }
        case 2:
        {
            sound += BUTTON_UNDO;
            break;
        }
        default:
        {
            sound += BUTTON_CHOOSE;
            break;
        }
    }
    alert = new QSound(sound, this);
    alert->setLoops(1);
    alert->play();
    delete alert;
}
