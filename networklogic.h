#ifndef QVK_LOGIC_H
#define QVK_LOGIC_H

#include <QString>
#include <QTime>
#include <QNetworkAccessManager>
#include <QQueue>

#include "mainwindow.h"
#include "sharedcookiejar.h"
#include "storage.h"
#include "request.h"
#include "message.h"

class MainWindow;
class Storage;
class Message;

class NetworkLogic : public QObject
{
Q_OBJECT

friend class SettingsManager;

public:
    NetworkLogic(MainWindow *mainWindow);
    ~NetworkLogic();

    bool isAuthorized() { return authorized; }

    QNetworkAccessManager *getNam() { return nam; }
    int getOwnProfileID(){ return ownProfileID; }

    QPixmap getAvatar(int profileID);

    void makeRequest(Request::RequestType type,
                     int profileID = 0,
                     QString URL = QString());
    void updateDialogList();
    void parseAuthURL(QString url);
    void checkAuth();

    Storage *storage;

private slots:
    void authorize();
    void proceedRequestsQueue();
    void serverReplyHandler(QNetworkReply *reply, Request const &request);

private:
    QTimer *authExpiredTimer;
    QTimer *requestTimer;

    MainWindow *mainWindow;
    SharedCookieJar *cookieJar;
    QNetworkAccessManager *nam;

    const QString apiVersion;

    const unsigned int requestsPerSecond;
    unsigned int numberOfActiveRequests;

    bool authorized = false;
    QString accessToken;
    QDateTime expiresIn;
    int ownProfileID = 0;
             //methodName parameters
    QQueue<Request> requestsQueue;

    const unsigned int dialogsCount = 50;
    int dialogsOffset = 0;
    const unsigned int dialogPreviewLength = 10;

    const unsigned int messagesCount = 50;
    int messagesOffset = 0;

    const int maxRandID = 100000;

    struct LongPollServer{
        const bool useSsl = true;
        const bool needPts = false;

        QString key;
        QString server;
        long ts;
    } longpoll;

    void authCompleted();
    void setExpiresIn(QString const &);
    bool authIsExpired();

    void proceedOneRequest(Request const &request);
    void parseJsonReply(QJsonDocument  const &jsonReply, Request const &request);
    void setEmptyAvatar(int profileID);
    void requestOwnInfo();
    void enqueueRequest(Request const &request);
    Message createMessageFromJsonObject(QJsonObject const &messageJson);
    QJsonObject prepareDialogJsonObject(QJsonValue const &messageJsonValue);
};

#endif // QVK_LOGIC_H
