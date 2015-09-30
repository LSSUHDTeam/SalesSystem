#ifndef LIBRARYMANAGER_H
#define LIBRARYMANAGER_H

#include <QDialog>
#include "structures.h"
#include "bookreceipts.h"

namespace Ui {
class LibraryManager;
}

class LibraryManager : public QDialog
{
    Q_OBJECT

public:
    explicit LibraryManager(_structs * _dataStructs, QWidget *parent = 0);
    ~LibraryManager();

private slots:
    void on_receiptChargeBackButton_clicked();

private:
    Ui::LibraryManager *ui;
    _structs * dataStructs;
};

#endif // LIBRARYMANAGER_H
