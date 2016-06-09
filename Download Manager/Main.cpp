#include "MainWindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication downloadManager(argc, argv);
    downloadManager.setWindowIcon(QIcon(":/Icons/Icon.png"));

    Client mainWindow;
    mainWindow.show();

    return downloadManager.exec();
}
