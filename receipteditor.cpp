#include "receipteditor.h"
#include "ui_receipteditor.h"

ReceiptEditor::ReceiptEditor(_structs * data,
                             struct receiptStruct* receipt,
                             QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReceiptEditor)
{
    ui->setupUi(this);

    voiding = false;
    _receipt = receipt;
    dataStructs = data;
    reservedCharachters << RESULT_MEMBER_DELIMITER << INCOMING_RESULT_DELIMITER << PACKAGE_DELIMITER
                        << LIST_DELIMITER << QUERY_DELIMITER << FOPAL_LIST_DELIMITER << DEAL_DELIMITER;

    populateWindow();
}

ReceiptEditor::~ReceiptEditor()
{
    delete ui;
}

void ReceiptEditor::populateWindow()
{
    // Fill dropdowns and text boxes with information available to select,
    // along with the idnex of the items selected.

    ui->dateTimeLabel->setText(_receipt->dateCreated);

    if( _receipt->seller == RECEIPT_DAILY_SALE )
    {
        ui->seller->setEnabled(false);
        ui->transactionGroup->setEnabled(false);
        ui->customerFirstName->setEnabled(false);
        ui->customerLastName->setEnabled(false);
        ui->voidReceiptButton->setEnabled(false);
    }
    ui->seller->setText(_receipt->seller);

    if(_receipt->description != EMPTY_DB_ENTRY)
    {
        ui->description->setText(_receipt->description);
    }
    ui->codeComboBox->addItems(dataStructs->saleInformation.codes);
    ui->typeComboBox->addItems(dataStructs->saleInformation.types);
    ui->deptCodeComboBox->addItem("Student");

    int i, j = -1;
    for(i = 0; i < dataStructs->counters.departmentCount; i++ )
    {
        ui->deptCodeComboBox->addItem(dataStructs->departments[i].name);
        if( dataStructs->departments[i].name == _receipt->deptCode)
        {
            j = i;
        }
    }
    if( j != -1 )
        ui->deptCodeComboBox->setCurrentIndex(j+1);
    else
        ui->deptCodeComboBox->setCurrentIndex(0);
    i = 0;
    foreach(QString item, dataStructs->saleInformation.codes)
    {
        if( item == _receipt->deptCode )
        {
            ui->codeComboBox->setCurrentIndex(i);
        }
        i++;
    }
    i = 0;
    foreach(QString item, dataStructs->saleInformation.types)
    {
        if( item == _receipt->transactionType )
        {
            ui->typeComboBox->setCurrentIndex(i);
        }
        i++;
    }

    ui->customerFirstName->setText(_receipt->firstName);
    ui->customerLastName->setText(_receipt->lastName);

    // Add sold items to the current sale
    foreach(QString item, _receipt->itemsSoldRaw.split(LIST_DELIMITER))
    {
        int quantity = item.split(PACKAGE_DELIMITER)[1].toInt();
        currentSale.insert(item.split(PACKAGE_DELIMITER)[0], quantity);
    }

    // Display the price
    currentTotal = _receipt->totalPrice.toDouble();
    ui->runningTotalLabel->setText(QString::number(currentTotal, 'f', 2));

    // Load available items
    updateAvailableItems();

    // Load sold items
    updateSoldItems();
}

void ReceiptEditor::updateAvailableItems()
{
    /*
        Inventory List
    */
    availableItemsList.clear();
    ui->availableItems->clear();
    for( int i = 0; i < dataStructs->counters.inventoryCount; i++ )
    {
        if( dataStructs->inventory[i].quantity > 0 )
        {
            availableItemsList.append(dataStructs->inventory[i].itemName + " [" + QString::number(dataStructs->inventory[i].quantity) + "]");
        }
    }
    ui->availableItems->addItems(availableItemsList);
}

void ReceiptEditor::updateSoldItems()
{
    ui->itemsSold->clear();

    foreach(QString itemId, currentSale.keys())
    {
        for( int i = 0; i < dataStructs->counters.inventoryCount; i++ )
        {
            if( dataStructs->inventory[i].inventoryId == itemId )
            {
                ui->itemsSold->addItem(new QListWidgetItem(dataStructs->inventory[i].itemName + " x" + QString::number(currentSale[itemId])));
            }
        }
    }
}

