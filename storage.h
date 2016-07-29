#ifndef STORAGE_H
#define STORAGE_H

#include <QMap>
#include <QDebug>
#include <QImage>

#include "profile.h"
#include "settingsmanager.h"
#include "message.h"

class Storage : public QObject
{
    Q_OBJECT

    friend class SettingsManager;

public:
    enum UpdateType {
        PROFILE_UPDATE,
        AVATAR_UPDATE,
        DIALOG_UPDATE,
        MESSAGE_UPDATE
    };

    void addProfile(int profileID, Profile const &profile)
    {
        profiles.insert(profileID, profile);
        emit storageUpdate(PROFILE_UPDATE, profileID);
    }

    Profile const getProfile(int profileID) const //returning copy
    {
        return profiles[profileID];
    }

    bool containsProfile(int profileID) const
    {
        return profiles.contains(profileID);
    }

    QList<int> const getProfilesKeys() const
    {
        return profiles.keys();
    }

    void setAvatar(int profileID, QImage const &avatar)
    {
        avatars[profileID] = QPixmap::fromImage(avatar);
        emit storageUpdate(AVATAR_UPDATE, profileID);
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
        emit storageUpdate(UpdateType::DIALOG_UPDATE,
                           dialogs.size());//index of last message

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
        emit storageUpdate(MESSAGE_UPDATE,
                           userID,
                           messages[userID].size());
    }

    int numberOfMessages(int userID) const
    {
        return messages[userID].size();
    }

    Message const getMessage(int userID, int messageID) const
    {
        return messages[userID][messageID];
    }

//    void deleteAllMessages(int profileID)
//    {
//        messages[profileID].clear();
//    }


signals:
    void storageUpdate(Storage::UpdateType type, int, int = 0);

private:
    QMap<int, QPixmap> avatars;
    QMap<int, Profile> profiles;
    QVector<Message> dialogs;
    QMap<int, QVector<Message>> messages;
};

#endif // STORAGE_H
