#include "MainWindow.h"
#include "AddDownload.h"

Client::Client(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->downloadsTable);

    connect(ui->addButton, SIGNAL(triggered(bool)), this, SLOT(addDownload()));
    connect(ui->addToolButton, SIGNAL(triggered(bool)), this, SLOT(addDownload()));
    connect(ui->stopToolButton, SIGNAL(triggered(bool)), this, SLOT(stopDownload()));
    connect(ui->removeToolButton, SIGNAL(triggered(bool)), this, SLOT(removeDownload()));
    connect(ui->aboutButton, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(ui->exitButton, SIGNAL(triggered(bool)), this, SLOT(close()));
}

Client::~Client()
{
    delete ui;
}

void Client::startDownload(const QUrl &url, const QString &fileName)
{
    Download* download = new Download(ui);
    downloadsList.insert(downloadsList.length(), download);
    download->run(url, fileName);
}

void Client::addDownload()
{
    AddDownload* addUrl = new AddDownload(this);
    addUrl->setModal(true);
    addUrl->exec();
}

void Client::stopDownload()
{    
    if (ui->downloadsTable->rowCount() == 0)
        return;

    if(QMessageBox::warning(0, tr("Download Manager"),
                                     tr("It will remove download from list, but file still exist."
                                        " Are you sure?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No) == QMessageBox::No)
        return;

    int row = ui->downloadsTable->currentRow();
    Download* download = downloadsList.at(row);

    if (download == NULL)
        return;

    download->cancel();
    downloadsList.removeAt(row);

    int index = 0;
    foreach (Download* download, downloadsList) {
        download->setIndex(index);
        index++;
    }
}

void Client::removeDownload()
{
    if (ui->downloadsTable->rowCount() == 0)
        return;

    if(QMessageBox::warning(0, tr("Download Manager"),
                                     tr("It will remove download with file. Are you sure?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No) == QMessageBox::No)
        return;

    int row = ui->downloadsTable->currentRow();
    Download* download = downloadsList.at(row);

    if (download == NULL)
        return;

    download->stop();
    downloadsList.removeAt(row);

    int index = 0;
    foreach (Download* download, downloadsList) {
        download->setIndex(index);
        index++;
    }
}

void Client::about()
{
    QMessageBox::about(this, "About", "Mishchenko Dmitry, 450503\nMinsk, BSUIR, 2016");
}
