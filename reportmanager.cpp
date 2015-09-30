#include "reportmanager.h"
#include "ui_reportmanager.h"

ReportManager::ReportManager(_structs * data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportManager)
{
    processSwitch = 0;
    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));

    ui->setupUi(this);

    for(int i = 1; i <= 12; i++ )
    {
        ui->monthSelector->addItem(QString::number(i));
    }
    // Build list of years
    QDate now = QDate::currentDate();
    int year = now.year();
    QStringList years;
    for(int i =0; i < 5; i++)
    {
        years.append(QString::number(year-i));
    }

    // Add to combo
    ui->yearSelector->addItems(years);

    dataStructs = data;
    ui->startDate->setDate(QDate::currentDate().addDays(-7));
    ui->endDate->setDate(QDate::currentDate());

}

ReportManager::~ReportManager()
{
    delete ui;
}

void ReportManager::on_closeButton_clicked()
{
    this->close();
}

void ReportManager::on_monthlySalesButton_clicked()
{
    MonthlyReportGenerator * mrg = new MonthlyReportGenerator(dataStructs, this);
    mrg->show();
}

void ReportManager::on_ReportManager_rejected()
{

}

void ReportManager::on_dateRangeReport_clicked()
{
    RangeReportGenerator * rr = new RangeReportGenerator(dataStructs, this);
    rr->show();
}

void ReportManager::on_closeDrawerButton_clicked()
{
    DrawerCloser * dc = new DrawerCloser(dataStructs, this);
    dc->show();
}

void ReportManager::on_closingsEditor_clicked()
{
    ClosingsEditor * ce = new ClosingsEditor(dataStructs, this);
    ce->show();
}

void ReportManager::on_dateRangeSelector_toggled(bool checked)
{
    ui->errLabel->clear();
    ui->endDate->setEnabled(checked);
    ui->startDate->setEnabled(checked);
    ui->dateRangeSelector->setChecked(checked);
    ui->viewSelector->setChecked(false);
    ui->idRangeSelector->setChecked(false);
    ui->generateCsvSelector->setChecked(false);
}

void ReportManager::on_idRangeSelector_toggled(bool checked)
{
    ui->rangeOne->clear();
    ui->rangeTwo->clear();
    ui->errLabel->clear();
    ui->rangeOne->setEnabled(checked);
    ui->rangeTwo->setEnabled(checked);
    ui->idRangeSelector->setChecked(checked);
    ui->viewSelector->setChecked(false);
    ui->dateRangeSelector->setChecked(false);
    ui->generateCsvSelector->setChecked(false);
}

void ReportManager::processData(QString data)
{
    if( data != EMPTY_DB_ENTRY )
    {
        // View/Edit Report
        if( processSwitch == 1)
        {
            QString report = generateReport(data.split(INCOMING_RESULT_DELIMITER));

            // If view is selected, send to viewer
            if( ui->viewSelector->isChecked() )
            {
                ReportViewer *rv = new ReportViewer(report, this);
                rv->show();
            }

            // If gen is selected, send to generator
            if( ui->generateCsvSelector->isChecked() )
            {
                QString dialogTitle = "Select where you would like to save the file";
                QString saveName = ("Detailed_Sales_Report_Made_"
                        + QDate::currentDate().toString(STANDARD_DATE_FORMAT_2)
                        + ".csv");

                QString filename = QFileDialog::getSaveFileName(this, dialogTitle, saveName, "CSV files (*.csv)", 0, 0);

                // Initialize and open the file.

                QFile theFile(filename);
                if( theFile.open(QFile::WriteOnly |QFile::Truncate))
                {
                    QTextStream stream(&theFile);
                    if( ui->idRangeSelector->isChecked() )
                        stream << "Receipt range " + ui->rangeOne->text() + " to " + ui->rangeTwo->text();

                    if( ui->dateRangeSelector->isChecked() )
                        stream << "Date range " + ui->startDate->date().toString(STANDARD_DATE_FORMAT_2) + " to " + ui->endDate->date().toString(STANDARD_DATE_FORMAT_2);

                    stream << "\n";
                    stream << report;
                    theFile.close();
                }
                else
                {
                    ui->errLabel->setText("Could not find the file.");
                }
            }
        }

        // Monthly Depertmental Summary Reports
        if( processSwitch == 2 )
        {
            generateSummaryReports(data);
        }
    }
    else
    {
        ui->errLabel->setText("Database returned empty values");
    }
}

