#ifndef REPORTMANAGER_H
#define REPORTMANAGER_H

#include <QDialog>
#include "globals.h"
#include "structures.h"
#include "drawercloser.h"
#include "reportviewer.h"
#include "socketclient.h"
#include "closingseditor.h"
#include "rangereportgenerator.h"
#include "monthlyreportgenerator.h"

namespace Ui {
class ReportManager;
}

class ReportManager : public QDialog
{
    Q_OBJECT

public:
    explicit ReportManager(_structs * dataStructs, QWidget *parent = 0);
    ~ReportManager();

public slots:
    void processData(QString);

private slots:
    void on_closeButton_clicked();

    void on_monthlySalesButton_clicked();

    void on_ReportManager_rejected();

    void on_dateRangeReport_clicked();

    void on_closeDrawerButton_clicked();

    void on_closingsEditor_clicked();

    void on_dateRangeSelector_toggled(bool checked);

    void on_idRangeSelector_toggled(bool checked);

    void on_generateRport_clicked();

    void on_generateDepartmentalSummary_clicked();

private:
    Ui::ReportManager *ui;

    _structs * dataStructs;
    socketClient socketConn;
    QHash<QString,QString> deptNames;
    unsigned processSwitch;


    QString generateReport(QStringList);
    void generateSummaryReports(QString);
    QString retrieveNameById(QString id);
    QString retieveFopalByDepartment(QString dept);
    void writeOutReport(QString, QString);
    QString checkIfFileExistsAndName(QString, QString);
    QString checkIfFolderExistsAndName(QString, QString);

};

#endif // REPORTMANAGER_H
