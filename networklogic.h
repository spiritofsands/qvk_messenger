#ifndef QVK_LOGIC_H
#define QVK_LOGIC_H

#include <QString>
#include <QTime>
#include <QNetworkReply>
//#include <QNetworkAccessManager>
#include <QQueue>

#include "mainwindow.h"
//#include "sharedcookiejar.h"
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
    QString accessToken;
    QDateTime expiresIn;
    int ownProfileID = 0;
    bool authorized = false;

    const QString apiVersion = "5.52";

    QTimer *requestTimer;
    const unsigned int requestsPerSecond = 3;
    unsigned int numberOfActiveRequests = 0;

    struct LongpollServer{
        const bool useSsl = true;
        const bool needPts = false;
        const int wait = 25; //seconds
        const int mode = 64;

        bool connected = false;

        QString key;
        QString server;
        int ts;
    } longpoll;

    MainWindow *mainWindow;
    QNetworkAccessManager *nam;

    QQueue<Request> requestsQueue;

    const unsigned int dialogsCount = 50;
    int dialogsOffset = 0;
    const unsigned int dialogPreviewLength = 20;

    const unsigned int messagesCount = 50;
    int messagesOffset = 0;

    const int maxRandID = 100000;
    const int dialogProfileConst = 2000000000;

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

    void handleLongpollUpdate(QJsonArray const &update);
};

#endif // QVK_LOGIC_H
