#include "monthlyreportgenerator.h"
#include "ui_monthlyreportgenerator.h"

MonthlyReportGenerator::MonthlyReportGenerator(_structs * data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MonthlyReportGenerator)
{
    ui->setupUi(this);

    dataStructs = data;
    months.insert(1,"January");
    months.insert(2,"February");
    months.insert(3,"March");
    months.insert(4,"April");
    months.insert(5,"May");
    months.insert(6,"June");
    months.insert(7,"July");
    months.insert(8,"August");
    months.insert(9,"September");
    months.insert(10,"October");
    months.insert(11,"November");
    months.insert(12,"December");

    // Build list of years
    now = QDate::currentDate();
    int year = now.year();
    QStringList years;
    for(int i =0; i < 5; i++)
    {
        years.append(QString::number(year-i));
    }
    // Add to combo
    ui->yearSelector->addItems(years);

    // Add months to combo
    for(int i = 1; i <= 12; i++)
    {
        ui->monthSelector->addItem(months[i]);
    }

    // Connect the socket data to slot
    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));
}

MonthlyReportGenerator::~MonthlyReportGenerator()
{
    delete ui;
}

void MonthlyReportGenerator::on_pushButton_clicked()
{
    QString request = (
                QString::number(ui->monthSelector->currentIndex()+1)
                + RESULT_MEMBER_DELIMITER +
                ui->yearSelector->currentText());

    socketConn.SubmitQuery(RECEIPT_QUERY, 2,0, request);
}

void MonthlyReportGenerator::processData(QString data)
{
    if( data != EMPTY_DB_ENTRY )
    {
        generateMonthlyChargeBack(data);
    }
    else
    {
        ui->userInfoLabel->setText("No records found.");
    }
}

