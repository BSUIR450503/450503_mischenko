#include <QByteArray>
#include <QMessageBox>
#include <QDir>
#include "Download.h"

Download::Download(Ui::MainWindow *ui, QObject *parent) :
    QNetworkAccessManager(parent)
{
    this->ui = ui;
}

void Download::resetConnections()
{
    disconnect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(downloadProgress(qint64, qint64)));
}

bool Download::checkFile(const QString &fileName)
{
    if(QFile::exists(fileName))
    {
        if(QMessageBox::question(0, tr("Download Manager"), tr("There already exists a file called %1 in " "the current directory. Overwrite?").arg(fileName), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
            return false;

        QFile::remove(fileName);
    }

    return true;
}

void Download::run(const QUrl &url, const QString &fileName)
{
    if(isRequestValid(url))
    {
        if(isSchemeValid(url))
        {
            if(!checkFile(fileName))
                return;

            this->fileName = fileName;
            init();

            QFileInfo fileInfo(fileName);

            if(QDir().exists(fileInfo.path()))
            {
                file.setFileName(fileName);

                if(!file.isOpen())
                    file.open(QIODevice::WriteOnly);

                get(url);
                resetConnections();
            }
            else
                QMessageBox::critical(0, tr("Error"),  fileInfo.path() + tr(" does not exists."), QMessageBox::Ok);
        }
        else
            QMessageBox::critical(0, tr("Error"),  tr("Protocol not supported."), QMessageBox::Ok);
    }
    else
        QMessageBox::critical(0, tr("Error"),  tr("This URL is not valid, please try again."), QMessageBox::Ok);
}

bool Download::isRequestValid(const QUrl &url)
{
    return  url.isValid();
}

bool Download::isSchemeValid(const QUrl &url)
{
    bool valid = false;
    QString scheme =  url.scheme();

    if(scheme == QString("http") || scheme == QString("https") || scheme == QString("ftp"))
        valid = true;

    return valid;
}

void Download::replyFinished(QNetworkReply *reply)
{
    if(file.isOpen())
        file.close();

    if(reply->error() == QNetworkReply::NoError)
    {
        QMessageBox msgBox;
        msgBox.setText(tr("Download successfully comleted."));
        msgBox.exec();
    }
}

void Download::replyReadyRead()
{
    QByteArray bytes = reply->readAll();

    if(!bytes.isEmpty())
    {
        qint64 val = file.write(bytes);

        if(val > 0)
            file.flush();
    }
}

void Download::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    this->bytesReceived = bytesReceived;
    this->bytesTotal = bytesTotal;

    if(!speedTimer->isActive())
    {
        speedTimer->start(interval);
    }
}

void Download::cancel()
{
    if(reply->isRunning())
        reply->abort();

    if(file.isOpen())
        file.close();

    if(file.exists())
        file.remove();
}

void Download::authenticationRequired(QNetworkReply*, QAuthenticator*)
{
    QMessageBox::critical(0, tr("Authentication"),  tr("Authentication required. This feature is not supported."), QMessageBox::Ok);
    cancel();
}

void Download::get(const QUrl &url)
{
    if(!url.isEmpty())
    {
        request.setUrl(QUrl());
        disconnect(this, SIGNAL(finished(QNetworkReply*)), 0, 0);

        request.setUrl(url);
        reply = QNetworkAccessManager::get(request);

        connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
        disconnect(reply, SIGNAL(readyRead()), this, SLOT(replyReadyRead()));
        connect(reply, SIGNAL(readyRead()), this, SLOT(replyReadyRead()));

        reconnectErrorHandlers();
    }
}

void Download::reconnectErrorHandlers()
{
    disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)),this, SLOT(error(QNetworkReply::NetworkError)));
    disconnect(reply, SIGNAL(sslErrors(const QList<QSslError>&)),this, SLOT(sslErrors(const QList<QSslError>&)));
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*,QAuthenticator*)), this, SLOT(authenticationRequired(QNetworkReply*, QAuthenticator*)));
    connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),this, SLOT(error(QNetworkReply::NetworkError)));
    connect(reply, SIGNAL(sslErrors(const QList<QSslError>&)),this, SLOT(sslErrors(const QList<QSslError>&)));
}

void Download::sslErrors(const QList<QSslError>&)
{
    QMessageBox::critical(0, tr("SSL"),  tr("SSL error occured."), QMessageBox::Ok);
}

void Download::error(QNetworkReply::NetworkError error)
{
    if(error == QNetworkReply::HostNotFoundError)
        QMessageBox::critical(0, tr("Host Not Found Error"),  tr("Please check your internet connection."), QMessageBox::Ok);
    else if(error != QNetworkReply::OperationCanceledError)
        QMessageBox::critical(0, tr("Error"),  tr("A network error occured. Please try again."), QMessageBox::Ok);
}

void Download::init()
{
    downloadIndex = ui->downloadsTable->rowCount();

    ui->downloadsTable->insertRow(downloadIndex);

    ui->downloadsTable->setItem(downloadIndex, 0,new QTableWidgetItem(this->fileName));
    ui->downloadsTable->setItem(downloadIndex, 1,new QTableWidgetItem(this->bytesTotal));
    ui->downloadsTable->setItem(downloadIndex, 2,new QTableWidgetItem(this->bytesReceived));

    speedTimer = new QTimer(this);

    interval = 1000 / timerCoefficient;
    kbPerSecVal = 1024 / timerCoefficient;

    connect(speedTimer, SIGNAL(timeout()), this, SLOT(update()));
}

void Download::update()
{
    ++totalUpdates;

    totalSeconds = totalUpdates /  timerCoefficient;

    if(previousBytesReceived != 0)
        updateControls();

    previousBytesReceived = bytesReceived;
}

void Download::updateControls()
{
    qint64  currentSpeedValue = (bytesReceived - previousBytesReceived)  / kbPerSecVal;
    qint64  averageSpeedValue = (bytesReceived / totalUpdates)  / kbPerSecVal;
    qint64 remainingBytes = bytesTotal  - bytesReceived;
    qint64 remainingSec = (remainingBytes / kbPerSecVal) / averageSpeedValue;

    ui->downloadsTable->setItem(downloadIndex,1,new QTableWidgetItem(QString::number(bytesTotal)));
    ui->downloadsTable->setItem(downloadIndex,2,new QTableWidgetItem(QString::number(bytesReceived)));
    ui->downloadsTable->setItem(downloadIndex,3,new QTableWidgetItem(QString::number((double)bytesReceived/(double)bytesTotal * 100)+"%"));
    ui->downloadsTable->setItem(downloadIndex,4,new QTableWidgetItem(QString::number(currentSpeedValue)+"kb/s"));
    ui->downloadsTable->setItem(downloadIndex,5,new QTableWidgetItem(QString::number(remainingSec)+"seconds remain"));
}
