#include "storage.h"

void Storage::addProfile(int profileID, Profile const &profile)
{
    profiles.insert(profileID, profile);
    emit storageUpdate(PROFILE_UPDATE, profileID);
}

Profile const Storage::getProfile(int profileID) const //returning copy
{
    return profiles[profileID];
}

bool Storage::containsProfile(int profileID) const
{
    return profiles.contains(profileID);
}

//for displaying profiles
QList<int> const Storage::getProfilesList() const
{
    return profiles.keys();
}

void Storage::setAvatar(int profileID, QImage const &avatar)
{
    avatars[profileID] = QPixmap::fromImage(avatar);
    emit storageUpdate(AVATAR_UPDATE, profileID);
}

QPixmap const Storage::getAvatar(int profileID) const
{
    return avatars[profileID];
}

bool Storage::hasAvatar(int profileID) const
{
    return avatars.contains(profileID);
}


QString Storage::getFullName(int profileID) const
{
    if (containsProfile(profileID))
        return profiles[profileID].first_name +
                 ' ' + profiles[profileID].last_name;

    qDebug() << "STORAGE: Requested name of unknown profile";
    return QString::number(profileID);
}

Message const Storage::getLastMessage(int profileID) const
{
    return messages[profileID].last();
}

void Storage::addMessage(int profileID, Message const &newMessage)
{
    //messages are sorted by time
    int index = 0;
    auto &list = messages[profileID];
    int endIndex = list.size();
    while (index < endIndex)
    {
        if (list[index].id == newMessage.id)
            if (newMessage.date == list[index].date)
            {
                qDebug() << "Same message are not added";
                return; //avoid same messages
            }

        if (newMessage.date > list[index].date)
            ++index;
        else
            break;
    }
    list.insert(index, newMessage);

    emit storageUpdate(NEW_MESSAGE_UPDATE,
                       profileID,
                       index);
}

Message const Storage::getMessage(int profileID, int messageID) const
{
    return messages[profileID][messageID];
}

QList<Message> Storage::getMessagesList(int profileID)
{
    return messages[profileID];
}
