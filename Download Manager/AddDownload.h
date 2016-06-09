#ifndef ADDDOWNLOAD_H
#define ADDDOWNLOAD_H

#include <QDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <Download.h>
#include <MainWindow.h>
#include "ui_AddDownload.h"

namespace Ui {
class AddDownload;
}

class AddDownload : public QDialog
{
    Q_OBJECT
public:
    explicit AddDownload(Client *mainWindow);
    ~AddDownload();

private:
    Ui::AddDownload *ui;
    Client *mainWindow;

private slots:
    void startDownload();
    void chooseFolder();
};

#endif // ADDDOWNLOAD_H
