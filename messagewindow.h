#ifndef MESSAGEWINDOW_H
#define MESSAGEWINDOW_H

#include <QDialog>
#include "globals.h"
#include "QDebug"

namespace Ui {
class MessageWindow;
}

class MessageWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MessageWindow(QStringList, QWidget *parent = 0);
    ~MessageWindow();

signals:
    void displayComplete(bool);

private slots:
    void on_okayButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::MessageWindow *ui;

};

#endif // MESSAGEWINDOW_H