void MonthlyReportGenerator::generateMonthlyChargeBack(QString data)
{
    // Get eacha fopal with a chargeback
    QStringList departmentsWithChargeBacks;
    QStringList departmentsWithCopyChargeBacks;
    QStringList departmentsWithILLChargeBacks;
    QStringList departmentsWithLibChargeBacks;
    QStringList receipts = data.split(INCOMING_RESULT_DELIMITER);

    foreach(QString receipt, receipts)
    {
       QStringList receiptData = receipt.split(RESULT_MEMBER_DELIMITER);

       // I.T ChargeBacks
       if( !departmentsWithChargeBacks.contains(receiptData[8]) && receiptData[10] == RECEIPT_IT_CODE)
       {
            departmentsWithChargeBacks.append(receiptData[8]);
       }

       // COPY Chargebacks
       if( !departmentsWithCopyChargeBacks.contains(receiptData[8]) && receiptData[10] == RECEIPT_COPY_CODE)
       {
            departmentsWithCopyChargeBacks.append(receiptData[8]);
       }

       // ILL Chargebacks
       if( !departmentsWithILLChargeBacks.contains(receiptData[8]) && receiptData[10] == RECEIPT_ILL_CODE)
       {
            departmentsWithILLChargeBacks.append(receiptData[8]);
       }

       // Lib Chargebacks.
       if( !departmentsWithLibChargeBacks.contains(receiptData[8]) && receiptData[10] == RECEIPT_LIB_CODE)
       {
            departmentsWithLibChargeBacks.append(receiptData[8]);
       }
    }

    // Create a file dialog for saving, and get filename selected.
    QString dialogTitle = "Select where you would like to save the file";
    QString saveName = "Business_Office_Monthly_Report"
            + ui->yearSelector->currentText()
            + "_" + QString::number(ui->monthSelector->currentIndex()+1)
            + "_Made_"
            + QDate::currentDate().toString(STANDARD_DATE_FORMAT_2)
            + ".csv";

    QString filename = QFileDialog::getSaveFileName(this, dialogTitle, saveName, "CSV files (*.csv)", 0, 0);

    // Initialize and open the file.
    QFile theFile(filename);
    if( theFile.open(QFile::WriteOnly |QFile::Truncate))
    {
        // Create a stream, and write the contents
        QTextStream stream(&theFile);

        stream << ( " IT Departmental Chargebacks for " + months[ui->monthSelector->currentIndex()+1] + " " + ui->yearSelector->currentText() + "\n\n");

        stream << " FOPAL " << ","
               << " DEPARTMENT " << ","
               << " DATE " << ","
               << " NAME " << ","
               << " DESCRIPTION " << ","
               << " QTY " << ","
               << " PRICE \n";

        // I could have made functions out of the next little bit, but there was a bit of a time crunch
        double runningTotal = 0.00;
        if( departmentsWithChargeBacks.length() > 0 )
        {
            /*
                    WRITE OUT IT CHARGEBACKS
            */
            foreach(QString department, departmentsWithChargeBacks)
            {
                // Write all receipts
                foreach(QString receipt, receipts)
                {
                    QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                    if( cells[8] == department && cells[10] == RECEIPT_IT_CODE)
                    {
                        // Grab total number of items sold
                        int totalItemCount = 0;
                        QStringList totalItemCounts;
                        QStringList itemBlocks = cells[5].split(LIST_DELIMITER);
                        foreach(QString block, itemBlocks)
                        {
                            totalItemCounts.append(block.split(PACKAGE_DELIMITER)[1]);
                        }
                        foreach(QString item, totalItemCounts)
                        {
                            totalItemCount += item.toInt();
                        }
                        // Add to department's total
                        runningTotal += cells[6].toDouble();
                        stream << retieveFopalByDepartment(department, RECEIPT_IT_CODE) << "," << department << "," << cells[1] << ","
                               << (cells[3] + " " + cells[4]) << "," << cells[7] << "," << QString::number(totalItemCount, 'f', 2) << ","
                               << QString("$%L1").arg( cells[6].toDouble(), 0, 'f', 2 ) << "\n";
                    }
                }
            }
            stream << "\n\n , , , , TOTAL : " << ", ," << QString("$%L1").arg( runningTotal, 0, 'f', 2 );

        }

        if( departmentsWithCopyChargeBacks.length() > 0 )
        {
            /*
                    WRITE OUT COPY CHARGEBACKS
            */
            stream << ("\n\n Library Copier Chargebacks for " + months[ui->monthSelector->currentIndex()+1] + " " + ui->yearSelector->currentText() + "\n\n");

            runningTotal = 0.00;
            foreach(QString department, departmentsWithCopyChargeBacks)
            {
                // Write all receipts
                foreach(QString receipt, receipts)
                {
                    QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                    if( cells[8] == department && cells[10] == RECEIPT_COPY_CODE)
                    {
                        // Grab total number of items sold
                        int totalItemCount = 0;
                        QStringList totalItemCounts;
                        QStringList itemBlocks = cells[5].split(LIST_DELIMITER);
                        foreach(QString block, itemBlocks)
                        {
                            totalItemCounts.append(block.split(PACKAGE_DELIMITER)[1]);
                        }
                        foreach(QString item, totalItemCounts)
                        {
                            totalItemCount += item.toInt();
                        }
                        // Add to department's total
                        runningTotal += cells[6].toDouble();
                        stream << retieveFopalByDepartment(department, RECEIPT_COPY_CODE) << "," << department << "," << cells[1] << ","
                               << (cells[3] + " " + cells[4]) << "," << cells[7] << "," << QString::number(totalItemCount, 'f', 2) << ","
                               << QString("$%L1").arg( cells[6].toDouble(), 0, 'f', 2 ) << "\n";
                    }
                }
            }
            stream << "\n\n , , , , TOTAL : " << ", ," << QString("$%L1").arg( runningTotal, 0, 'f', 2 );
        }

        if( departmentsWithILLChargeBacks.length() > 0 )
        {
            /*
                    WRITE OUT ILL CHARGEBACKS
            */
            stream << ("\n\n ILL Chargebacks for " + months[ui->monthSelector->currentIndex()+1] + " " + ui->yearSelector->currentText() + "\n\n");

            runningTotal = 0.00;
            foreach(QString department, departmentsWithILLChargeBacks)
            {
                // Write all receipts
                foreach(QString receipt, receipts)
                {
                    QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                    if( cells[8] == department && cells[10] == RECEIPT_ILL_CODE)
                    {
                        // Grab total number of items sold
                        int totalItemCount = 0;
                        QStringList totalItemCounts;
                        QStringList itemBlocks = cells[5].split(LIST_DELIMITER);
                        foreach(QString block, itemBlocks)
                        {
                            totalItemCounts.append(block.split(PACKAGE_DELIMITER)[1]);
                        }
                        foreach(QString item, totalItemCounts)
                        {
                            totalItemCount += item.toInt();
                        }
                        // Add to department's total
                        runningTotal += cells[6].toDouble();
                        stream << retieveFopalByDepartment(department, RECEIPT_ILL_CODE) << "," << department << "," << cells[1] << ","
                               << (cells[3] + " " + cells[4]) << "," << cells[7] << "," << QString::number(totalItemCount, 'f', 2) << ","
                               << QString("$%L1").arg( cells[6].toDouble(), 0, 'f', 2 ) << "\n";
                    }
                }
            }
            stream << "\n\n , , , , TOTAL : " << ", ," << QString("$%L1").arg( runningTotal, 0, 'f', 2 );
        }

        if( departmentsWithLibChargeBacks.length() > 0 )
        {
            /*
                    WRITE OUT Library CHARGEBACKS
            */
            stream << ("\n\n Library Chargebacks for " + months[ui->monthSelector->currentIndex()+1] + " " + ui->yearSelector->currentText() + "\n\n");

            runningTotal = 0.00;
            foreach(QString department, departmentsWithLibChargeBacks)
            {
                // Write all receipts
                foreach(QString receipt, receipts)
                {
                    QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                    if( cells[8] == department && cells[10] == RECEIPT_LIB_CODE)
                    {
                        // Grab total number of items sold
                        int totalItemCount = 0;
                        QStringList totalItemCounts;
                        QStringList itemBlocks = cells[5].split(LIST_DELIMITER);
                        foreach(QString block, itemBlocks)
                        {
                            totalItemCounts.append(block.split(PACKAGE_DELIMITER)[1]);
                        }
                        foreach(QString item, totalItemCounts)
                        {
                            totalItemCount += item.toInt();
                        }
                        // Add to department's total
                        runningTotal += cells[6].toDouble();
                        stream << retieveFopalByDepartment(department, RECEIPT_LIB_CODE) << "," << department << "," << cells[1] << ","
                               << (cells[3] + " " + cells[4]) << "," << cells[7] << "," << QString::number(totalItemCount, 'f', 2) << ","
                               << QString("$%L1").arg( cells[6].toDouble(), 0, 'f', 2 ) << "\n";
                    }
                }
            }
            stream << "\n\n , , , , TOTAL : " << ", ," << QString("$%L1").arg( runningTotal, 0, 'f', 2 );
        }

        ui->userInfoLabel->setText("File Generated.");
    }
    else
    {
        ui->userInfoLabel->setText("Unable to locate file.");
    }
}

