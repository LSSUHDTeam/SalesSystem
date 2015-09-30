#include "drawercloser.h"
#include "ui_drawercloser.h"

DrawerCloser::DrawerCloser(_structs * data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DrawerCloser)
{
    ui->setupUi(this);
    dataStructs = data;

    reservedCharachters << RESULT_MEMBER_DELIMITER << INCOMING_RESULT_DELIMITER << PACKAGE_DELIMITER
                        << LIST_DELIMITER << QUERY_DELIMITER << FOPAL_LIST_DELIMITER << DEAL_DELIMITER;

    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));

    textEntrySwitch = 1;
    ui->remainingCommentCharachterLabel->setText(QString::number(MAX_COMMENT_CHAR));

    populateClosingData();
}

DrawerCloser::~DrawerCloser()
{
    delete ui;
}

void DrawerCloser::purgeReceipts()
{
    drawerAmount = 0.00;
    for(unsigned i = 0; i < receiptCount; i++)
    {
        _receipts[receiptCount].id = "";
        _receipts[receiptCount].dateCreated = "";
        _receipts[receiptCount].seller = "";;
        _receipts[receiptCount].firstName = "";
        _receipts[receiptCount].lastName = "";
        _receipts[receiptCount].itemsSoldRaw = "";
        _receipts[receiptCount].totalPrice = "";
        _receipts[receiptCount].itemsSold = "";
        _receipts[receiptCount].description = "";
        _receipts[receiptCount].deptCode = "";
        _receipts[receiptCount].transactionType = "";
        _receipts[receiptCount].transactionCode = "";
    }
    receiptCount = 0;
}

void DrawerCloser::populateClosingData()
{
    // Make DB Call to get last dateTime, if none return NA, and select the day prior to
    // current date for close dateTime as clerk prefers dily closings
    // if not, grab endDate from most recent closing and get receipts from then until now and
    // populate the receipts struct array
    receiptCount = 0;
    populationSwitch = 0;
    subtotalInDrawer = 0.00;
    chargebackAmount = 0.00;
    drawerAmount = 0.00;
    checkAmount = 0.00;

    // Get receipts for drawer close
    socketConn.SubmitQuery(RECEIPT_QUERY,5);
    updateWindow();
}

void DrawerCloser::processData(QString data)
{
    if( data != EMPTY_DB_ENTRY)
    {
        QStringList chunks = data.split(INCOMING_RESULT_DELIMITER);

        switch( populationSwitch )
        {
        case 0:
        {
            purgeReceipts();

            // Grab the datetime that receipts start after
            closeStartDateTime = QDateTime::fromString(chunks[0], STANDARD_DATE_FORMAT);

            chunks.removeAt(0);

            // Process receipts
            receiptCount = 0;
            foreach (QString receipt, chunks)
            {
                QStringList cells = receipt.split(RESULT_MEMBER_DELIMITER);

                _receipts[receiptCount].id = cells[0];
                _receipts[receiptCount].dateCreated = cells[1];
                _receipts[receiptCount].seller = cells[2];
                _receipts[receiptCount].firstName = cells[3];
                _receipts[receiptCount].lastName = cells[4];
                _receipts[receiptCount].itemsSoldRaw = cells[5];
                _receipts[receiptCount].totalPrice = cells[6];

                QStringList itemList = cells[5].split(LIST_DELIMITER);

                QString itemsSold = "";

                foreach(QString item, itemList)
                {
                    for(int i = 0; i < dataStructs->counters.inventoryCount; i++)
                    {
                        if(dataStructs->inventory[i].inventoryId == item.split(PACKAGE_DELIMITER)[0])
                        {
                            itemsSold += dataStructs->inventory[i].itemName;
                            itemsSold += " x ";
                            itemsSold += item.split(PACKAGE_DELIMITER)[1];

                            if( itemList.indexOf(item) != itemList.length()-1)
                                itemsSold += ". ";
                        }
                    }
                }

                _receipts[receiptCount].itemsSold = itemsSold;
                _receipts[receiptCount].description = cells[7];
                _receipts[receiptCount].deptCode = cells[8];
                _receipts[receiptCount].transactionType = cells[9];
                _receipts[receiptCount].transactionCode = cells[10];

                if( _receipts[receiptCount].transactionType == RECEIPT_CASH)
                    drawerAmount += _receipts[receiptCount].totalPrice.toDouble();

                if( _receipts[receiptCount].transactionType == RECEIPT_CHECK)
                    checkAmount += _receipts[receiptCount].totalPrice.toDouble();

                if( _receipts[receiptCount].transactionType == RECEIPT_CHARGE_BACK)
                    chargebackAmount += _receipts[receiptCount].totalPrice.toDouble();

                receiptCount++;
            }
            break;
        }
        case 1:
        {
            // Returned from updating closings table
            break;
        }
        default:
            break;
        }
    }
}

