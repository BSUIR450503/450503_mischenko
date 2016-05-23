#ifndef ADDURL_H
#define ADDURL_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <Download.h>
#include <MainWindow.h>
#include "ui_AddUrl.h"

namespace Ui {
class AddUrl;
}

class AddUrl : public QDialog
{
    Q_OBJECT
public:
    explicit AddUrl(MainWindow *mainWindow);
    ~AddUrl();

private:
    Ui::AddUrl *ui;
    MainWindow *mainWindow;

private slots:
    void startDownload();
    void chooseFolder();
};

#endif // ADDURL_H