QString MonthlyReportGenerator::retrieveNameById(QString id)
{
    for( int i = 0; i < dataStructs->counters.inventoryCount; i++ )
    {
        if( dataStructs->inventory[i].inventoryId == id )
        {
            return dataStructs->inventory[i].itemName;
        }
    }
    return "Unknown Item";
}

QString MonthlyReportGenerator::retieveFopalByDepartment(QString dept, QString chargeAcc)
{
    for( int i = 0; i < dataStructs->counters.departmentCount; i++ )
    {
        if( dataStructs->departments[i].name == dept)
        {
            QString foapal = dataStructs->departments[i].deptFopal.fund;
            foapal += "-";
            foapal += dataStructs->departments[i].deptFopal.organization;
            foapal += "-";

            if( chargeAcc == RECEIPT_IT_CODE )
            {
                foapal += ACCOUNT_IT;
            }
            else if( chargeAcc == RECEIPT_ILL_CODE )
            {
                foapal += ACCOUNT_ILL;
            }
            else if( chargeAcc == RECEIPT_LIB_CODE )
            {
                foapal += ACCOUNT_LIB;
            }
            else if( chargeAcc == RECEIPT_COPY_CODE )
            {
                foapal += ACCOUNT_COPY;
            }

            foapal += "-";
            foapal += dataStructs->departments[i].deptFopal.program;
            if( dataStructs->departments[i].extra != EMPTY_DB_ENTRY )
                foapal += ("-" + dataStructs->departments[i].extra);
            return foapal;
        }
    }
    return "Unknown Department";
}
