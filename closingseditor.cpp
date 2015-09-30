#include "closingseditor.h"
#include "ui_closingseditor.h"

ClosingsEditor::ClosingsEditor(_structs * data, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ClosingsEditor)
{
    ui->setupUi(this);

    dataStructs = data;

    textEntrySwitch = 1;
    reservedCharachters << RESULT_MEMBER_DELIMITER << INCOMING_RESULT_DELIMITER << PACKAGE_DELIMITER
                        << LIST_DELIMITER << QUERY_DELIMITER << FOPAL_LIST_DELIMITER << DEAL_DELIMITER;

    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));
    socketSwitch = 0;


    ui->searchStart->setDate(QDate::currentDate());
    ui->searchEnd->setDate(QDate::currentDate());
    enableLoad = true;
}

ClosingsEditor::~ClosingsEditor()
{
    delete ui;
}

void ClosingsEditor::purgeClosings()
{
    for(unsigned i = 0; i < closingsCount; i++)
    {
        _closings[i].id = "";
        _closings[i].startDate = "";
        _closings[i].endDate = "";
        _closings[i].calculatedAmount = "";
        _closings[i].drawerAmount = "";
        _closings[i].comments = "";
    }
}

void ClosingsEditor::processData(QString data)
{
    if( socketSwitch == 0 )
    {
        ui->errLabel->clear();

        if( data != EMPTY_DB_ENTRY )
        {
            purgeClosings();

            closingsCount = 0;
            QStringList closings = data.split(INCOMING_RESULT_DELIMITER);
            foreach(QString close, closings)
            {
                QStringList cells = close.split(RESULT_MEMBER_DELIMITER);
                _closings[closingsCount].id = cells[0];
                _closings[closingsCount].startDate = cells[1];
                _closings[closingsCount].endDate = cells[2];
                _closings[closingsCount].calculatedAmount = cells[3];
                _closings[closingsCount].drawerAmount = cells[4];
                _closings[closingsCount].comments = cells[5];
                closingsCount++;
            }
            updateWindow();
        }
        else
        {
            ui->errLabel->setText("No Closings.");
        }
    }
    socketSwitch = 0;
}

void ClosingsEditor::updateWindow()
{
    // Populate the table with _closing[n] information

    if ( closingsCount > 0 )
    {
        ui->closingsTable->clear();
        ui->closingsTable->setColumnCount(6);
        ui->closingsTable->setHorizontalHeaderLabels(
                    (QStringList() << "Id" << "Start Date"
                     << "End Date" << "Calculated Amount"
                     << "Amount Counted" << "Comments")
        );
        ui->closingsTable->setColumnWidth(0, 50);

        QPalette* palette = new QPalette();
        palette->setColor(QPalette::Highlight,Qt::darkBlue);
        ui->closingsTable->setPalette(*palette);

        ui->closingsTable->setRowCount(closingsCount);

        for(unsigned i =0; i < closingsCount; i++ )
        {
            QTableWidgetItem *id = new QTableWidgetItem();
            id->setText( _closings[i].id  );
            id->setFlags(id->flags() ^ Qt::ItemIsEditable);
            ui->closingsTable->setItem(i, 0, id);

            QTableWidgetItem *sd = new QTableWidgetItem();
            sd->setText( _closings[i].startDate  );
            sd->setFlags(sd->flags() ^ Qt::ItemIsEditable);
            ui->closingsTable->setItem(i, 1, sd);

            QTableWidgetItem *ed = new QTableWidgetItem();
            ed->setText( _closings[i].endDate  );
            ed->setFlags(ed->flags() ^ Qt::ItemIsEditable);
            ui->closingsTable->setItem(i, 2, ed);

            QTableWidgetItem *ca = new QTableWidgetItem();
            ca->setText( _closings[i].calculatedAmount  );
            ca->setFlags(ca->flags() ^ Qt::ItemIsEditable);
            ui->closingsTable->setItem(i, 3, ca);

            QTableWidgetItem *da = new QTableWidgetItem();
            da->setText( _closings[i].drawerAmount  );
            da->setFlags(da->flags() ^ Qt::ItemIsEditable);
            ui->closingsTable->setItem(i, 4, da);

            QTableWidgetItem *com = new QTableWidgetItem();
            com->setText( _closings[i].comments  );
            com->setFlags(com->flags() ^ Qt::ItemIsEditable);
            ui->closingsTable->setItem(i, 5, com);
        }
    }
}

void ClosingsEditor::on_searchButton_clicked()
{
    availabilitySwitch = 0;
    toggleAvailability();

    ui->errLabel->clear();
    if( ui->searchStart->date() != ui->searchEnd->date() )
    {
        if( ui->searchStart->date() < ui->searchEnd->date() )
        {
            QString range = ui->searchStart->date().toString(STANDARD_DATE_FORMAT_2) + "|"
                    + ui->searchEnd->date().toString(STANDARD_DATE_FORMAT_2);
            socketConn.SubmitQuery(CLOSING_QUERY, 1, 0, range);
        }
        else
        {
            ui->errLabel->setText("Invalid Range");
        }
    }
    else
    {
        ui->errLabel->setText("Invalid Range");
    }
}

void ClosingsEditor::on_cancelEditButton_clicked()
{
    // CLEAR OUT GROUP BOX AND UNSELECT THE TABLE, REINABLE LOAD, DISABLE SAVE AND CANCEL
    availabilitySwitch = 0;
    toggleAvailability();

}

