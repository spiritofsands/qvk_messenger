#include "networklogic.h"

#include <QTimer>

void NetworkLogic::checkAuth()
{
    if (accessToken.isEmpty()
        || ownProfileID == 0
        || expiresIn.isNull())
    {
        qDebug() << "Auth data is empty. Requesting new";
        authorize();
    }
    else
    {
        if (authIsExpired()) {
            qDebug() << "Auth data is expired";
            authorize();
        }
        else {
            qDebug() << "Auth is ok";
            authCompleted();
        }
    }
}

void NetworkLogic::authorize()
{
    authorized = false;

    mainWindow->showAuthPageAndLoadURL();
}

void NetworkLogic::authCompleted()
{
    authorized = true;

    unsigned int expiresInMSec =
            QDateTime::currentDateTime().msecsTo(expiresIn);

    qDebug() << "access token =" << accessToken;
    qDebug() << "id = " << ownProfileID;
    qDebug() << "Starting auth expired timer: " << (int)(expiresInMSec/1000);
    authExpiredTimer->start( expiresInMSec );

    mainWindow->showContentPage();
}

void NetworkLogic::parseAuthURL(QString url)
{
    static bool authOk;

    QString parameters = url.mid( url.indexOf('#')+1 );
    QStringList parametersList = parameters.split('&');

    if (parametersList.size() != 3) {
        qDebug() << "Strange answer from server:\n" << parameters;
        authOk = false;
    }
    else
    {
        //error=access_denied&error_reason=user_denied&error_description=User denied your request
        static int eqPos;
        static QString parameterName;

        authOk = true;

        eqPos = parametersList[0].indexOf('=');
        parameterName = parametersList[0].left(eqPos);
        if (parameterName == "access_token")
            accessToken = parametersList[0].mid(eqPos + 1);
        else {
            authOk = false;
            qDebug() << "First parameter is not 'access_token'";
        }

        eqPos = parametersList[1].indexOf('=');
        parameterName = parametersList[1].left(eqPos);
        if (parameterName == "expires_in")
            setExpiresIn( parametersList[1].mid(eqPos+1) );
        else {
            authOk = false;
            qDebug() << "Second parameter is not 'expires_in'";
        }

        eqPos = parametersList[2].indexOf('=');
        parameterName = parametersList[2].left(eqPos);
        if (parameterName == "user_id")
            ownProfileID = parametersList[2].mid(eqPos+1).toInt();
        else {
            authOk = false;
            qDebug() << "Third parameter is not 'user_id'";
        }
    }

    if (!authOk) {
        qDebug() << "Auth failed:"
                 << parameters;
        authorize();
    }
    else {
        qDebug() << "Earned token, id and exp time";

        authCompleted();
    }
}

void NetworkLogic::setExpiresIn(QString const &expiresInStr)
{
    bool ok;
    int expiresInSeconds = expiresInStr.toInt(&ok);
    if (!ok) {
        qDebug() << "Wrong expiresInStr = \"" << expiresInStr << "\"\n";
        expiresInSeconds = 0;
    }

    expiresIn = QDateTime::currentDateTime().addSecs( expiresInSeconds );
}

bool NetworkLogic::authIsExpired()
{
    return expiresIn < QDateTime::currentDateTime();
}
