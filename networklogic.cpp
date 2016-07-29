#include "networklogic.h"

#include <QTimer>

NetworkLogic::NetworkLogic(MainWindow *new_mainWindow)
 :  storage(new Storage),
    authExpiredTimer(new QTimer(this)),
    requestTimer(new QTimer(this)),

    mainWindow(new_mainWindow),

    //cookieJar(new SharedCookieJar), //nam takes ownership
    nam(new QNetworkAccessManager)
{
    //nam->setCookieJar(cookieJar);

    connect(authExpiredTimer, SIGNAL(timeout()), this, SLOT(authorize()));
    connect(requestTimer, SIGNAL(timeout()), this, SLOT(proceedRequestsQueue()));
}

NetworkLogic::~NetworkLogic()
{
    delete storage;
}
