#ifndef LOOKUPMANAGER_H
#define LOOKUPMANAGER_H

#include <QDialog>

namespace Ui {
class LookupManager;
}

class LookupManager : public QDialog
{
    Q_OBJECT

public:
    explicit LookupManager(QWidget *parent = 0);
    ~LookupManager();

private:
    Ui::LookupManager *ui;
};

#endif // LOOKUPMANAGER_H
