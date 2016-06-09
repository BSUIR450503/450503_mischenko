#ifndef DOWNLOAD_H
#define DOWNLOAD_H

#include <QFile>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QSslError>
#include <QUrl>
#include <QTimer>
#include <QProgressBar>
#include "ui_mainwindow.h"

class Download : public QNetworkAccessManager
{
    Q_OBJECT
public:
    explicit Download(Ui::MainWindow *ui, QObject *parent = 0);

    void run(const QUrl &url, const QString &fileName);
    void stop();
    void cancel();
    void setIndex(int);

private:
    QFile file;
    QNetworkReply *reply;
    QNetworkRequest request;
    Ui::MainWindow *ui;

    int downloadIndex;

    QString fileName;

    QTimer* speedTimer;
    int interval;
    qint64 kbPerSec;

    int totalUpdates = 0;
    qint64 totalSeconds = 0;
    const qint64 timerCoefficient = 2;

    qint64 bytesReceived;
    qint64 previousBytesReceived = 0;
    qint64 bytesTotal = 0;

    void initTableView();

    void get(const QUrl &url);
    void reconnectErrorHandlers();

    bool checkFile(const QString &fileName);
    bool isRequestValid(const QUrl &url);
    bool isSchemeValid(const QUrl &url);

private slots:
    void replyFinished(QNetworkReply*reply);
    void replyReadyRead();
    void resetConnections();
    void error(QNetworkReply::NetworkError error);
    void sslErrors(const QList<QSslError> &);
    void authenticationRequired(QNetworkReply*, QAuthenticator*);
    void updateProgress(qint64 bytesReceived, qint64 bytesTotal);
    void updateTableView();
};

#endif // DOWNLOAD_H
