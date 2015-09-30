#include "lookupmanager.h"
#include "ui_lookupmanager.h"

LookupManager::LookupManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LookupManager)
{
    ui->setupUi(this);
}

LookupManager::~LookupManager()
{
    delete ui;
}