void ClosingsEditor::on_loadButton_clicked()
{
    // Load selected item into editing fields and toggle availability to match

    if( ui->closingsTable->currentRow() != -1 )
    {
        int i = ui->closingsTable->currentRow();

        ui->closingStartDate->setDateTime(QDateTime::fromString(_closings[i].startDate, STANDARD_DATE_FORMAT));
        ui->closingEndDate->setDateTime( QDateTime::fromString(_closings[i].endDate, STANDARD_DATE_FORMAT) );
        ui->calculatedAmountLabel->setText(_closings[i].calculatedAmount);
        ui->comments->insertPlainText(_closings[i].comments);
        availabilitySwitch = 1;
        toggleAvailability();
    }
}

void ClosingsEditor::on_saveButton_clicked()
{
    // Check for issues then update the server
    if ( hasBeenEdited() )
    {
        // If its been edited, check to see if drawer amount is different
        if ( ui->drawerAmount->text().length() > 0 )
        {
            // If it is, see if its a valid amount.
            if ( !amountFoundIsValid() )
            {
                QStringList m;
                m << ERR_INCOMPLETE_FORM << "Drawer Amound Invalid" << "\n The entered drawer amount is invalid. " << MESSAGE_OKAY_ONLY;
                generateMessageWindow(m);
            }
            else
            {

                // Add ID
                QString update = _closings[ui->closingsTable->currentRow()].id;
                update += QUERY_DELIMITER;

                // Add Info
                update += ui->closingStartDate->dateTime().toString(STANDARD_DATE_FORMAT);
                update += RESULT_MEMBER_DELIMITER;
                update += ui->closingEndDate->dateTime().toString(STANDARD_DATE_FORMAT);
                update += RESULT_MEMBER_DELIMITER;
                update += _closings[ui->closingsTable->currentRow()].calculatedAmount;
                update += RESULT_MEMBER_DELIMITER;

                if( ui->drawerAmount->text().length() > 0 )
                {
                    if( amountFoundIsValid() )
                    {
                        update += ui->drawerAmount->text();
                    }
                }
                else
                {
                    update += _closings[ui->closingsTable->currentRow()].drawerAmount;
                }

                update += RESULT_MEMBER_DELIMITER;

                if( filteredText(ui->comments->toPlainText()).length() > 0 &&
                    filteredText(ui->comments->toPlainText()) != _closings[ui->closingsTable->currentRow()].comments)
                {
                    update += filteredText(ui->comments->toPlainText());
                }
                else
                {
                    update += _closings[ui->closingsTable->currentRow()].comments;
                }

                // Push Update
                socketSwitch = 1;
                socketConn.SubmitQuery(CLOSING_QUERY,2,0,update);

                QStringList m;
                m << "Complete." << "Closing updated" << "\n The entry has been updated" << MESSAGE_OKAY_ONLY;
                generateMessageWindow(m);

                availabilitySwitch = 0;
                toggleAvailability();
            }
        }
    }
    else
    {
        QStringList m;
        m << ERR_INCOMPLETE_FORM << "No Editions" << "\n Nothing has been edited" << MESSAGE_OKAY_ONLY;
        generateMessageWindow(m);
    }

}

void ClosingsEditor::on_closeButton_clicked()
{
    this->close();
}

void ClosingsEditor::toggleAvailability()
{
    if( availabilitySwitch )
    {
        ui->cancelEditButton->setEnabled(true);
        ui->saveButton->setEnabled(true);
        ui->groupBox->setEnabled(true);
        ui->loadButton->setEnabled(false);
        enableLoad = false;
    }
    else
    {
        ui->comments->clear();
        ui->closingStartDate->clear();
        ui->closingEndDate->clear();
        ui->calculatedAmountLabel->clear();

        enableLoad = true;
        ui->cancelEditButton->setEnabled(false);
        ui->saveButton->setEnabled(false);
        ui->groupBox->setEnabled(false);
        ui->loadButton->setEnabled(false);
    }

    // CHanges avaialbility of buttons as per user state
}

QString ClosingsEditor::filteredText(QString text)
{
    // Filter Reserved Words

    if ( text.length() > 0 )
    {
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
    else
    {
        return "";
    }
}

void ClosingsEditor::on_comments_textChanged()
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

void ClosingsEditor::on_closingsTable_clicked(const QModelIndex &index)
{
    if( index.row() != -1 )
    {
        if( enableLoad )
        {
            ui->loadButton->setEnabled(true);
        }
        ui->closingsTable->selectRow(index.row());
    }
}

bool ClosingsEditor::amountFoundIsValid()
{
    bool decimalFound = false;
    bool nonNumberFound = false;
    foreach(QString ch, ui->drawerAmount->text())
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
            if(ui->drawerAmount->text().toDouble())
            {
                return true;
            }
        }
    }
    return false;
}

bool ClosingsEditor::hasBeenEdited()
{
    bool r = false;

    int i = ui->closingsTable->currentRow();

    if ( ui->closingStartDate->dateTime().toString(STANDARD_DATE_FORMAT) != _closings[i].startDate )
        r = true;

    if ( ui->closingEndDate->dateTime().toString(STANDARD_DATE_FORMAT) != _closings[i].endDate )
        r = true;

    if ( ui->drawerAmount->text().length() > 0 )
        r = true;

    if ( filteredText(ui->comments->toPlainText()) != _closings[i].comments )
        r = true;

    return r;
}

void ClosingsEditor::generateMessageWindow(QStringList message)
{
    MessageWindow * mw = new MessageWindow(message, this);
    mw->show();
}

