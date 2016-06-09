#include "AddDownload.h"

AddDownload::AddDownload(Client *mainWindow) :
    QDialog(0),
    ui(new Ui::AddDownload)
{
    ui->setupUi(this);
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->mainWindow = mainWindow;

    connect(ui->chooseFolderButton, SIGNAL(clicked(bool)), this, SLOT(chooseFolder()));
    connect(ui->downloadButton, SIGNAL(clicked(bool)),this, SLOT(startDownload()));
    connect(ui->cancelButton, SIGNAL(clicked(bool)), this, SLOT(close()));
}

AddDownload::~AddDownload()
{
    delete ui;
}

void AddDownload::chooseFolder()
{
    QString downloadPath = QFileDialog::getExistingDirectory
            (this, tr("Open Directory"), "/home",
             QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    ui->folderLineEdit->setText(downloadPath);
}

void AddDownload::startDownload()
{
    QMessageBox msgBox(this);
    bool allowed = true;

    if(ui->urlLineEdit->text().isEmpty()) {
        msgBox.setText(tr("Please specify an URL"));
        allowed = false;
    }

    if(ui->folderLineEdit->text().isEmpty()) {
        msgBox.setText(tr("Please specify a folder to download"));
        allowed = false;
    }

    if(allowed) {
        mainWindow->startDownload(ui->urlLineEdit->text(),
                                  ui->folderLineEdit->text() + QDir::separator() +
                                  QUrl(ui->urlLineEdit->text()).fileName());
        this->close();
    }
    else
        msgBox.exec();
}
