#include "inventorymanager.h"
#include "ui_inventorymanager.h"

InventoryManager::InventoryManager(_structs * data,QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InventoryManager)
{
    ui->setupUi(this);

    dataStructs = data;
    ui->saveAddButton->setEnabled(false);
    ui->deleteItemButton->setEnabled(false);
    connect(&socketConn, SIGNAL(dataReady(QString)), this, SLOT(processData(QString)));
    loadEditPage();
}

InventoryManager::~InventoryManager()
{
    delete ui;
}

/*

    Add Items Tab

*/

void InventoryManager::on_checkBox_toggled(bool checked)
{
    ui->dealEditor->setEnabled(checked);
}

void InventoryManager::on_cancelButton_clicked()
{
    this->close();
}

void InventoryManager::on_clearButton_clicked()
{
    ui->checkBox->setChecked(false);
    ui->itemNameField->clear();
    ui->descriptionField->clear();
    ui->dealPrice->clear();
    ui->dealQuantity->clear();
    ui->priceField->clear();
    ui->quantityField->clear();
}

void InventoryManager::on_addItemButton_clicked()
{
    ui->userInfoLabel->clear();
    // Make sure item doesn't already exist
    if( !itemAlreadyExists() )
    {
        if( ui->itemNameField->text().length() > 3 )
        {
            // Check price field to make sure it is valid
            switch(priceFieldIsValid())
            {
            case -1:
                ui->userInfoLabel->setText("Please enter the price in a decimal format.");
                break;
            case -2:
                ui->userInfoLabel->setText("A non-number was found in the price field.");
                break;
            case -3:
                ui->userInfoLabel->setText("The price could not be converted to a double. Please double check it.");
                break;
            case 1:
            {
                if( ui->quantityField->text().toInt() )
                {
                    if( ui->checkBox->isChecked() )
                    {

                        switch(dealIsValid())
                        {
                        case -1:
                            ui->userInfoLabel->setText("Please enter the deal price in a decimal format.");
                            break;
                        case -2:
                            ui->userInfoLabel->setText("A non-number was found in the deal price field.");
                            break;
                        case -3:
                            ui->userInfoLabel->setText("The deal price could not be converted to a double. Please check it.");
                            break;
                        case -4:
                            ui->userInfoLabel->setText("Deal quantity must be an integer.");
                            break;
                        case -5:
                            ui->userInfoLabel->setText("This is an invalid deal. Deals save money.");
                            break;
                        case 1:
                            callServerToAddItem();
                            break;
                        default:
                            break;
                        }
                    }
                    else
                    {
                        callServerToAddItem();
                    }
                }
                else
                {
                    ui->userInfoLabel->setText("Quantity must be an integer.");
                }
                break;
            }
            default:
                break;
            }
        }
        else
        {
            ui->userInfoLabel->setText(" Item name must be more than three charachters. ");
        }
    }
}

unsigned InventoryManager::priceFieldIsValid()
{
    // Check price field
    bool decimalFound = false;
    bool nonNumberFound = false;
    foreach(QString ch, ui->priceField->text())
    {
        if( ch == "." )
            decimalFound = true;

        if( ch != "." && ch != "0")
            if ( !ch.toInt() )
                nonNumberFound = true;
    }
    if(!decimalFound)
        return -1;
    if(nonNumberFound)
        return -2;
    if(!ui->priceField->text().toDouble())
        return -3;
    return 1;
}

unsigned InventoryManager::dealIsValid()
{
    // Check price field
    bool decimalFound = false;
    bool nonNumberFound = false;
    foreach(QString ch, ui->dealPrice->text())
    {
        if( ch == "." )
            decimalFound = true;

        if( ch != "." && ch != "0")
            if ( !ch.toInt() )
                nonNumberFound = true;
    }
    if(!decimalFound)
        return -1;
    if(nonNumberFound)
        return -2;
    if(!ui->dealPrice->text().toDouble())
        return -3;
    if( !ui->dealQuantity->text().toInt())
        return -4;
    if( ui->priceField->text().toDouble() * ui->dealQuantity->text().toInt() <= ui->dealPrice->text().toDouble() )
        return -5;
    return 1;
}

bool InventoryManager::itemAlreadyExists()
{
    QString item = ui->itemNameField->text().toUpper();
    for(int i =0; i < dataStructs->counters.inventoryCount; i++ )
    {
        if( dataStructs->inventory[i].itemName.toUpper() == item )
        {
            return true;
        }
    }
    return false;
}

