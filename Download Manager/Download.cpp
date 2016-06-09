#include <QByteArray>
#include <QMessageBox>
#include <QDir>
#include "Download.h"

Download::Download(Ui::MainWindow *ui, QObject *parent) :
    QNetworkAccessManager(parent)
{
    this->ui = ui;
}

void Download::run(const QUrl &url, const QString &fileName)
{
    if(isRequestValid(url)) {
        if(isSchemeValid(url)) {
            if(!checkFile(fileName))
                return;

            this->fileName = fileName;
            initTableView();

            QFileInfo fileInfo(fileName);

            if(QDir().exists(fileInfo.path())) {
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

void Download::get(const QUrl &url)
{
    if(!url.isEmpty()) {
        request.setUrl(QUrl());
        disconnect(this, SIGNAL(finished(QNetworkReply*)), 0, 0);

        request.setUrl(url);
        reply = QNetworkAccessManager::get(request);

        disconnect(reply, SIGNAL(readyRead()), this, SLOT(replyReadyRead()));
        connect(reply, SIGNAL(readyRead()), this, SLOT(replyReadyRead()));
        connect(this, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

        reconnectErrorHandlers();
    }
}

void Download::stop()
{
    if(reply->isRunning())
        reply->abort();

    if(file.isOpen())
        file.close();

    if(file.exists())
        file.remove();

    disconnect(speedTimer, SIGNAL(timeout()), this, SLOT(updateTableView()));
    speedTimer->stop();
    ui->downloadsTable->removeRow(downloadIndex);
}

void Download::cancel()
{
    if(reply->isRunning())
        reply->abort();

    if(file.isOpen())
        file.close();

    disconnect(speedTimer, SIGNAL(timeout()), this, SLOT(updateTableView()));
    speedTimer->stop();
    ui->downloadsTable->removeRow(downloadIndex);
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

bool Download::checkFile(const QString &fileName)
{
    if(QFile::exists(fileName)) {
        if(QMessageBox::question(0, tr("Download Manager"),
                                 tr("There already exists a file called %1 in "
                                    "the current directory. Overwrite?").arg(fileName),
                                 QMessageBox::Yes | QMessageBox::No,
                                 QMessageBox::No) == QMessageBox::No)
            return false;

        QFile::remove(fileName);
    }

    return true;
}

void Download::replyReadyRead()
{
    QByteArray bytes = reply->readAll();

    if(!bytes.isEmpty()) {
        qint64 val = file.write(bytes);
        if(val > 0)
            file.flush();
    }
}

void Download::replyFinished(QNetworkReply *reply)
{
    if(file.isOpen())
        file.close();

    if(reply->error() == QNetworkReply::NoError) {
        disconnect(this, SIGNAL(finished(QNetworkReply*)), 0, 0);
        speedTimer->stop();
        ui->downloadsTable->setItem(downloadIndex, 1, new QTableWidgetItem
                                    (QString::number(bytesTotal/1048576., 'f', 2) + " Mb"));
        ui->downloadsTable->setItem(downloadIndex, 2,new QTableWidgetItem
                                    (QString::number(bytesTotal/1048576., 'f', 2) + " Mb"));
        ui->downloadsTable->setItem(downloadIndex, 3, new QTableWidgetItem
                                    (QString("Finished")));
        ui->downloadsTable->setItem(downloadIndex, 4, new QTableWidgetItem
                                    (QString("")));
        ui->downloadsTable->setItem(downloadIndex, 5,new QTableWidgetItem
                                    (QString("")));
    }
}

void Download::authenticationRequired(QNetworkReply*, QAuthenticator*)
{
    QMessageBox::critical(0, tr("Authentication"),  tr("Authentication required. This feature is not supported."), QMessageBox::Ok);
    stop();
}

void Download::resetConnections()
{
    disconnect(reply, SIGNAL(downloadProgress(qint64, qint64)),
               this, SLOT(updateProgress(qint64, qint64)));
    connect(reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(updateProgress(qint64, qint64)));
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

void Download::initTableView()
{
    downloadIndex = ui->downloadsTable->rowCount();

    ui->downloadsTable->insertRow(downloadIndex);

    ui->downloadsTable->setItem(downloadIndex, 0,new QTableWidgetItem(this->fileName));
    ui->downloadsTable->setItem(downloadIndex, 1,new QTableWidgetItem(this->bytesTotal));
    ui->downloadsTable->setItem(downloadIndex, 2,new QTableWidgetItem(this->bytesReceived));

    speedTimer = new QTimer(this);

    interval = 1000 / timerCoefficient;
    kbPerSec = 1024 / timerCoefficient;

    connect(speedTimer, SIGNAL(timeout()), this, SLOT(updateTableView()));
}

void Download::updateProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    this->bytesReceived = bytesReceived;
    this->bytesTotal = bytesTotal;

    if(!speedTimer->isActive()) {
        speedTimer->start(interval);
    }
}

void Download::updateTableView()
{
    ++totalUpdates;

    totalSeconds = totalUpdates /  timerCoefficient;

    if(previousBytesReceived != 0) {
        qint64  currentSpeedValue = (bytesReceived - previousBytesReceived)  / kbPerSec;
        qint64  averageSpeedValue = (bytesReceived / totalUpdates)  / kbPerSec;
        qint64 remainingBytes = bytesTotal  - bytesReceived;
        qint64 remainingSec = (remainingBytes / kbPerSec) / averageSpeedValue;

        ui->downloadsTable->setItem(downloadIndex, 1, new QTableWidgetItem
                                    (QString::number(bytesTotal/1048576., 'f', 2) + " Mb"));
        ui->downloadsTable->setItem(downloadIndex, 2,new QTableWidgetItem
                                    (QString::number(bytesReceived/1048576., 'f', 2) + " Mb"));
        ui->downloadsTable->setItem(downloadIndex, 3,new QTableWidgetItem
                                    (QString::number((double)bytesReceived/
                                                     (double)bytesTotal * 100, 'f', 2) + "%"));
        ui->downloadsTable->setItem(downloadIndex, 4,new QTableWidgetItem
                                    (QString::number(currentSpeedValue) + " Kb/s"));
        ui->downloadsTable->setItem(downloadIndex, 5,new QTableWidgetItem
                                    (QString::number(remainingSec) + " sec"));
    }

    previousBytesReceived = bytesReceived;
}

void Download::setIndex(int index) {
    downloadIndex = index;
}
