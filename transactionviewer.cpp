#include "transactionviewer.h"
#include "ui_transactionviewer.h"

TransactionViewer::TransactionViewer(_structs * data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransactionViewer)
{
    ui->setupUi(this);

    dataStructures = data;

    // Setup dateSearch
    dateToLookup = QDate::currentDate();
    ui->dateSearch->setDate(dateToLookup);
    ui->dateTwoSearch->setDate(dateToLookup);
    ui->dateTwoSearch->setHidden(true);

    // Socket connection
    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));

    receiptCount = 0;
    // Initialize table with todays receipts
    retrieveDataFromServer();
 //   updateTable();
}

TransactionViewer::~TransactionViewer()
{
    delete ui;
}

void TransactionViewer::retrieveDataFromServer()
{
    if( receiptCount != 0 )
    {
        purgeStructures();
    }

    QString dateRange = "";
    if( ui->searchByRange->isChecked() )
    {
        dateRange = (ui->dateSearch->date().toString(STANDARD_DATE_FORMAT_2)
                    + RESULT_MEMBER_DELIMITER +
                    ui->dateTwoSearch->date().toString(STANDARD_DATE_FORMAT_2) );
    }
    else
    {
        dateRange = (ui->dateSearch->date().toString(STANDARD_DATE_FORMAT_2)
                    + RESULT_MEMBER_DELIMITER +
                    ui->dateSearch->date().toString(STANDARD_DATE_FORMAT_2) );
    }

    dateRange += QUERY_DELIMITER;
    dateRange += "3";

    socketConn.SubmitQuery(RECEIPT_QUERY, 0,0,dateRange);
}

void TransactionViewer::purgeStructures()
{
    for( receiptIndex = 0; receiptIndex < receiptCount; receiptIndex++ )
    {
        currentReceipts[receiptIndex].id = "";
        currentReceipts[receiptIndex].dateCreated = "";
        currentReceipts[receiptIndex].deptCode = "";
        currentReceipts[receiptIndex].description = "";
        currentReceipts[receiptIndex].firstName = "";
        currentReceipts[receiptIndex].lastName = "";
        currentReceipts[receiptIndex].itemsSold = "";
        currentReceipts[receiptIndex].seller = "";
        currentReceipts[receiptIndex].totalPrice = "";
        currentReceipts[receiptIndex].transactionCode = "";
        currentReceipts[receiptIndex].transactionType = "";
    }
    receiptIndex = 0;
    receiptCount = 0;
}

void TransactionViewer::processData(QString data)
{
    if( data != EMPTY_DB_ENTRY )
    {
        // Erase old struct info if any, and repopulate with data from db, then call updatetable
        receiptCount = 0;
        receiptIndex = 0;
        QStringList listOfReceipts = data.split(INCOMING_RESULT_DELIMITER);
        foreach(QString reciept, listOfReceipts)
        {
            QStringList cell = reciept.split(RESULT_MEMBER_DELIMITER);

            currentReceipts[receiptCount].id = cell[0];
            currentReceipts[receiptCount].dateCreated = cell[1];
            currentReceipts[receiptCount].seller = cell[2];
            currentReceipts[receiptCount].firstName = cell[3];
            currentReceipts[receiptCount].lastName = cell[4];
            currentReceipts[receiptCount].itemsSoldRaw = cell[5];

            // Go through each item sold
            QString itemDisplay;
            QStringList itemsSold = cell[5].split(LIST_DELIMITER);
            foreach(QString item, itemsSold)
            {
                // Find its name, as we retrieved ids
                for(int i = 0; i < dataStructures->counters.inventoryCount; i++ )
                {
                    if( dataStructures->inventory[i].inventoryId == item.split(PACKAGE_DELIMITER)[0] )
                    {
                        itemDisplay += dataStructures->inventory[i].itemName;
                        i = dataStructures->counters.inventoryCount;
                    }
                }
                // Once name retrieved, add quantity of item sold. Add LIST_DELIMITER to break apart in table
                itemDisplay += (" x " + item.split(PACKAGE_DELIMITER)[1] + CSV_INCELL_DELIMITER + " ");

            }
            // Trim trailing comma
            itemDisplay = itemDisplay.left(itemDisplay.length()-1);

            currentReceipts[receiptCount].itemsSold = itemDisplay;
            currentReceipts[receiptCount].totalPrice = cell[6];
            currentReceipts[receiptCount].description = cell[7];
            currentReceipts[receiptCount].deptCode = cell[8];
            currentReceipts[receiptCount].transactionType = cell[9];
            currentReceipts[receiptCount].transactionCode = cell[10];
            receiptCount++;
        }
    }
}