void InventoryManager::callServerToAddItem()
{
    QString update = ui->itemNameField->text();
    update += RESULT_MEMBER_DELIMITER;
    if( ui->descriptionField->text().isEmpty() )
    {
        update += EMPTY_DB_ENTRY;
    }
    else
    {
        update += ui->descriptionField->text();
    }
    update += RESULT_MEMBER_DELIMITER;
    update += ui->priceField->text();
    update += RESULT_MEMBER_DELIMITER;
    update += ui->quantityField->text();
    update += RESULT_MEMBER_DELIMITER;
    if( ui->checkBox->isChecked() )
    {
        update += (ui->dealQuantity->text() + DEAL_DELIMITER + ui->dealPrice->text());
    }
    else
    {
        update += EMPTY_DB_ENTRY;
    }
    processSwitch = 1;
    socketConn.SubmitQuery(INVENTORY_QUERY,1,0,update);
}

void InventoryManager::processData(QString data)
{
    switch(processSwitch)
    {
    case 1:
        if(data == "1")
        {
            ui->userInfoLabel->setText("Item added.");
            ui->itemNameField->clear();
            ui->priceField->clear();
            ui->descriptionField->clear();
            ui->quantityField->clear();
            ui->checkBox->setChecked(false);
            ui->dealQuantity->clear();
            ui->dealPrice->clear();

            emit forceUpdate(UPDATE_INVENTORY,"");
            loadEditPage();
        }
        if(data == "-1")
            ui->userInfoLabel->setText("An error occured adding the item.");
        break;
    case 2:
        if(data == "1")
        {
            ui->itemName->clear();
            ui->itemDescription->clear();
            ui->itemPrice->clear();
            ui->itemQuantity->clear();
            ui->editDealCheckBox->setChecked(false);
            ui->editPageUserInfoLabel->setText("Item Deleted.");
            emit forceUpdate(UPDATE_INVENTORY,"");
            loadEditPage();
            ui->inventoryList->setEnabled(true);
            ui->saveAddButton->setEnabled(false);
            ui->deleteItemButton->setEnabled(false);
        }
        else
        {
            ui->editPageUserInfoLabel->setText("Error deleting item.");
        }
        break;
    case 3:
        if(data == "1")
        {
            ui->itemName->clear();
            ui->itemDescription->clear();
            ui->itemPrice->clear();
            ui->itemQuantity->clear();
            ui->editDealCheckBox->setChecked(false);
            ui->editPageUserInfoLabel->setText("Item Edited.");
            emit forceUpdate(UPDATE_INVENTORY,"");
            loadEditPage();
            ui->inventoryList->setEnabled(true);
        }
        else
        {
            ui->editPageUserInfoLabel->setText("Error editing item.");
        }
        break;
    default:
        break;
    }
}

/*

        Edit Page

*/

void InventoryManager::loadEditPage()
{
    ui->inventoryList->clear();
    for( int i = 0; i < dataStructs->counters.inventoryCount; i++ )
    {
        ui->inventoryList->addItem(new QListWidgetItem(dataStructs->inventory[i].itemName
                                                        + " x" + QString::number(dataStructs->inventory[i].quantity)));
    }
}

void InventoryManager::on_editDealCheckBox_toggled(bool checked)
{
    if( checked )
    {
        ui->editDealEditor->setEnabled(true);
    }
    else
    {
        ui->editDealEditor->setEnabled(false);
        ui->editPageDealQuantity->clear();
        ui->editPageDealPrice->clear();
    }
}

void InventoryManager::on_loadButton_clicked()
{
    int i = ui->inventoryList->currentRow();

    if( i != -1 )
    {

        ui->itemName->setText(dataStructs->inventory[i].itemName);
        ui->itemDescription->clear();
        ui->itemDescription->insertPlainText(dataStructs->inventory[i].description);
        ui->itemPrice->setText(QString::number(dataStructs->inventory[i].price, 'f', 2));
        ui->itemQuantity->setText(QString::number(dataStructs->inventory[i].quantity));

        if(dataStructs->inventory[i].deal != EMPTY_DB_ENTRY)
        {
            ui->editDealCheckBox->setChecked(true);
            ui->editPageDealQuantity->setText(dataStructs->inventory[i].deal.split(DEAL_DELIMITER)[0]);
            ui->editPageDealPrice->setText(dataStructs->inventory[i].deal.split(DEAL_DELIMITER)[1]);
        }
        else
        {
            ui->editDealCheckBox->setChecked(false);
        }

        ui->inventoryList->setEnabled(false);
        ui->saveAddButton->setEnabled(true);
        ui->deleteItemButton->setEnabled(true);
    }
}

void InventoryManager::on_cancelEditButton_clicked()
{
    ui->inventoryList->setEnabled(true);
    ui->saveAddButton->setEnabled(false);
    ui->itemName->clear();
    ui->itemDescription->clear();
    ui->itemPrice->clear();
    ui->itemQuantity->clear();
    ui->editDealCheckBox->setChecked(false);
    ui->deleteItemButton->setEnabled(false);
}

