#ifndef STORAGE_H
#define STORAGE_H

#include <QMap>
#include <QDebug>
#include <QImage>

#include "profile.h"
#include "settingsmanager.h"
#include "message.h"

class Storage
{

friend class SettingsManager;

public:
    void addProfile(int profileID, Profile const &profile)
    {
        profiles.insert(profileID, profile);
    }

    Profile const getProfile(int profileID) const //returning copy
    {
        return profiles[profileID];
    }

    bool containsProfile(int profileID) const
    {
        return profiles.contains(profileID);
    }

    QList<int> const getProfilesKays() const
    {
        return profiles.keys();
    }

    void setAvatar(int profileID, QImage const &avatar)
    {
        avatars[profileID] = QPixmap::fromImage(avatar);
    }

    QPixmap const getAvatar(int profileID) const
    {
        return avatars[profileID];
    }

    bool hasAvatar(int profileID) const
    {
        return avatars.contains(profileID);
    }


    QString getFullName(int profileID) const
    {
        QString name;
        if (containsProfile(profileID))
            name = profiles[profileID].first_name +
                     ' ' + profiles[profileID].last_name;
        else {
            qDebug() << "STORAGE: Requested name of unknown profile";
            name = QString::number(profileID);
        }

        return name;
    }


    void addDialog(Message const &m)
    {
        dialogs.push_back(m);
    }

    int numberOfDialogs() const
    {
        return dialogs.size();
    }

    Message const getDialog(int messageID) const
    {
        return dialogs[messageID];
    }


    void addMessage(int userID, Message const &m)
    {
        messages[userID].push_back(m);
    }

    int numberOfMessages(int userID) const
    {
        return messages[userID].size();
    }

    Message const getMessage(int userID, int messageID) const
    {
        return messages[userID][messageID];
    }

    void deleteAllMessages(int profileID)
    {
        messages[profileID].clear();
    }

private:
    QMap<int, QPixmap> avatars;
    QMap<int, Profile> profiles;
    QVector<Message> dialogs;
    QMap<int, QVector<Message>> messages;
};

#endif // STORAGE_H