void ReportManager::on_generateRport_clicked()
{
    if( ui->viewSelector->isChecked() || ui->generateCsvSelector->isChecked())
    {

        if( ui->dateRangeSelector->isChecked() )
        {
            processSwitch = 1;
            QString dateRange = ui->startDate->date().toString(STANDARD_DATE_FORMAT_2) + RESULT_MEMBER_DELIMITER + ui->endDate->date().toString(STANDARD_DATE_FORMAT_2);
            socketConn.SubmitQuery(RECEIPT_QUERY, 0,0,(dateRange + "^1"));
        }
        else if ( ui->idRangeSelector->isChecked() )
        {
            if( ui->rangeOne->text().toInt() )
            {
                if( ui->rangeTwo->text().toInt() )
                {
                    processSwitch = 1;
                    QString idRange = ui->rangeOne->text() + RESULT_MEMBER_DELIMITER + ui->rangeTwo->text();
                    socketConn.SubmitQuery(RECEIPT_QUERY, 6, 0, idRange);
                }
                else
                {
                    ui->errLabel->setText("ID is non-integer");
                }
            }
            else
            {
                ui->errLabel->setText("ID is non-integer");
            }
        }
        else
        {
            ui->errLabel->setText("No selector specified");
        }
    }
    else
    {
        ui->errLabel->setText("View or Generate must be selected.");
    }
}

