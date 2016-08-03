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
        NEW_MESSAGE_UPDATE
    };

    void addProfile(int profileID, Profile const &profile);
    Profile const getProfile(int profileID) const; //returning copy
    bool containsProfile(int profileID) const;
    QString getFullName(int profileID) const;
    QList<int> const getProfilesList() const;//for displaying profiles

    void setAvatar(int profileID, QImage const &avatar);
    QPixmap const getAvatar(int profileID) const;
    bool hasAvatar(int profileID) const;

    Message const getLastMessage(int profileID) const;
    void addMessage(int profileID, Message const &newMessage);
    Message const getMessage(int profileID, int messageID) const;
    QList<Message> getMessagesList(int profileID);

signals:
    void storageUpdate(Storage::UpdateType type, int, int = 0);

private:
    QMap<int, QPixmap> avatars;
    QMap<int, Profile> profiles;
    QMap<int, QList<Message>> messages;
};

#endif // STORAGE_H