void ReceiptEditor::on_availableItems_clicked(const QModelIndex &index)
{
    if( index.row() > -1 )
    {
        ui->addQuantityComboBox->clear();

        for( int i = 1; i <= dataStructs->inventory[index.row()].quantity; i++ )
        {
            ui->addQuantityComboBox->addItem(QString::number(i));
        }

        if( dataStructs->inventory[index.row()].deal == EMPTY_DB_ENTRY )
        {
            ui->priceInfoLabel->setText(QString::number(dataStructs->inventory[index.row()].price, 'f', 2));
        }
        else
        {
            QString info = QString::number(dataStructs->inventory[index.row()].price, 'f', 2);
            info += " or "; info += dataStructs->inventory[index.row()].deal.split(DEAL_DELIMITER)[0];
            info += " for "; info += dataStructs->inventory[index.row()].deal.split(DEAL_DELIMITER)[1];
            ui->priceInfoLabel->setText(info);
        }
    }
}

void ReceiptEditor::on_itemsSold_clicked(const QModelIndex &index)
{
    if( index.row() > -1 )
    {
        int j = 0;
        QString currentItem = "";
        ui->priceInfoLabel->setText("");
        foreach(QString key, currentSale.keys())
        {
            if ( j == index.row() )
                currentItem = key;
            j++;
        }
        if( currentItem != "")
        {
            ui->removeQuantityComboBox->clear();
            for( int i = 1; i <= currentSale[currentItem]; i++ )
            {
                ui->removeQuantityComboBox->addItem(QString::number(i));
            }
        }
    }
}

void ReceiptEditor::on_addItemButton_clicked()
{
    if( ui->availableItems->currentRow() > -1 )
    {
        int index = ui->availableItems->currentRow();

        // Get the id of the item being added, and the selected quantity
        QString itemSelected = dataStructs->inventory[index].inventoryId;
        int quantity = ui->addQuantityComboBox->currentText().toInt();
        dataStructs->inventory[index].quantity -= quantity;

        if( currentSale.contains(itemSelected) )
        {
            currentSale[itemSelected] += quantity;
        }
        else
        {
            // Add item to sale
            currentSale.insert(itemSelected, quantity);
        }
        dataStructs->counters.inventoryIndex = index;

        halt = true;
        updateAvailableItems();
        calculateAndSetPrice();
        updateSoldItems();
        halt = false;
    }
}

void ReceiptEditor::on_removeItemButton_clicked()
{
    // Find currently selected item, and quantity in combobox
    // remove it from current sale and add what was taken back to inventor

    if( ui->itemsSold->currentRow() > -1 )
    {
        int j = 0;
        QString currentItem = "";
        foreach(QString key, currentSale.keys())
        {
            if ( j == ui->itemsSold->currentRow() )
                currentItem = key;
            j++;
        }
        if( currentItem != "")
        {
            int quantity = ui->removeQuantityComboBox->currentText().toInt();
            currentSale[currentItem]-=quantity;

            for(int k = 0; k < dataStructs->counters.inventoryCount; k++ )
            {
                if( dataStructs->inventory[k].inventoryId == currentItem)
                {
                    dataStructs->inventory[k].quantity += quantity;
                }
            }

            if( currentSale[currentItem] <= 0 )
            {
                currentSale.remove(currentItem);
            }
            else
            {
                ui->removeQuantityComboBox->clear();
                for( int i = 1; i <= currentSale[currentItem]; i++ )
                {
                    ui->removeQuantityComboBox->addItem(QString::number(i));
                }
            }
            halt = true;
            updateAvailableItems();
            calculateAndSetPrice();
            updateSoldItems();
            halt = false;
        }
    }
}

