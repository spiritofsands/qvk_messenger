#include "settingsmanager.h"
#include "ui_mainwindow.h"

#include <mainwindow.h>

#include <QSettings>
#include <QDir>

QDataStream& operator<<(QDataStream& out, const Profile& profile) {
    out << profile.first_name
        << profile.last_name
        << profile.deactivated
        << profile.hidden
        << profile.bdate
        << profile.city.id
        << profile.city.title
        << profile.connections
        << profile.contacts.mobile_phone
        << profile.contacts.home_phone
        << profile.country.id
        << profile.country.title
        << profile.is_favorite
        << profile.is_friend
        << profile.last_seen.time
        << profile.last_seen.platform
        << profile.photo_100
        << profile.status
        << profile.sex;

    return out;
}

QDataStream& operator>>(QDataStream& in, Profile& profile) {
    in  >> profile.first_name
        >> profile.last_name
        >> profile.deactivated
        >> profile.hidden
        >> profile.bdate
        >> profile.city.id
        >> profile.city.title
        >> profile.connections
        >> profile.contacts.mobile_phone
        >> profile.contacts.home_phone
        >> profile.country.id
        >> profile.country.title
        >> profile.is_favorite
        >> profile.is_friend
        >> profile.last_seen.time
        >> profile.last_seen.platform
        >> profile.photo_100
        >> profile.status
        >> profile.sex;
    return in;
}

SettingsManager::SettingsManager(
        const QString &inp_companyName,
        const QString &inp_appName,
        MainWindow *inp_MainWin,
        NetworkLogic *inp_VkLogic)

 : companyName(inp_companyName),
   appName(inp_appName),
   mainWin(inp_MainWin),
   vkLogic(inp_VkLogic)
{
    qRegisterMetaTypeStreamOperators<Profile>("Profile");
}

void SettingsManager::writeSettings()
{
    QSettings settings(companyName, appName);

    settings.beginGroup("MainWindow");
    settings.setValue("size", mainWin->size());
    settings.setValue("pos", mainWin->pos());
    settings.setValue("UserPageButton",
                      mainWin->ui->userPageButton->text());
    settings.endGroup();

    settings.beginGroup("Cookies");
    QByteArray rawCookies;

    foreach( QNetworkCookie currentCookie,
             vkLogic->cookieJar->getCookieList())
        //save only crossession cookies
        if ( !currentCookie.isSessionCookie() )
            rawCookies.append( currentCookie.toRawForm() ).append( "\n" );

    settings.setValue("cookies", rawCookies);
    settings.endGroup();

    if ( vkLogic->isAuthorized() ) {
        settings.beginGroup("OwnData");
        settings.setValue("accessToken", vkLogic->accessToken);
        settings.setValue("profileID", vkLogic->ownProfileID);
        settings.setValue("expiresIn",
                          vkLogic->expiresIn);

        settings.endGroup();

        bool pathIsOk = true;
        if (!QDir(appDataPath).exists())
            pathIsOk = QDir().mkpath(appDataPath);

        if (pathIsOk) {
            //saving profiles
            QFile profilesFile(appDataPath + "/profiles");
            if (profilesFile.open(QIODevice::WriteOnly))
            {
                QDataStream out(&profilesFile);
                out << vkLogic->storage->profiles.size();
                foreach (int const &id, vkLogic->storage->profiles.keys()){
                    out << id << vkLogic->storage->profiles[id];

//                    qDebug() << "Wrote profile of"
//                             << vkLogic->storage->getFullName(id);
                }

                qDebug() << "Saved" << vkLogic->storage->profiles.size()
                         << "profiles";
            }
            else
            {
                qDebug() << "Can't save users to" << profilesFile.fileName()
                         << "due to" << profilesFile.errorString();
            }

            //saving avatars
            QFile avatarsFile(appDataPath + "/avatars");
            if (avatarsFile.open(QIODevice::WriteOnly))
            {
                QDataStream out(&avatarsFile);
                out << vkLogic->storage->avatars.size();
                foreach (int const &id, vkLogic->storage->avatars.keys()){
                    out << id << vkLogic->storage->avatars[id];
                }
                qDebug() << "Saved" << vkLogic->storage->avatars.size() << "avatars";
            }
            else
            {
                qDebug() << "Can't save avatars to" << avatarsFile.fileName()
                         << "due to" << avatarsFile.errorString();
            }
        }
        else //path is not ok
        {
            qDebug() << "Can't create path for settings:"
                     << appDataPath;
        }

        qDebug() << "Saved auth data";
    }
    else
        qDebug() << "Not auhtorized, so nothing to save";

}

void SettingsManager::readSettings()
{
    QSettings settings(companyName, appName);

    settings.beginGroup("MainWindow");
    mainWin->resize(settings.value("size").toSize());
    mainWin->move(settings.value("pos", QPoint(200, 200)).toPoint());
    settings.endGroup();

    settings.beginGroup("Cookies");
    QByteArray rawCookies = settings.value("cookies").toByteArray();
    vkLogic->cookieJar->setCookieJar( QNetworkCookie::parseCookies( rawCookies ) );
    settings.endGroup();

    settings.beginGroup("OwnData");
    QString accessToken = settings.value("accessToken").toString();
    int ownProfileID = settings.value("profileID").toInt();
    QDateTime expiresIn = settings.value("expiresIn").toDateTime();
    vkLogic->accessToken = accessToken;
    vkLogic->ownProfileID = ownProfileID;
    vkLogic->expiresIn = expiresIn;
    settings.endGroup();

    //loading profiles
    QFile profilesFile(appDataPath + "/profiles");
    if (profilesFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Loading profiles:";

        QDataStream in(&profilesFile);
        int size, id;
        Profile profile;
        in >> size;
        for (int counter = 1; counter <= size; ++counter) {
            in >> id >> profile;
            vkLogic->storage->profiles.insert(id, profile);
            //qDebug() << vkLogic->storage->getFullName(id);
        }
        qDebug() << "Total: " << size << "profiles";
    }
    else
    {
        qDebug() << "Can't read profiles from" << profilesFile.fileName()
                 << "due to" << profilesFile.errorString();
    }

    //loading avatars
    QFile avatarsFile(appDataPath + "/avatars");
    if (avatarsFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Loading avatars:";

        QDataStream in(&avatarsFile);
        int size, id;
        in >> size;
        QPixmap pix;
        for (int counter = 0; counter < size; ++counter) {
            in >> id >> pix;
            vkLogic->storage->avatars.insert(id, pix);
            //qDebug() << vkLogic->storage->getFullName(id);
        }
        qDebug() << "Total:" << vkLogic->storage->avatars.size()
                 << "avatars";
    }
    else
    {
        qDebug() << "Can't read avatars from" << avatarsFile.fileName()
                 << "duel to" << avatarsFile.errorString();
    }

    qDebug() << "Loaded settings.";
}
