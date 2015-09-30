#include "reportviewer.h"
#include "ui_reportviewer.h"

ReportViewer::ReportViewer(QString report, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ReportViewer)
{
    ui->setupUi(this);
    populateTable(report);
}

ReportViewer::~ReportViewer()
{
    delete ui;
}

void ReportViewer::populateTable(QString report)
{
    QStringList lines = report.split("\n");

    if( lines.length() > 0 )
    {
        QPalette* palette = new QPalette();
        palette->setColor(QPalette::Highlight,Qt::cyan);

        ui->tableWidget->setPalette(*palette);
        ui->tableWidget->setColumnCount(6);
        ui->tableWidget->setRowCount(lines.length());
        ui->tableWidget->verticalHeader()->setVisible(false);
        ui->tableWidget->setColumnWidth(1, 150);
        ui->tableWidget->setColumnWidth(2, 150);
        ui->tableWidget->setColumnWidth(3, 150);
        ui->tableWidget->setColumnWidth(4, 160);
        ui->tableWidget->setHorizontalHeaderLabels(
                    (QStringList() << "Receipt ID" << "Date"
                     << "Customer" << "Serviced By" << "Description"
                     << "Price")
        );

        int row = 0, col = row;
        foreach(QString line, lines)
        {
            QStringList cells = line.split(LIST_DELIMITER);
            foreach(QString cell, cells)
            {
                QTableWidgetItem *item = new QTableWidgetItem();
                item->setText( cell  );
                item->setFlags(item->flags() ^ Qt::ItemIsEditable);
                ui->tableWidget->setItem(row, col, item);

                col++;
            }
            row++;
            col = 0;
        }
    }
}

void ReportViewer::on_pushButton_clicked()
{
    this->close();
}


void ReportViewer::on_tableWidget_clicked(const QModelIndex &index)
{
    if( index.row() != -1)
    {
        ui->tableWidget->selectRow(index.row());
    }
}
