#ifndef MONTHLYREPORTGENERATOR_H
#define MONTHLYREPORTGENERATOR_H

#include <QDate>
#include <QDebug>
#include <QDialog>
#include "globals.h"
#include <QFileDialog>
#include "structures.h"
#include "socketclient.h"

namespace Ui {
class MonthlyReportGenerator;
}

class MonthlyReportGenerator : public QDialog
{
    Q_OBJECT

public:
    explicit MonthlyReportGenerator(_structs * data, QWidget *parent = 0);
    ~MonthlyReportGenerator();

private slots:
    void on_pushButton_clicked();

    void processData(QString);

private:
    Ui::MonthlyReportGenerator *ui;

    _structs * dataStructs;
    socketClient socketConn;

    QDate now;
    QHash<int, QString> months;
    QString retrieveNameById(QString);
    QString retieveFopalByDepartment(QString, QString);

    void generateMonthlyChargeBack(QString);
};

#endif // MONTHLYREPORTGENERATOR_H