void TransactionViewer::updateTable()
{
    ui->receiptInformationLabel->setText("There are " + QString::number(receiptCount) + " receipts.");

    if ( receiptCount > 0 )
    {
        ui->receiptTable->clear();

        ui->receiptTable->setColumnCount(11);
        ui->receiptTable->setHorizontalHeaderLabels(
                    (QStringList() << "Id" << "Created" << "Seller" << "First Name" << "Last Name" << "Items Sold"
                                                     << "Total" << "Desc" << "Dept" << "Type" << "Code")
                    );
        ui->receiptTable->setColumnWidth(0, 50);
        ui->receiptTable->setColumnWidth(1, 110);
        ui->receiptTable->setColumnWidth(9, 100);
        ui->receiptTable->setColumnWidth(10, 50);

        QPalette* palette = new QPalette();
        palette->setColor(QPalette::Highlight,Qt::darkBlue);
        ui->receiptTable->setPalette(*palette);

        ui->receiptTable->setRowCount(receiptCount);

        for(unsigned i =0; i < receiptCount; i++ )
        {
            QTableWidgetItem *id = new QTableWidgetItem();
            id->setText( currentReceipts[i].id  );
            id->setFlags(id->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 0, id);

            QTableWidgetItem *date = new QTableWidgetItem();
            date->setText( currentReceipts[i].dateCreated  );
            date->setFlags(date->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 1, date);

            QTableWidgetItem *seller = new QTableWidgetItem();
            seller->setText( currentReceipts[i].seller  );
            seller->setFlags(seller->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 2, seller);

            QTableWidgetItem *firstName = new QTableWidgetItem();
            firstName->setText( currentReceipts[i].firstName  );
            firstName->setFlags(firstName->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 3, firstName);

            QTableWidgetItem *lastName = new QTableWidgetItem();
            lastName->setText( currentReceipts[i].lastName  );
            lastName->setFlags(lastName->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 4, lastName);

            QTableWidgetItem *items = new QTableWidgetItem();
            items->setText( currentReceipts[i].itemsSold  );
            items->setFlags(items->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 5, items);

            QTableWidgetItem *totalPrice = new QTableWidgetItem();
            totalPrice->setText( currentReceipts[i].totalPrice  );
            totalPrice->setFlags(totalPrice->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 6, totalPrice);

            QTableWidgetItem *desc = new QTableWidgetItem();
            desc->setText( currentReceipts[i].description  );
            desc->setFlags(desc->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 7, desc);

            QTableWidgetItem *dept = new QTableWidgetItem();
            dept->setText( currentReceipts[i].deptCode  );
            dept->setFlags(dept->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 8, dept);

            QTableWidgetItem *tt = new QTableWidgetItem();
            tt->setText( currentReceipts[i].transactionType  );
            tt->setFlags(tt->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 9, tt);

            QTableWidgetItem *tc = new QTableWidgetItem();
            tc->setText( currentReceipts[i].transactionCode  );
            tc->setFlags(tc->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 10, tc);
        }
    }
    else
    {
        ui->receiptTable->clear();

        ui->receiptTable->setColumnCount(11);
        ui->receiptTable->setHorizontalHeaderLabels(
                    (QStringList() << "Id" << "Created" << "Seller" << "First Name" << "Last Name" << "Items Sold"
                                                     << "Total" << "Desc" << "Dept" << "Type" << "Code")
                    );
        ui->receiptTable->setColumnWidth(0, 50);
        ui->receiptTable->setColumnWidth(1, 110);
        ui->receiptTable->setColumnWidth(9, 100);
        ui->receiptTable->setColumnWidth(10, 50);

        QPalette* palette = new QPalette();
        palette->setColor(QPalette::Highlight,Qt::darkBlue);
        ui->receiptTable->setPalette(*palette);

        ui->receiptTable->setRowCount(receiptCount);
    }
}

void TransactionViewer::on_searchButton_clicked()
{
    retrieveDataFromServer();
    updateTable();
}

void TransactionViewer::on_viewDetailsButton_clicked()
{
    if( ui->receiptTable->currentRow() != -1 && receiptCount > 0 )
    {
        ReceiptViewer * rec = new ReceiptViewer(currentReceipts[ui->receiptTable->currentRow()], this);
        rec->show();
    }

}

void TransactionViewer::on_receiptTable_clicked(const QModelIndex &index)
{
    ui->receiptTable->selectRow(index.row());
}

void TransactionViewer::on_closeButton_clicked()
{
    this->close();
}

void TransactionViewer::requestUpdateOnCurrentDate()
{
    retrieveDataFromServer();
    updateTable();
}

void TransactionViewer::on_editButton_clicked()
{
    if( ui->receiptTable->currentIndex().row() != -1 )
    {
        ReceiptEditor * editor = new ReceiptEditor(dataStructures, &currentReceipts[ui->receiptTable->currentIndex().row()], this);
        connect(editor, SIGNAL(forceUpdate()),this, SLOT(requestUpdateOnCurrentDate()));
        editor->show();
    }
}

void TransactionViewer::on_searchByRange_toggled(bool checked)
{
    (checked) ? ui->dateTwoSearch->setHidden(false) : ui->dateTwoSearch->setHidden(true);
}

void TransactionViewer::on_exportButton_clicked()
{
    QString dialogTitle = "Select where you would like to save the file";
    QString saveName = "IT-Receipt-Export_Made_";
    saveName += QDate::currentDate().toString(STANDARD_DATE_FORMAT_2) + ".csv";
    QString filename = QFileDialog::getSaveFileName(this, dialogTitle, saveName, "CSV files (*.csv)", 0, 0);
    QFile theFile(filename);
    if( theFile.open(QFile::WriteOnly |QFile::Truncate))
    {
        QTextStream stream(&theFile);

        stream << " Receipt Id " << ","
               << " Date Created " << ","
               << " Sold By " << ","
               << " Customer First Name " << ","
               << " Customer Last Name " << ","
               << " Items Purchased" << ","
               << " Description of sale" << ","
               << " Department" << ","
               << " Transaction Type" << ","
               << " Transaction Code" << ","
               << " Total Price" << "\n";

        for(unsigned i =0; i < receiptCount; i++ )
        {
            stream << currentReceipts[i].id << ","
                   << currentReceipts[i].dateCreated << ","
                   << currentReceipts[i].seller << ","
                   << currentReceipts[i].firstName << ","
                   << currentReceipts[i].lastName << ","
                   << currentReceipts[i].itemsSold << ","
                   << currentReceipts[i].description << ","
                   << currentReceipts[i].deptCode << ","
                   << currentReceipts[i].transactionType << ","
                   << currentReceipts[i].transactionCode << ","
                   << currentReceipts[i].totalPrice << "\n";
        }
    }
}