void DrawerCloser::updateWindow()
{
    ui->receiptTable->clear();
    ui->lastCloseDate->setText(closeStartDateTime.toString(STANDARD_DATE_FORMAT));

    if ( receiptCount > 0 )
    {
        ui->receiptTable->setColumnCount(11);
        ui->receiptTable->setHorizontalHeaderLabels(
                    (QStringList() << "Id" << "Created" << "Seller" << "First Name" << "Last Name" << "Items Sold"
                                                     << "Total" << "Desc" << "Dept" << "Type" << "Code")
                    );
        ui->receiptTable->setColumnWidth(0, 50);
        ui->receiptTable->setColumnWidth(1, 110);
        ui->receiptTable->setColumnWidth(9, 100);
        ui->receiptTable->setColumnWidth(10, 50);

        subtotalInDrawer = drawerAmount + checkAmount + chargebackAmount;

        ui->cashInDrawer->setText(QString::number(drawerAmount,'f',2));
        ui->checksInDrawer->setText(QString::number(checkAmount,'f',2));
        ui->subtotalInDrawer->setText(QString::number(subtotalInDrawer,'f',2));
        ui->chargebacksInDrawer->setText(QString::number(chargebackAmount,'f',2));

        QPalette* palette = new QPalette();
        palette->setColor(QPalette::Highlight,Qt::darkBlue);
        ui->receiptTable->setPalette(*palette);

        ui->receiptTable->setRowCount(receiptCount);

        for(unsigned i =0; i < receiptCount; i++ )
        {
            QTableWidgetItem *id = new QTableWidgetItem();
            id->setText( _receipts[i].id  );
            id->setFlags(id->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 0, id);

            QTableWidgetItem *date = new QTableWidgetItem();
            date->setText( _receipts[i].dateCreated  );
            date->setFlags(date->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 1, date);

            QTableWidgetItem *seller = new QTableWidgetItem();
            seller->setText( _receipts[i].seller  );
            seller->setFlags(seller->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 2, seller);

            QTableWidgetItem *firstName = new QTableWidgetItem();
            firstName->setText( _receipts[i].firstName  );
            firstName->setFlags(firstName->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 3, firstName);

            QTableWidgetItem *lastName = new QTableWidgetItem();
            lastName->setText( _receipts[i].lastName  );
            lastName->setFlags(lastName->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 4, lastName);

            QTableWidgetItem *items = new QTableWidgetItem();
            items->setText( _receipts[i].itemsSold  );
            items->setFlags(items->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 5, items);

            QTableWidgetItem *totalPrice = new QTableWidgetItem();
            totalPrice->setText( _receipts[i].totalPrice  );
            totalPrice->setFlags(totalPrice->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 6, totalPrice);

            QTableWidgetItem *desc = new QTableWidgetItem();
            desc->setText( _receipts[i].description  );
            desc->setFlags(desc->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 7, desc);

            QTableWidgetItem *dept = new QTableWidgetItem();
            dept->setText( _receipts[i].deptCode  );
            dept->setFlags(dept->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 8, dept);

            QTableWidgetItem *tt = new QTableWidgetItem();
            tt->setText( _receipts[i].transactionType  );
            tt->setFlags(tt->flags() ^ Qt::ItemIsEditable);
            ui->receiptTable->setItem(i, 9, tt);

            QTableWidgetItem *tc = new QTableWidgetItem();
            tc->setText( _receipts[i].transactionCode  );
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
        ui->closeDrawer->setEnabled(false);
    }
}

void DrawerCloser::doUpdate()
{
    // Get receipts for drawer close
    socketConn.SubmitQuery(RECEIPT_QUERY,5);
    updateWindow();
}

void DrawerCloser::on_viewDetails_clicked()
{
    if( ui->receiptTable->currentRow() != -1 )
    {
        int i = ui->receiptTable->currentRow();
        ReceiptViewer * rv = new ReceiptViewer(_receipts[i],this);
        rv->show();
    }
}

void DrawerCloser::on_receiptTable_clicked(const QModelIndex &index)
{
    if( index.row() != -1 )
    {
        ui->receiptTable->selectRow(index.row());
    }
}

void DrawerCloser::on_comments_textChanged()
{
    if( textEntrySwitch )
    {
        QString comment_text = filteredText(ui->comments->toPlainText());
        QString new_comment_text = "";

        int count = 0;
        foreach(QString ch, comment_text)
        {
            if( count < MAX_COMMENT_CHAR )
            {
                new_comment_text += ch;
                count++;
            }
        }

        int remaining_chars = MAX_COMMENT_CHAR - count;

        // textEntrySwitch is flipped off and on here to prevent a crash
        // since clear and insert counts as textChanged, initiates an infinite loop
        textEntrySwitch = 0;
        ui->comments->clear();
        ui->comments->insertPlainText(new_comment_text);
        textEntrySwitch = 1;

        ui->remainingCommentCharachterLabel->setText(QString::number(remaining_chars));
    }
}


QString DrawerCloser::filteredText(QString text)
{
    // Filter Reserved Words
    QString temp = "";
    foreach(QString ch, text)
    {
        if( !reservedCharachters.contains(ch) )
        {
            temp += ch;
        }
    }
    text = temp;

    return text;
}

void DrawerCloser::on_closeDrawer_clicked()
{
    if( receiptCount > 0 )
    {

        if( amountFoundIsValid() )
        {
            QStringList departmentsInReceipts;

            // Get a list of departments in receipts
            for(unsigned i = 0; i < receiptCount; i++)
            {
                if( !departmentsInReceipts.contains(_receipts[i].deptCode) )
                    departmentsInReceipts.append(_receipts[i].deptCode);
            }

            QString dialogTitle = "Select where you would like to save the file";
            QString saveName = "IT-Drawer_Closing_"
                    + QDate::currentDate().toString(STANDARD_DATE_FORMAT_2)
                    + ".csv";

            QString filename = QFileDialog::getSaveFileName(this, dialogTitle, saveName, "CSV files (*.csv)", 0, 0);

            // Initialize and open the file.

            QFile theFile(filename);
            if( theFile.open(QFile::WriteOnly |QFile::Truncate))
            {
                QTextStream stream(&theFile);

                // Report date generated
                stream << " Drawer Closing : " << "," << QDateTime::currentDateTime().toString(STANDARD_DATE_FORMAT) << "\n\n";

                // Add headers
                stream << "\n"
                         << " Receipt Id " << ","
                         << " Date Created " << ","
                         << " Sold By " << ","
                         << " Customer Name " << ","
                         << " Receipt Total "
                         << "\n";

                stream << "\n" << "I.T Sales " << "\n";

                // Show IT Sales and calculate subtotal
                double subtotal = 0.00;
                for( unsigned i = 0; i < receiptCount; i++ )
                {
                    if(  _receipts[i].transactionCode != RECEIPT_COPY_CODE)
                    {
                        stream << _receipts[i].id
                               << ","
                               << _receipts[i].dateCreated
                               << ","
                               << _receipts[i].seller
                               << ","
                               << _receipts[i].firstName
                               << " "
                               << _receipts[i].lastName
                               << ","
                               << _receipts[i].totalPrice
                               << "\n"
                               << ","
                               << "Description :"
                               << ","
                               << _receipts[i].itemsSold
                               << "\n\n";
                        subtotal += _receipts[i].totalPrice.toDouble();
                    }
                }
                stream << "\n\n" << "End of I.T Sales" << "," << "," << ","
                       << "I.T Subtotal : " << "," << QString::number(subtotal, 'f', 2);

                stream << "\n\n" << "Photocopies" << "\n";

                // Show Photocopies
                double photo_subtotal = 0.00;
                for( unsigned i = 0; i < receiptCount; i++ )
                {
                    if(  _receipts[i].transactionCode == RECEIPT_COPY_CODE)
                    {

                        stream << _receipts[i].id
                               << ","
                               << _receipts[i].dateCreated
                               << ","
                               << _receipts[i].seller
                               << ","
                               << _receipts[i].firstName
                               << " "
                               << _receipts[i].lastName
                               << ","
                               << _receipts[i].totalPrice
                               << "\n"
                               << ","
                               << "Description :"
                               << ","
                               << _receipts[i].itemsSold
                               << "\n\n";
                        photo_subtotal += _receipts[i].totalPrice.toDouble();
                    }
                }
                stream << "\n\n" << "End of photocopies" << "," << "," << ","
                       << "Photocopy Subtotal : " << "," << QString::number(photo_subtotal, 'f', 2);

                stream << "\n\n\n" << "," << "  SUMMARY  " << "\n\n";
                stream << " I.T Income " << "," << QString::number(subtotal, 'f', 2)
                       << "\n" << "Photocopy Income " << "," << QString::number(photo_subtotal, 'f', 2)
                       << "\n" << "Total : " << "," << QString::number(subtotal + photo_subtotal, 'f', 2);

                // Construct update for db, and update.

                QString update = closeStartDateTime.toString(STANDARD_DATE_FORMAT);
                update += RESULT_MEMBER_DELIMITER;
                update += QDateTime::currentDateTime().toString(STANDARD_DATE_FORMAT);
                update += RESULT_MEMBER_DELIMITER;
                update += QString::number(drawerAmount,'f',2);
                update += RESULT_MEMBER_DELIMITER;
                update += ui->amountFound->text();
                update += RESULT_MEMBER_DELIMITER;

                if(  filteredText(ui->comments->toPlainText()).length() > 1)
                    update += filteredText(ui->comments->toPlainText());
                else
                    update += EMPTY_DB_ENTRY;

                socketConn.SubmitQuery(CLOSING_QUERY, 0 ,0, update);

                ui->lastCloseDate->setText(QDateTime::currentDateTime().toString(STANDARD_DATE_FORMAT));
                ui->remainingCommentCharachterLabel->setText(QString::number(MAX_COMMENT_CHAR));
                ui->cashInDrawer->clear();
                ui->amountFound->clear();
                ui->comments->clear();

                populateClosingData();
            }
        }
        else
        {
            QStringList m;
            m << ERR_INCOMPLETE_FORM << " Invalid Drawer Amount " << "\n Amount must be in decimal format " << MESSAGE_OKAY_ONLY;
            MessageWindow * msg = new MessageWindow(m, this);
            msg->show();
        }
    }
    else
    {
        QStringList m;
        m << ERR_INCOMPLETE_FORM << " No new Receipts " << "\n No receipts since last close. " << MESSAGE_OKAY_ONLY;
        MessageWindow * msg = new MessageWindow(m, this);
        msg->show();
    }
}

bool DrawerCloser::amountFoundIsValid()
{
    bool decimalFound = false;
    bool nonNumberFound = false;
    foreach(QString ch, ui->amountFound->text())
    {
        if( ch == "." )
            decimalFound = true;

        if( ch != "." && ch != "0")
            if ( !ch.toInt() )
                nonNumberFound = true;
    }
    if(decimalFound)
    {
        if(!nonNumberFound)
        {
            if(ui->amountFound->text().toDouble())
            {
                return true;
            }
        }
    }
    return false;
}


void DrawerCloser::on_removeCloseButton_clicked()
{
    int index = ui->receiptTable->currentRow();
    if( index > -1 )
    {
        for( int i = index; i < receiptCount; i++ )
        {
            _receipts[i] = _receipts[i+1];
        }
        receiptCount--;
        updateWindow();
    }
}
