#include "AddUrl.h"

AddUrl::AddUrl(MainWindow *mainWindow) :
    QDialog(0),
    ui(new Ui::AddUrl)
{
    ui->setupUi(this);

    this->mainWindow = mainWindow;

    connect(ui->chooseFolderButton, SIGNAL(clicked(bool)), this, SLOT(chooseFolder()));
    connect(ui->downloadButton, SIGNAL(clicked(bool)),this, SLOT(startDownload()));
    connect(ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
}

AddUrl::~AddUrl()
{
    delete ui;
}

void AddUrl::chooseFolder()
{
    QString downloadPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"), "/home", QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->folderLineEdit->setText(downloadPath);
}

void AddUrl::startDownload()
{
    QMessageBox msgBox(this);
    bool allowed = true;

    if(ui->urlLineEdit->text().isEmpty())
    {
        msgBox.setText(tr("Please specify an URL"));
        allowed = false;
    }
    else if(ui->folderLineEdit->text().isEmpty())
    {
        msgBox.setText(tr("Please specify a folder to download"));
        allowed = false;
    }

    if(allowed)
    {
        mainWindow->startDownload(ui->urlLineEdit->text(), ui->folderLineEdit->text() + QDir::separator() + QUrl(ui->urlLineEdit->text()).fileName());
        this->close();
    }
    else
        msgBox.exec();
}
