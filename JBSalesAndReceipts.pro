#-------------------------------------------------
#
# Project created by QtCreator 2014-12-22T08:38:04
#
#-------------------------------------------------

QT       += core gui network printsupport multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = JBSalesAndReceipts
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    socketclient.cpp \
    structures.cpp \
    salewindow.cpp \
    messagewindow.cpp \
    salesprinterengine.cpp \
    transactionviewer.cpp \
    receiptviewer.cpp \
    userverification.cpp \
    reportmanager.cpp \
    monthlyreportgenerator.cpp \
    inventorymanager.cpp \
    receipteditor.cpp \
    bookreceipts.cpp \
    librarymanager.cpp \
    drawercloser.cpp \
    rangereportgenerator.cpp \
    closingseditor.cpp \
    reportviewer.cpp \
    lookupmanager.cpp

HEADERS  += mainwindow.h \
    socketclient.h \
    globals.h \
    structures.h \
    salewindow.h \
    messagewindow.h \
    salesprinterengine.h \
    transactionviewer.h \
    receiptviewer.h \
    userverification.h \
    reportmanager.h \
    monthlyreportgenerator.h \
    inventorymanager.h \
    receipteditor.h \
    bookreceipts.h \
    librarymanager.h \
    drawercloser.h \
    rangereportgenerator.h \
    closingseditor.h \
    reportviewer.h \
    lookupmanager.h

FORMS    += mainwindow.ui \
    salewindow.ui \
    messagewindow.ui \
    transactionviewer.ui \
    receiptviewer.ui \
    userverification.ui \
    reportmanager.ui \
    monthlyreportgenerator.ui \
    inventorymanager.ui \
    receipteditor.ui \
    bookreceipts.ui \
    librarymanager.ui \
    drawercloser.ui \
    rangereportgenerator.ui \
    closingseditor.ui \
    reportviewer.ui \
    lookupmanager.ui
