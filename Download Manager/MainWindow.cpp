#include "MainWindow.h"
#include "AddUrl.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setCentralWidget(ui->downloadsTable);

    connect(ui->addButton, SIGNAL(triggered(bool)), this, SLOT(addDownload()));
    connect(ui->addToolButton, SIGNAL(triggered(bool)), this, SLOT(addDownload()));
    connect(ui->aboutButton, SIGNAL(triggered(bool)), this, SLOT(about()));
    connect(ui->exitButton, SIGNAL(triggered(bool)), this, SLOT(close()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::startDownload(const QUrl &url, const QString &fileName)
{
    Download* download = new Download(ui);
    downloadsList.insert(downloadsList.length(), download);
    download->run(url, fileName);
}

void MainWindow::addDownload()
{
    AddUrl* addUrl = new AddUrl(this);
    addUrl->setModal(true);
    addUrl->exec();
}

void MainWindow::about()
{
    QMessageBox::about(this, "About", "Mishchenko Dmitry, 450503\nMinsk, BSUIR, 2016");
}
