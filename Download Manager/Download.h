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
    void get(const QUrl &url);
    void cancel();

private:
    QFile file;
    QNetworkReply *reply;
    QNetworkRequest request;
    Ui::MainWindow *ui;

    QString fileName;
    QTimer* speedTimer;
    int interval;
    int totalUpdates = 0;
    qint64 totalSeconds = 0;
    qint64 bytesReceived;
    qint64 previousBytesReceived = 0;
    const qint64 timerCoefficient = 2;
    double mbTotal = 0.;
    qint64 kbPerSecVal;
    qint64 bytesTotal = 0;

    void init();
    void updateControls();
    void setProgress(qint64 bytesReceived, qint64 bytesTotal);

    int downloadIndex;

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

    void downloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void update();
};

#endif // DOWNLOAD_H
