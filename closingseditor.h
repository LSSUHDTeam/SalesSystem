#ifndef CLOSINGSEDITOR_H
#define CLOSINGSEDITOR_H

#include <QDebug>
#include <QDialog>
#include <QDateTime>
#include "globals.h"
#include "structures.h"
#include "socketclient.h"
#include "messagewindow.h"

namespace Ui {
class ClosingsEditor;
}

class ClosingsEditor : public QDialog
{
    Q_OBJECT

public:
    explicit ClosingsEditor(_structs * data, QWidget *parent = 0);
    ~ClosingsEditor();

private slots:
    void processData(QString);

    void on_searchButton_clicked();

    void on_loadButton_clicked();

    void on_saveButton_clicked();

    void on_closeButton_clicked();

    void on_comments_textChanged();

    void on_closingsTable_clicked(const QModelIndex &index);

    void on_cancelEditButton_clicked();

private:
    Ui::ClosingsEditor *ui;

    _structs * dataStructs;
    socketClient socketConn;

    unsigned closingsCount;
    bool textEntrySwitch;
    bool availabilitySwitch;
    bool enableLoad;
    bool socketSwitch;
    QStringList reservedCharachters;

    void purgeClosings();
    void updateWindow();
    void toggleAvailability();
    bool hasBeenEdited();
    bool amountFoundIsValid();

    void generateMessageWindow(QStringList);
    QString filteredText(QString);

    struct ClosingEntry _closings[MAX_RECEIPTS_PER_SELECT];

};

#endif // CLOSINGSEDITOR_H
