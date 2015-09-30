#ifndef REPORTVIEWER_H
#define REPORTVIEWER_H

#include <QDialog>
#include <QDebug>
#include "globals.h"

namespace Ui {
class ReportViewer;
}

class ReportViewer : public QDialog
{
    Q_OBJECT

public:
    explicit ReportViewer(QString report, QWidget *parent = 0);
    ~ReportViewer();

private slots:
    void on_pushButton_clicked();


    void on_tableWidget_clicked(const QModelIndex &index);

private:
    Ui::ReportViewer *ui;
    QList<QStringList> currentReport;

    void populateTable(QString);

};

#endif // REPORTVIEWER_H
