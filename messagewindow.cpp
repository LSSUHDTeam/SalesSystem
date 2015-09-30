#include "messagewindow.h"
#include "ui_messagewindow.h"

MessageWindow::MessageWindow( QStringList messageDetails, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MessageWindow)
{
    ui->setupUi(this);

    /*
        0 - Dialog Title
        1 - Message Header
        2 - Message String
        3 - Button Options
    */

    QWidget::setWindowTitle(messageDetails[0]);
    ui->messageHeader->setText(messageDetails[1]);
    ui->messageBody->insertPlainText(messageDetails[2]);

    if( messageDetails[3] == MESSAGE_OKAY_ONLY )
    {
        ui->cancelButton->setHidden(true);
    }
    else if( messageDetails[3] == MESSAGE_CANCEL_ONLY )
    {
        ui->okayButton->setHidden(true);
    }
    else if ( messageDetails[3] == MESSAGE_OKAY_CANCEL )
    {
        ui->cancelButton->setHidden(false);
        ui->okayButton->setHidden(false);
    }
}

MessageWindow::~MessageWindow()
{
    delete ui;
}

void MessageWindow::on_okayButton_clicked()
{
    emit displayComplete(true);
    this->close();
}

void MessageWindow::on_cancelButton_clicked()
{
    emit displayComplete(false);
    this->close();
}
