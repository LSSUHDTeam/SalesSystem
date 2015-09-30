#include "bookreceipts.h"
#include "ui_bookreceipts.h"

BookReceipts::BookReceipts(_structs * dataStructs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::BookReceipts)
{
    messageSwitch = 0;
    ui->setupUi(this);

    ui->transactionCode->addItems(dataStructs->saleInformation.codes);
    ui->transactionType->addItems(dataStructs->saleInformation.types);

    ui->department->addItem("Student");
    for(int i = 0; i < dataStructs->counters.departmentCount; i++ )
    {
        ui->department->addItem(dataStructs->departments[i].name);
    }

    ui->dateTimeLabel->setText(QDateTime::currentDateTime().toString(STANDARD_DATE_FORMAT));

}

BookReceipts::~BookReceipts()
{
    delete ui;
}

void BookReceipts::on_submitButton_clicked()
{
    if( validDataEntry() )
    {
        update = ui->dateTimeLabel->text();
        update += RESULT_MEMBER_DELIMITER;
        update += subroutine_filterText(ui->seller->text());
        update += RESULT_MEMBER_DELIMITER;
        update += subroutine_filterText(ui->firstName->text());
        update += RESULT_MEMBER_DELIMITER;
        update += subroutine_filterText(ui->lastName->text());
        update += RESULT_MEMBER_DELIMITER;

        (this->ui->fineCheckBox->isChecked()) ? update += "104#" : update += "94#";

        update += ui->quantity->text();
        update += RESULT_MEMBER_DELIMITER;
        update += ui->total->text();
        update += RESULT_MEMBER_DELIMITER;

        update += subroutine_filterText(ui->description->toPlainText());
        update += RESULT_MEMBER_DELIMITER;

        update += ui->department->currentText();
        update += RESULT_MEMBER_DELIMITER;
        update += ui->transactionType->currentText();
        update += RESULT_MEMBER_DELIMITER;
        update += ui->transactionCode->currentText();

        socketClient socketConn;

        connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));

        socketConn.SubmitQuery(RECEIPT_QUERY,1,0,update);
        this->close();

    }
}

void BookReceipts::processData(QString x)
{
    if( ui->printReceiptCheckBox->isChecked() )
    {
        QStringList cells = update.split(RESULT_MEMBER_DELIMITER);
        cells[4] = "Library Manual Receipt";
        cells.append(x);

        SalesPrinterEngine printerEngine(PRINTER_RECEIPT_STANDARD, this);
        printerEngine.printData(cells);
    }
}

void BookReceipts::on_cancelButton_clicked()
{
    this->close();
}

bool BookReceipts::validDataEntry()
{
    QStringList m;
    if( ui->seller->text().length() >= 3 )
    {
        if( subroutine_filterText(ui->firstName->text()).length() >= 3
                || subroutine_filterText(ui->lastName->text()).length() >= 3)
        {
            if( validQuantity() )
            {
                if( validPrice() )
                {
                    if( validTotal() )
                    {
                        if( subroutine_filterText(ui->description->toPlainText()).length() >= 10 )
                        {
                            return true;
                        }
                        else
                        {
                            m << ERR_INCOMPLETE_FORM << " Invalid description " << "\n\t You must enter a description > 10 charachters." << MESSAGE_OKAY_ONLY;
                            generateMessageBox(m);
                        }
                    }
                    else
                    {
                        m << ERR_INCOMPLETE_FORM << " Invalid total price " << "\n\t You must enter a decimal price with two points precision." << MESSAGE_OKAY_ONLY;
                        generateMessageBox(m);
                    }
                }
                else
                {
                    m << ERR_INCOMPLETE_FORM << " Invalid price information " << "\n\t You must enter a decimal price with two points precision." << MESSAGE_OKAY_ONLY;
                    generateMessageBox(m);
                }
            }
            else
            {
                m << ERR_INCOMPLETE_FORM << " Invalid Quantity " << "\n\t You must enter an integer for quantity." << MESSAGE_OKAY_ONLY;
                generateMessageBox(m);
            }
        }
        else
        {
            m << ERR_INCOMPLETE_FORM << " Invalid Buyer information " << "\n\t You must enter buyer first and last name." << MESSAGE_OKAY_ONLY;
            generateMessageBox(m);
        }
    }
    else
    {
        m << ERR_INCOMPLETE_FORM << " Invalid seller information " << "\n\t You must have a seller entered." << MESSAGE_OKAY_ONLY;
        generateMessageBox(m);
    }
    return false;
}

bool BookReceipts::validQuantity()
{
    if( ui->quantity->text().toInt() )
        return true;
    else
        return false;
}

bool BookReceipts::validPrice()
{
    bool decimalFound = false;
    bool nonNumberFound = false;
    foreach(QString ch, ui->unitPrice->text())
    {
        if( ch == "." )
            decimalFound = true;

        if( ch != "." && ch != "0")
            if ( !ch.toInt() )
                nonNumberFound = true;
    }

    if( decimalFound && !nonNumberFound && ui->unitPrice->text().toDouble() )
        return true;
    else
        return false;
}

bool BookReceipts::validTotal()
{
    bool decimalFound = false;
    bool nonNumberFound = false;
    foreach(QString ch, ui->total->text())
    {
        if( ch == "." )
            decimalFound = true;

        if( ch != "." && ch != "0")
            if ( !ch.toInt() )
                nonNumberFound = true;
    }

    if( decimalFound && !nonNumberFound && ui->total->text().toDouble() )
        return true;
    else
        return false;
}

void BookReceipts::generateMessageBox(QStringList messageDetails)
{
    MessageWindow * msg = new MessageWindow(messageDetails, this);
    connect(msg, SIGNAL(displayComplete(bool)), this, SLOT(messageWindowReturn(bool)));
    msg->show();
}

void BookReceipts::messageWindowReturn(bool x)
{
    if(!x)
        Q_UNUSED(x);

    switch(messageSwitch)
    {
    case 1:
        break;
    default:
        break;
    }
}