void InventoryManager::on_deleteItemButton_clicked()
{
    int i = ui->inventoryList->currentRow();
    if( i != -1 )
    {
        processSwitch = 2;
        socketConn.SubmitQuery(INVENTORY_QUERY,3,0,dataStructs->inventory[i].inventoryId);
    }
}

void InventoryManager::on_saveAddButton_clicked()
{
    if( ui->itemName->text().length() > 3 )
    {
        // Check price field to make sure it is valid
        switch(editPriceFieldIsValid())
        {
        case -1:
            ui->editPageUserInfoLabel->setText("Please enter the price in a decimal format.");
            break;
        case -2:
            ui->editPageUserInfoLabel->setText("A non-number was found in the price field.");
            break;
        case -3:
            ui->editPageUserInfoLabel->setText("The price could not be converted to a double. Please double check it.");
            break;
        case 1:
        {
            if( ui->itemQuantity->text().toInt() )
            {
                if( ui->editDealCheckBox->isChecked() )
                {

                    switch(editDealIsValid())
                    {
                    case -1:
                        ui->editPageUserInfoLabel->setText("Please enter the deal price in a decimal format.");
                        break;
                    case -2:
                        ui->editPageUserInfoLabel->setText("A non-number was found in the deal price field.");
                        break;
                    case -3:
                        ui->editPageUserInfoLabel->setText("The deal price could not be converted to a double. Please check it.");
                        break;
                    case -4:
                        ui->editPageUserInfoLabel->setText("Deal quantity must be an integer.");
                        break;
                    case -5:
                        ui->editPageUserInfoLabel->setText("This is an invalid deal. Deals save money.");
                        break;
                    case 1:
                        callServerToUpdateItem();
                        break;
                    default:
                        break;
                    }
                }
                else
                {
                    callServerToUpdateItem();
                }
            }
            else
            {
                ui->editPageUserInfoLabel->setText("Quantity must be an integer.");
            }
            break;
        }
        default:
            break;
        }
    }
    else
    {
        ui->editPageUserInfoLabel->setText(" Item name must be more than three charachters. ");
    }
}

unsigned InventoryManager::editPriceFieldIsValid()
{
    // Check price field
    bool decimalFound = false;
    bool nonNumberFound = false;
    foreach(QString ch, ui->itemPrice->text())
    {
        if( ch == "." )
            decimalFound = true;

        if( ch != "." && ch != "0")
            if ( !ch.toInt() )
                nonNumberFound = true;
    }
    if(!decimalFound)
        return -1;
    if(nonNumberFound)
        return -2;
    if(!ui->itemPrice->text().toDouble())
        return -3;
    return 1;
}

unsigned InventoryManager::editDealIsValid()
{
    bool decimalFound = false;
    bool nonNumberFound = false;
    foreach(QString ch, ui->editPageDealPrice->text())
    {
        if( ch == "." )
            decimalFound = true;

        if( ch != "." && ch != "0")
            if ( !ch.toInt() )
                nonNumberFound = true;
    }
    if(!decimalFound)
        return -1;
    if(nonNumberFound)
        return -2;
    if(!ui->editPageDealPrice->text().toDouble())
        return -3;
    if( !ui->editPageDealQuantity->text().toInt())
        return -4;
    if( ui->itemPrice->text().toDouble() * ui->editPageDealQuantity->text().toInt() <= ui->editPageDealPrice->text().toDouble() )
        return -5;
    return 1;
}

void InventoryManager::callServerToUpdateItem()
{
    int i = ui->inventoryList->currentRow();
    if( i != -1 )
    {
        processSwitch = 3;

        QString update = dataStructs->inventory[i].inventoryId;
        update += RESULT_MEMBER_DELIMITER;
        update += ui->itemName->text();
        update += RESULT_MEMBER_DELIMITER;
        update += ui->itemDescription->toPlainText();
        update += RESULT_MEMBER_DELIMITER;
        update += ui->itemPrice->text();
        update += RESULT_MEMBER_DELIMITER;
        update += ui->itemQuantity->text();

        if( ui->editDealCheckBox->isChecked() )
        {
            update += RESULT_MEMBER_DELIMITER;
            update += ui->editPageDealQuantity->text();
            update += DEAL_DELIMITER;
            update += ui->editPageDealPrice->text();
        }
        else
        {
            update += RESULT_MEMBER_DELIMITER;
            update += EMPTY_DB_ENTRY;
        }
        ui->saveAddButton->setEnabled(false);
        ui->deleteItemButton->setEnabled(false);

        socketConn.SubmitQuery(INVENTORY_QUERY,2,0,update);
    }
}

void InventoryManager::on_editCloseButton_clicked()
{
    this->close();
}