void ReceiptEditor::calculateAndSetPrice()
{
    currentTotal = 0.00;

    // Go Through every item sold
    foreach(QString sale, currentSale.keys())
    {
        // Find it in inventory
        for( int i = 0; i < dataStructs->counters.inventoryCount; i++ )
        {
            if( dataStructs->inventory[i].inventoryId == sale )
            {
                // Check if this item has a deal
                if( dataStructs->inventory[i].deal != EMPTY_DB_ENTRY )
                {
                    int dealQuantity = dataStructs->inventory[i].deal.split(DEAL_DELIMITER)[0].toInt();
                    double dealPrice = dataStructs->inventory[i].deal.split(DEAL_DELIMITER)[1].toDouble();

                    if( currentSale[sale] % dealQuantity == 0 )
                    {
                        // Deal found and is divided evenly, so add number of deals * deal price
                        int numberOfDeals = currentSale[sale] / dealQuantity;
                        currentTotal += numberOfDeals * dealPrice;
                    }
                    else if ( currentSale[sale] > dealQuantity )
                    {
                        // Deal is within number of total quantity, but not divided evenly.
                        // Find how many deals exist within total quantitiy, and give deal price for however many deals exist.
                        // Then add the remainder ad reqular price
                        int numberOfDealsInTotalQuantity = qFloor( currentSale[sale] / dealQuantity);
                        int remainderOfQuantity = currentSale[sale] - ( numberOfDealsInTotalQuantity * dealQuantity);

                        // Add the deals available to the toal amount
                        currentTotal += numberOfDealsInTotalQuantity * dealPrice;

                        // Add the remainder at original price.
                        currentTotal += remainderOfQuantity * dataStructs->inventory[i].price;
                    }
                    else
                    {
                        currentTotal += currentSale[sale] * dataStructs->inventory[i].price;
                    }
                }
                else
                {
                    // Item is not eligible for a deal, so add up total amount.
                    currentTotal += currentSale[sale] * dataStructs->inventory[i].price;
                }

            }
        }
    }
    ui->runningTotalLabel->setText(QString::number(currentTotal, 'f', 2));
}

void ReceiptEditor::on_availableItems_currentRowChanged(int currentRow)
{
    // This connection was causing a crash with qList when updating the table.
    // The boolean halt was implemented to ensure its stability
    if( !halt )
    {
        ui->addQuantityComboBox->clear();
        for( int i = 1; i <= dataStructs->inventory[currentRow].quantity; i++ )
        {
            ui->addQuantityComboBox->addItem(QString::number(i));
        }
        if( dataStructs->inventory[currentRow].deal == EMPTY_DB_ENTRY )
        {
            ui->priceInfoLabel->setText(QString::number(dataStructs->inventory[currentRow].price, 'f', 2));
        }
        else
        {
            QString info = QString::number(dataStructs->inventory[currentRow].price, 'f', 2);
            info += " or "; info += dataStructs->inventory[currentRow].deal.split(DEAL_DELIMITER)[0];
            info += " for "; info += dataStructs->inventory[currentRow].deal.split(DEAL_DELIMITER)[1];
            ui->priceInfoLabel->setText(info);
        }
    }
}

void ReceiptEditor::on_itemsSold_currentRowChanged(int currentRow)
{
    // This connection was causing a crash with qList when updating the table.
    // The boolean halt was implemented to ensure its stability
    if( !halt )
    {
        ui->removeQuantityComboBox->clear();

        int j = 0;
        QString currentItem = "";
        foreach(QString key, currentSale.keys())
        {
            if ( j == currentRow )
                currentItem = key;
            j++;
        }
        if( currentItem != "")
        {
            ui->removeQuantityComboBox->clear();
            for( int i = 1; i <= currentSale[currentItem]; i++ )
            {
                ui->removeQuantityComboBox->addItem(QString::number(i));
            }
        }
    }
}

void ReceiptEditor::on_cancelButton_clicked()
{
    this->close();
}

void ReceiptEditor::on_doneButton_clicked()
{
    QStringList m;
    saving = false;

    if( ui->seller->text().length() >= 3 )
    {
        if ( ui->description->text().length() >= 5 || _receipt->seller == RECEIPT_DAILY_SALE)
        {
            if( ui->customerFirstName->text().length() >= 4 || _receipt->seller == RECEIPT_DAILY_SALE)
            {
                if( ui->customerLastName->text().length() >= 4 || _receipt->seller == RECEIPT_DAILY_SALE)
                {
                    if( currentTotal != 0.00 )
                    {
                        saving = true;
                        QString userVerifyMessage = " ";
                        userVerifyMessage += ui->dateTimeLabel->text();
                        userVerifyMessage += "\n ";
                        userVerifyMessage += ui->seller->text();
                        userVerifyMessage += "\n ";
                        userVerifyMessage += ui->customerFirstName->text();
                        userVerifyMessage += "\n ";
                        userVerifyMessage += ui->customerLastName->text();
                        userVerifyMessage += "\n Code: ";
                        userVerifyMessage += ui->codeComboBox->currentText();
                        userVerifyMessage += "\n Dept Code: ";
                        userVerifyMessage += ui->deptCodeComboBox->currentText();
                        userVerifyMessage += "\n Sale Type: ";
                        userVerifyMessage += ui->typeComboBox->currentText();
                        userVerifyMessage += "\n\n";
                        userVerifyMessage += ui->description->text();
                        messageReturnSwitch = 1;
                        m << "Verify Data" << "Please make sure this data is correct." << userVerifyMessage << MESSAGE_OKAY_CANCEL;
                        generateMessageBox(m);
                    }
                    else
                    {
                        m << ERR_INCOMPLETE_FORM << "No Orders Added" << "\n There must be items on the order." << MESSAGE_OKAY_ONLY;
                        generateMessageBox(m);
                    }
                }
                else
                {
                    m << ERR_INCOMPLETE_FORM << "Invalid Customer Last Name" << "\n Customer last name field must have 4 or more charachters." << MESSAGE_OKAY_ONLY;
                    generateMessageBox(m);
                }
            }
            else
            {
                m << ERR_INCOMPLETE_FORM << "Invalid Customer First Name" << "\n Customer first name field must have 4 or more charachters." << MESSAGE_OKAY_ONLY;
                generateMessageBox(m);
            }
        }
        else
        {
            m << ERR_INCOMPLETE_FORM << "Invalid Description" << "\n\tYou must have an accurate description entered.\n\t" << MESSAGE_OKAY_ONLY;
            generateMessageBox(m);
        }
    }
    else
    {
        m << ERR_INCOMPLETE_FORM << "Invalid Seller Information" << "\n\tYou must have a seller entered.\n\t( 3 or more charachters )" << MESSAGE_OKAY_ONLY;
        generateMessageBox(m);
    }
}

