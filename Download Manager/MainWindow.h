#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <Download.h>
#include "ui_MainWindow.h"

namespace Ui {
class MainWindow;
}

class Client : public QMainWindow
{
    Q_OBJECT

public:
    explicit Client(QWidget *parent = 0);
    ~Client();
    void startDownload(const QUrl &url, const QString &fileName);

private:
    Ui::MainWindow *ui;
    QList<Download*> downloadsList;

private slots:
    void addDownload();
    void stopDownload();
    void removeDownload();
    void about();
};

#endif // MAINWINDOW_H