QString ReportManager::generateReport(QStringList data)
{
    QString report = "";
    QStringList itCashSales, illCashSales, libCashSales, photoCashSales, libFines;
    double itTotal = 0.00, illTotal = 0.00, libTotal = 0.00, photoTotal = 0.00;

    foreach(QString receipt, data)
    {
        QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);

        if(!itCashSales.contains(cells[2])  && cells[2]  == RECEIPT_DAILY_SALE)
        {
            itCashSales.append(cells[0]);
            itTotal += cells[6].toDouble();
        }

        if(( cells[9] == RECEIPT_CASH ) || (cells[9] == RECEIPT_CHECK))
        {
            if(!itCashSales.contains(cells[10]) && cells[10] == RECEIPT_IT_CODE)
            {
                itCashSales.append(cells[0]);
                itTotal += cells[6].toDouble();
            }

            if( !illCashSales.contains(cells[10]) && cells[10] == RECEIPT_ILL_CODE)
            {
                illCashSales.append(cells[0]);
                illTotal += cells[6].toDouble();
            }

            // Seperate fines form sales for the library . . . . *sigh
            if ( cells[10] == RECEIPT_LIB_CODE )
            {
                if( cells[5].split(PACKAGE_DELIMITER)[0] == "104" )
                {
                    if(!libFines.contains(cells[10]))
                    {
                        libFines.append(cells[0]);
                    }
                }
                else
                {
                    if(!libCashSales.contains(cells[10]))
                    {
                        libCashSales.append(cells[0]);
                    }
                }
                libTotal += cells[6].toDouble();
            }

            if( !photoCashSales.contains(cells[10]) && cells[10] == RECEIPT_COPY_CODE)
            {
                photoCashSales.append(cells[0]);
                photoTotal += cells[6].toDouble();
            }
        }
    }

    if( itCashSales.length() > 0)
    {
        report += "\n\nIT SALES,\n ";
        foreach(QString id, itCashSales)
        {
            foreach(QString receipt, data)
            {
                QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                if( cells[0] == id )
                {
                    report += cells[0];
                    report += LIST_DELIMITER;
                    report += cells[1];
                    report += LIST_DELIMITER;
                    report += cells[3] + " " + cells[4];
                    report += LIST_DELIMITER;
                    report += cells[2];
                    report += LIST_DELIMITER;
                    foreach(QString item, cells[5].split(LIST_DELIMITER))
                    {
                        QString dsp = retrieveNameById(item.split(PACKAGE_DELIMITER)[0]);
                        dsp += " Qty: ";
                        dsp += item.split(PACKAGE_DELIMITER)[1];
                        dsp += CSV_INCELL_DELIMITER;
                        dsp += " ";
                        report += dsp;
                    }
                    report += LIST_DELIMITER;
                    report += cells[6];
                    report += "\n";
                }
            }
        }
    }

    if( illCashSales.length() > 0)
    {
        report += "\n\nILL SALES,\n ";
        foreach(QString id, illCashSales)
        {
            foreach(QString receipt, data)
            {
                QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                if( cells[0] == id )
                {
                    report += cells[0];
                    report += LIST_DELIMITER;
                    report += cells[1];
                    report += LIST_DELIMITER;
                    report += cells[3] + " " + cells[4];
                    report += LIST_DELIMITER;
                    report += cells[2];
                    report += LIST_DELIMITER;
                    foreach(QString item, cells[5].split(LIST_DELIMITER))
                    {
                        QString dsp = retrieveNameById(item.split(PACKAGE_DELIMITER)[0]);
                        dsp += " Qty: ";
                        dsp += item.split(PACKAGE_DELIMITER)[1];
                        dsp += CSV_INCELL_DELIMITER;
                        dsp += " ";
                        report += dsp;
                    }
                    report += LIST_DELIMITER;
                    report += cells[6];
                    report += "\n";
                }
            }
        }
    }

    if( libFines.length() > 0)
    {
        report += "\n\nLibrary FINES,\n ";
        foreach(QString id, libFines)
        {
            foreach(QString receipt, data)
            {
                QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                if( cells[0] == id )
                {
                    report += cells[0];
                    report += LIST_DELIMITER;
                    report += cells[1];
                    report += LIST_DELIMITER;
                    report += cells[3] + " " + cells[4];
                    report += LIST_DELIMITER;
                    report += cells[2];
                    report += LIST_DELIMITER;
                    foreach(QString item, cells[5].split(LIST_DELIMITER))
                    {
                        QString dsp = retrieveNameById(item.split(PACKAGE_DELIMITER)[0]);
                        dsp += " Qty: ";
                        dsp += item.split(PACKAGE_DELIMITER)[1];
                        dsp += CSV_INCELL_DELIMITER;
                        dsp += " ";
                        report += dsp;
                    }
                    report += LIST_DELIMITER;
                    report += cells[6];
                    report += "\n";
                }
            }
        }
    }

    if( libCashSales.length() > 0)
    {
        report += "\n\nLibrary SALES,\n ";
        foreach(QString id, libCashSales)
        {
            foreach(QString receipt, data)
            {
                QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                if( cells[0] == id )
                {
                    report += cells[0];
                    report += LIST_DELIMITER;
                    report += cells[1];
                    report += LIST_DELIMITER;
                    report += cells[3] + " " + cells[4];
                    report += LIST_DELIMITER;
                    report += cells[2];
                    report += LIST_DELIMITER;
                    foreach(QString item, cells[5].split(LIST_DELIMITER))
                    {
                        QString dsp = retrieveNameById(item.split(PACKAGE_DELIMITER)[0]);
                        dsp += " Qty: ";
                        dsp += item.split(PACKAGE_DELIMITER)[1];
                        dsp += CSV_INCELL_DELIMITER;
                        dsp += " ";
                        report += dsp;
                    }
                    report += LIST_DELIMITER;
                    report += cells[6];
                    report += "\n";
                }
            }
        }
    }

    if( photoCashSales.length() > 0)
    {
        report += "\n\nCOPY SALES,\n ";
        foreach(QString id, photoCashSales)
        {
            foreach(QString receipt, data)
            {
                QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                if( cells[0] == id )
                {
                    report += cells[0];
                    report += LIST_DELIMITER;
                    report += cells[1];
                    report += LIST_DELIMITER;
                    report += cells[3] + " " + cells[4];
                    report += LIST_DELIMITER;
                    report += cells[2];
                    report += LIST_DELIMITER;
                    foreach(QString item, cells[5].split(LIST_DELIMITER))
                    {
                        QString dsp = retrieveNameById(item.split(PACKAGE_DELIMITER)[0]);
                        dsp += " Qty: ";
                        dsp += item.split(PACKAGE_DELIMITER)[1];
                        dsp += CSV_INCELL_DELIMITER;
                        dsp += " ";
                        report += dsp;
                    }
                    report += LIST_DELIMITER;
                    report += cells[6];
                    report += "\n";
                }
            }
        }
    }

    if( itCashSales.length() > 0)
    {
        double amountTax = dataStructs->saleInformation.salesTax * itTotal;
        report += "\n\n,IT Taxable Income : ," + QString::number(itTotal-amountTax, 'f', 2) + "\n";
        report += ",Tax on IT Income : ," + QString::number(amountTax, 'f', 2) + "\n";
    }
    else
    {
        report += "\n\n";
    }

    if( libCashSales.length() > 0 || libFines.length() > 0 )
        report += ",Library Income :," + QString::number(libTotal, 'f', 2) + "\n";

    if( illCashSales.length() > 0)
        report += ",ILL Income :," + QString::number(illTotal, 'f', 2) + "\n";

    if( photoCashSales.length() > 0)
        report += ",Copy Income:," + QString::number(photoTotal, 'f', 2) + "\n";

    report += ",Total Income:," + QString::number( (itTotal + libTotal + illTotal + photoTotal) , 'f', 2) + "\n";

    return report;
}

void ReportManager::on_generateDepartmentalSummary_clicked()
{
    processSwitch = 2;
    QString monthYear = ui->monthSelector->currentText() + RESULT_MEMBER_DELIMITER + ui->yearSelector->currentText();
    socketConn.SubmitQuery(RECEIPT_QUERY,2,0,monthYear);
}

