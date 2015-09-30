#include "salewindow.h"
#include "ui_salewindow.h"

SaleWindow::SaleWindow(_structs * _dataStructs, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SaleWindow)
{
    dataStructs = _dataStructs;

    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));

    ui->setupUi(this);

    ui->dateTimeLabel->setText(QDateTime::currentDateTime().toString(STANDARD_DATE_FORMAT));

    reservedCharachters << RESULT_MEMBER_DELIMITER << INCOMING_RESULT_DELIMITER << PACKAGE_DELIMITER
                        << LIST_DELIMITER << QUERY_DELIMITER << FOPAL_LIST_DELIMITER << DEAL_DELIMITER;


    updateWindow();
}

SaleWindow::~SaleWindow()
{
    delete ui;
}

void SaleWindow::updateWindow()
{
    currentTotal = 0.00;

    /*
        Transaction dropdowns
    */
    ui->codeComboBox->clear();
    ui->typeComboBox->clear();
    ui->deptCodeComboBox->clear();

    ui->codeComboBox->addItems(dataStructs->saleInformation.codes);
    ui->typeComboBox->addItems(dataStructs->saleInformation.types);

    ui->deptCodeComboBox->addItem("Student");
    for(int i = 0; i < dataStructs->counters.departmentCount; i++ )
    {
        ui->deptCodeComboBox->addItem(dataStructs->departments[i].name);
    }

    updateAvailableItems();
}

void SaleWindow::updateAvailableItems()
{
    /*
        Inventory List
    */
    availableItemsList.clear();
    ui->availableItems->clear();
    for( int i = 0; i < dataStructs->counters.inventoryCount; i++ )
    {
            availableItemsList.append(dataStructs->inventory[i].itemName + " [" + QString::number(dataStructs->inventory[i].quantity) + "]");
    }
    ui->availableItems->addItems(availableItemsList);
}

// Update the list of inventory items
void SaleWindow::updateSoldItems()
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

// Populate Quantity Combos Under Available Items
void SaleWindow::on_availableItems_clicked(const QModelIndex &index)
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

void SaleWindow::on_addItemButton_clicked()
{
    if( ui->availableItems->currentRow() > -1 )
    {
        int index = ui->availableItems->currentRow();

        if ( dataStructs->inventory[index].quantity > 0 )
        {
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
}

// Populate Quantity Combos Under Sold Items
void SaleWindow::on_itemsSold_clicked(const QModelIndex &index)
{
    halt = true;
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
    halt = false;
}

// Remove Item From Order
void SaleWindow::on_removeItemButton_clicked()
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


void SaleWindow::calculateAndSetPrice()
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


void SaleWindow::on_finishOrderButton_clicked()
{
    QStringList m;
    saving = false;

    if( ui->seller->text().length() >= 3 )
    {
        if ( ui->description->text().length() >= 5 )
        {
            if( ui->customerFirstName->text().length() >= 4)
            {
                if( ui->customerLastName->text().length() >= 4)
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

void SaleWindow::generateMessageBox(QStringList messageDetails)
{
    MessageWindow * msg = new MessageWindow(messageDetails, this);
    connect(msg, SIGNAL(displayComplete(bool)), this, SLOT(messageWindowReturn(bool)));
    msg->show();
    // No Connection required ATM
}

void SaleWindow::messageWindowReturn(bool x)
{
    if( x && saving )
    {
        QString update = ui->dateTimeLabel->text();     update += RESULT_MEMBER_DELIMITER;
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

        update += ui->deptCodeComboBox->currentText();  update += RESULT_MEMBER_DELIMITER;
        update += ui->typeComboBox->currentText();      update += RESULT_MEMBER_DELIMITER;
        update += ui->codeComboBox->currentText();

        socketConn.SubmitQuery(RECEIPT_QUERY,1,0,update);
        printOutReceipt(update.split(RESULT_MEMBER_DELIMITER));

        emit forceUpdate(UPDATE_INVENTORY,"");
        this->close();
    }
}

void SaleWindow::on_availableItems_currentRowChanged(int currentRow)
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

void SaleWindow::on_itemsSold_currentRowChanged(int currentRow)
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

void SaleWindow::on_cancelButton_clicked()
{
    this->close();
}

void SaleWindow::processData(QString data)
{
    // Get the new id back from the DB
    receiptId = data;
}


/*

            PRINT THE RECEIPT

*/

void SaleWindow::printOutReceipt(QStringList data)
{
    QString itemDisplay = "";
    QStringList itemsSold = data[4].split(LIST_DELIMITER);
    foreach(QString item, itemsSold)
    {
        QString itemName;
        for(int i = 0; i < dataStructs->counters.inventoryCount; i++ )
        {
            if( dataStructs->inventory[i].inventoryId == item.split(PACKAGE_DELIMITER)[0] )
            {
                itemName = dataStructs->inventory[i].itemName + ", Qty : " + item.split(PACKAGE_DELIMITER)[1] + "\n ";
                itemDisplay += itemName;
            }
        }
    }
    data[4] = itemDisplay;
    data.append(receiptId);
    SalesPrinterEngine printerEngine(PRINTER_RECEIPT_STANDARD, this);
    printerEngine.printData(data);
}

QString SaleWindow::filteredText(QString text)
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
