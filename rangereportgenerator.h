#ifndef RANGEREPORTGENERATOR_H
#define RANGEREPORTGENERATOR_H

#include <QDate>
#include <QDebug>
#include <QDialog>
#include "globals.h"
#include <QFileDialog>
#include "structures.h"
#include "socketclient.h"

namespace Ui {
class RangeReportGenerator;
}

class RangeReportGenerator : public QDialog
{
    Q_OBJECT

public:
    explicit RangeReportGenerator(_structs * data, QWidget *parent = 0);
    ~RangeReportGenerator();

private slots:
    void on_generateReport_clicked();
    void processData(QString);

private:
    Ui::RangeReportGenerator *ui;

    _structs * dataStructs;
    socketClient socketConn;

    void generateRangedReport(QString);
    QString retrieveNameById(QString);
    QString retieveFopalByDepartment(QString);
};

#endif // RANGEREPORTGENERATOR_H
