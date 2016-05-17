#-------------------------------------------------
#
# Project created by QtCreator 2016-04-22T18:26:39
#
#-------------------------------------------------

QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = DownloadManager
TEMPLATE = app


SOURCES +=\
    MainWindow.cpp \
    Main.cpp \
    AddUrl.cpp \
    Download.cpp

HEADERS  += MainWindow.h \
    AddUrl.h \
    Download.h

FORMS    += MainWindow.ui \
    AddUrl.ui

RESOURCES += \
    Icons.qrc