void ReportManager::generateSummaryReports(QString data)
{
    ui->errLabel->clear();

    // Find where to save
    QString dir = QFileDialog::getExistingDirectory(this, tr("Select Directory"),
                                                 "C:\\",
                                                 QFileDialog::ShowDirsOnly
                                                 | QFileDialog::DontResolveSymlinks);
    dir.replace("/", "\\\\");

    QString test = dir;
    if( test.replace("\\", "") == "C:")
    {
        ui->errLabel->setText("Can not save directly to C:\\");
    }
    else
    {

        dir = checkIfFolderExistsAndName(
                    ("Summary_Reports_" + QDate::currentDate().toString(STANDARD_DATE_FORMAT_2).replace(" ", "_"))
                    , dir);

        QStringList departmentsWithChargeBacks;
        QStringList receipts = data.split(INCOMING_RESULT_DELIMITER);

        foreach(QString receipt, receipts)
        {
            QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);

            if( cells[9] == RECEIPT_CHARGE_BACK && !departmentsWithChargeBacks.contains(cells[8]) )
            {
                if(cells[8] != "Student")
                {
                    QString renamed = cells[8];
                    deptNames.insert(cells[8], checkIfFileExistsAndName(renamed.replace(" ", "_"),dir));
                    departmentsWithChargeBacks.append(cells[8]);
                }
            }
        }


        foreach(QString department, departmentsWithChargeBacks)
        {
            double departmentTotal = 0.00;

            QString report = "";
            report += ("Report for " + department + ". " + QDate::currentDate().toString(STANDARD_DATE_FORMAT_2));
            report += ("\n" + retieveFopalByDepartment(department) + "\n");

            foreach(QString receipt, receipts)
            {
                QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);
                if( cells[8] == department )
                {
                    departmentTotal += cells[6].toDouble();

                    report += (",,"
                               + cells[0]  + "," + cells[1] + "," + cells[2] + ","
                               + cells[3]  + "," + cells[4] + "," + cells[7] + ","
                               + cells[10] + ",");

                    foreach(QString item, cells[5].split(LIST_DELIMITER))
                    {
                        QString dsp = retrieveNameById(item.split(PACKAGE_DELIMITER)[0]);
                        dsp += " Qty: ";
                        dsp += item.split(PACKAGE_DELIMITER)[1];
                        dsp += CSV_INCELL_DELIMITER;
                        dsp += " ";
                        report += dsp;
                    }
                    report += ("," + QString("$%L1").arg( cells[6].toDouble(), 0, 'f', 2 ) + "\n");
                }
            }

           report += ("\n,,,,,,,,,,Department Total\n");
           report += ("\n,,,,,,,,,," + QString("$%L1").arg( departmentTotal, 0, 'f', 2 ) + "\n");


           // Write the report to file
           writeOutReport(report, deptNames[department]);
        }
    }
}

void ReportManager::writeOutReport(QString report, QString filename)
{
    QFile theFile(filename);
    if( theFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QTextStream stream(&theFile);
        stream << report;
        theFile.close();
    }
    else
    {
        ui->errLabel->setText("Unable to locate file for save.");
    }
}

QString ReportManager::checkIfFileExistsAndName(QString currName, QString dir)
{
    int count = 0;
    bool done = false;

    QString fileName = dir + "\\\\" + currName + ".csv";

    while(!done)
    {
        QFile Fout(fileName);
        if( Fout.exists() )
        {
            count++;
            fileName = dir + "\\\\" + currName + "_" + QString::number(count) + ".csv";
        }
        else
        {
            Fout.close();
            done = true;
        }
    }
    return fileName;
}

QString ReportManager::checkIfFolderExistsAndName(QString folder, QString dir)
{
    int count = 0;
    bool done = false;

    QString fileName = dir + "\\\\" + folder;

    while(!done)
    {
        if( QDir(fileName).exists() )
        {
            count++;
            fileName = dir + "\\\\" + folder + "_" + QString::number(count);
        }
        else
        {
            done = true;
        }
    }

    QDir().mkdir(fileName);
    return fileName;
}

QString ReportManager::retrieveNameById(QString id)
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

QString ReportManager::retieveFopalByDepartment(QString dept)
{
    for( int i = 0; i < dataStructs->counters.departmentCount; i++ )
    {
        if( dataStructs->departments[i].name == dept)
        {
            QString foapal = dataStructs->departments[i].deptFopal.fund;
            foapal += "-";
            foapal += dataStructs->departments[i].deptFopal.organization;
            foapal += "-";
            foapal += dataStructs->departments[i].deptFopal.account;
            foapal += "-";
            foapal += dataStructs->departments[i].deptFopal.program;

            if( dataStructs->departments[i].extra != EMPTY_DB_ENTRY )
                foapal += ("-" + dataStructs->departments[i].extra);

            return foapal;
        }
    }
    return "Unknown Department";
}
