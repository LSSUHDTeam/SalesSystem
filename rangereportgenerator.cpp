#include "rangereportgenerator.h"
#include "ui_rangereportgenerator.h"

RangeReportGenerator::RangeReportGenerator(_structs * data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RangeReportGenerator)
{
    ui->setupUi(this);

    dataStructs = data;

    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));

    ui->startDate->setDate(QDate::currentDate());
    ui->endDate->setDate(QDate::currentDate());
}

RangeReportGenerator::~RangeReportGenerator()
{
    delete ui;
}

void RangeReportGenerator::on_generateReport_clicked()
{
    ui->errLabel->clear();

    if( ui->startDate->date() != ui->endDate->date() )
    {
        if( ui->endDate->date() > ui->startDate->date() )
        {
            QString dateRange = ui->startDate->date().toString(STANDARD_DATE_FORMAT_2) + RESULT_MEMBER_DELIMITER + ui->endDate->date().toString(STANDARD_DATE_FORMAT_2);

            socketConn.SubmitQuery(RECEIPT_QUERY, 0,0,(dateRange + "^2"));
        }
        else
        {
            ui->errLabel->setText("Incorrect Range");
        }
    }
    else
    {
        ui->errLabel->setText("Dates can not be equal.");
    }
}

void RangeReportGenerator::processData(QString data)
{
    if( data != EMPTY_DB_ENTRY )
    {
        generateRangedReport(data);
    }
    else
    {
        ui->errLabel->setText("No records found.");
    }
}

void RangeReportGenerator::generateRangedReport(QString data)
{
    QLocale::setDefault( QLocale(QLocale::English, QLocale::UnitedStates) );

    // Retrieve the departments that contain chargebacks.

    QStringList departmentsWithChargeBacks;
    QStringList receipts = data.split(INCOMING_RESULT_DELIMITER);

    foreach(QString receipt, receipts)
    {
       QStringList receiptData = receipt.split(RESULT_MEMBER_DELIMITER);
       if( !departmentsWithChargeBacks.contains(receiptData[8]) && receiptData[9] == RECEIPT_CHARGE_BACK )
       {
           departmentsWithChargeBacks.append(receiptData[8]);
       }
    }

    // Create a file dialog for saving, and get filename selected.

    QString dialogTitle = "Select where you would like to save the file";
    QString saveName = ("Departmental_Chargeback_Report_"
            + QDate::currentDate().toString(STANDARD_DATE_FORMAT_2)
            + ".csv");

    QString filename = QFileDialog::getSaveFileName(this, dialogTitle, saveName, "CSV files (*.csv)", 0, 0);

    // Initialize and open the file.

    QFile theFile(filename);
    if( theFile.open(QFile::WriteOnly |QFile::Truncate))
    {
        // Create a stream, and write the contents
        QTextStream stream(&theFile);

        // Report date generated
        stream << (" Report for " +
                   ui->startDate->date().toString(STANDARD_DATE_FORMAT_2)
                   + " to " +
                   ui->startDate->date().toString(STANDARD_DATE_FORMAT_2)
                   ) << "\n\n";

        // Add headers
        stream << "\n" << " Department " << "," <<  " FOAPAL " << ","
               << " Receipt Id " << ","
               << " Date Created " << ","
               << " Sold By " << ","
               << " Customer First Name " << ","
               << " Customer Last Name " << ","
               << " Description " << ","
               << " Transaction Code " << ","
               << " Items Sold " << ","
               << " Receipt Total "
               << "\n";

        // For each department with chargebacks in given month
        foreach(QString department, departmentsWithChargeBacks)
        {
            // Keep track of total
            double departmentTotal = 0.00;

            // Indicate which department the information is about, and give fopal
            stream << "\n"
                   << department << "," << retieveFopalByDepartment(department) << "\n";

            // Write all receipts
            foreach(QString receipt, receipts)
            {
                QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);

                if( cells[8] == department )
                {
                    // Add up this department's total
                    departmentTotal += cells[6].toDouble();

                    stream << "    "    << "," << "    "   << ","
                           << cells[0]  << "," << cells[1] << "," << cells[2] << ","
                           << cells[3]  << "," << cells[4] << "," << cells[7] << ","
                           << cells[10] << ",";

                    // Write all items sold and their quantity
                    foreach(QString item, cells[5].split(LIST_DELIMITER))
                    {
                        QString dsp = retrieveNameById(item.split(PACKAGE_DELIMITER)[0]);
                        dsp += " Qty: ";
                        dsp += item.split(PACKAGE_DELIMITER)[1];
                        dsp += CSV_INCELL_DELIMITER;
                        dsp += " ";
                        stream << dsp;
                    }
                    stream << "," << QString("$%L1").arg( cells[6].toDouble(), 0, 'f', 2 ) << "\n";
                }
            }

            // Display department's total
            stream << "\n" << " " << "," << " " << "," << " " << "," << " " << "," << " " << ","
                   << " " << "," << " " << "," << " " << "," << " " << ","
                   << " " << "," << " Department Total " << "\n";
            stream << " " << "," << " " << "," << " " << "," << " " << "," << " " << ","
                   << " " << "," << " " << "," << " " << "," << " " << ","
                   << " " << "," << QString("$%L1").arg( departmentTotal, 0, 'f', 2 )<< "\n";
        }

        // Close file, and close the window.
        theFile.close();
        this->close();
    }
    else
    {
        ui->errLabel->setText("Unable to locate file.");
    }
}

QString RangeReportGenerator::retrieveNameById(QString id)
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

QString RangeReportGenerator::retieveFopalByDepartment(QString dept)
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