void ReceiptEditor::generateMessageBox(QStringList messageDetails)
{
    MessageWindow * msg = new MessageWindow(messageDetails, this);
    connect(msg, SIGNAL(displayComplete(bool)), this, SLOT(messageWindowReturn(bool)));
    msg->show();
}

void ReceiptEditor::messageWindowReturn(bool x)
{


    switch( messageReturnSwitch )
    {
    case 1:
    {
        if( x && saving )
        {
            QString update = _receipt->id;                  update += RESULT_MEMBER_DELIMITER;
            update += ui->dateTimeLabel->text();            update += RESULT_MEMBER_DELIMITER;
            update += filteredText(ui->seller->text());                   update += RESULT_MEMBER_DELIMITER;
            update += filteredText(ui->customerFirstName->text());        update += RESULT_MEMBER_DELIMITER;
            update += filteredText(ui->customerLastName->text());         update += RESULT_MEMBER_DELIMITER;

            int i = 1;
            foreach(QString key, currentSale.keys())
            {
                update += key;
                update += PACKAGE_DELIMITER;
                update += QString::number(currentSale[key]);

                if( i != currentSale.keys().count() )
                    update += LIST_DELIMITER;
                i++;
            }

            update += RESULT_MEMBER_DELIMITER;
            update += ui->runningTotalLabel->text();
            update += RESULT_MEMBER_DELIMITER;

            if( ui->description->text() != "" && ui->description->text() != " ")
            {
                update += filteredText(ui->description->text());
            }
            else
            {
                update += EMPTY_DB_ENTRY;
            }
            update += RESULT_MEMBER_DELIMITER;

            if( _receipt->seller != RECEIPT_DAILY_SALE )
            {
                update += ui->deptCodeComboBox->currentText();  update += RESULT_MEMBER_DELIMITER;
                update += ui->typeComboBox->currentText();      update += RESULT_MEMBER_DELIMITER;
                update += ui->codeComboBox->currentText();
            }
            else
            {
                update += EMPTY_DB_ENTRY;  update += RESULT_MEMBER_DELIMITER;
                update += EMPTY_DB_ENTRY;      update += RESULT_MEMBER_DELIMITER;
                update += EMPTY_DB_ENTRY;
            }

            socketConn.SubmitQuery(RECEIPT_QUERY,3,0,update);

            emit forceUpdate();
            this->close();
        }
        break;
    }
    case 2:
    {
        if( voiding )
        {
            if( x )
            {
                socketConn.SubmitQuery(RECEIPT_QUERY,4,0,_receipt->id);
                ui->doneButton->setEnabled(false);
                ui->doneButton->hide();
                emit forceUpdate();
                ui->cancelButton->setText("Done");
            }
        }
        else
        {
            voiding = false;
        }
        break;
    }
    default:
        break;
    }



}

QString ReceiptEditor::filteredText(QString text)
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

void ReceiptEditor::on_voidReceiptButton_clicked()
{
    messageReturnSwitch = 2;
    voiding = true;
    QStringList m ;
    m << "ALERT" << "You are about to VOID this transaction" << "\n Would you like to void this transaction ?" << MESSAGE_OKAY_CANCEL;
    generateMessageBox(m);
}
