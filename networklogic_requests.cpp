#include "networklogic.h"

#include <QTimer>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QImageReader>

void NetworkLogic::enqueueRequest(Request const &request)
{
    requestsQueue.enqueue(request);

    requestTimer->start(1000); // one second
}

void NetworkLogic::proceedRequestsQueue()
{
    static int availableRequests;

    if (!requestsQueue.isEmpty()) {
        availableRequests = requestsPerSecond - numberOfActiveRequests;
        if (availableRequests > 0) {
            qDebug() << "Proceeding"
                     << availableRequests << "requests";
            for (int i = 0;
                 !requestsQueue.isEmpty()
                    && i < availableRequests;
                 ++i)
            {
                proceedOneRequest(requestsQueue.dequeue());
            }
        }
        else {
            qDebug() << "Waiting for server responses"
                        "(3 requests per sec is maximum)";
        }
    }
    else {
        qDebug() << "Request queue is empty. We have "
                 << numberOfActiveRequests << " active requests";
        qDebug() << "Stopping requests timer";
        requestTimer->stop();
    }


}

void NetworkLogic::proceedOneRequest( Request const &request )
{
    qDebug() << "Sending request: of type"
             << Request::type(request.type())
             << "\nWith URL: " << request.URL();

    QNetworkReply *reply = nam->get(QNetworkRequest(request.URL()));
    connect(reply, &QNetworkReply::finished,
            this, [=]{ handleServerReply(reply, request); });

    numberOfActiveRequests++;
}

void NetworkLogic::handleServerReply(QNetworkReply *reply, Request const &request)
{
    auto type = request.type();
    qDebug() << "Handling reply of type" << Request::type(type)
             << "\nURL:" << request.URL();

    auto error = reply->error();
    if (error != QNetworkReply::NoError) {
        qDebug() << "Network error: " << reply->errorString();
    }
    else
    {//===============================================================================
        //JSON answers
        if (type == Request::GET_PROFILE_INFO
                || type == Request::LOAD_DIALOGS
                || type == Request::LOAD_CONVERSATION
                || type == Request::SEND_MESSAGE
                || type == Request::GET_LONGPOLL_SERVER
                || type == Request::CONNECT_TO_LONGPOLL_SERVER)
        {
            QByteArray rawReply = reply->readAll();

            QJsonParseError e;
            QJsonDocument jsonReply(QJsonDocument::fromJson(rawReply, &e));

            if (e.error != QJsonParseError::NoError) {
                qDebug() << "Parse error:"
                         << e.errorString();
            }
            if (jsonReply.isNull()) {
                qDebug() << "jsonreply is NULL";
                qDebug() << "error: " << e.errorString();
                qDebug() << "Raw reply: " << rawReply;
            }
            else if (jsonReply.isEmpty()) {
                qDebug() << "jsonreply is empty";
                qDebug() << "Raw reply: " << rawReply;
            }
            else {
                qDebug() << "jsonreply is OK";

                parseJsonReply(jsonReply, request);
            }
        }//===========================================================================
        else if (type == Request::GET_AVATAR) {
            int profileID = request.profileID();
            QImageReader imageReader(reply);
            QImage avatar = imageReader.read();

            storage->setAvatar(profileID, avatar);
            qDebug() << "Set avatar for ID = "
                     << storage->getFullName(profileID);

            //mainWindow->handleStorageUpdate(type, profileID);
        }//==========================================================================
        else
            qDebug() << "Unknown type of request in serverReplyHandler";
    }

    reply->deleteLater();

    qDebug() << "Finished request of type" << Request::type(type);
    numberOfActiveRequests--;
}

void NetworkLogic::parseJsonReply(QJsonDocument const &jsonReply, Request const &request)
{
    qDebug() << "Parsing JSON:" << jsonReply;

    //assuming jsonReply is not empty and it is an object
    QJsonObject jsonObject(jsonReply.object());

    //error?

    auto type = request.type();
    if (type == Request::GET_PROFILE_INFO)
    {
        //failproof
        if (!jsonObject.contains("response")) {
            qDebug() << "Error: jsonObject is not containing 'response'";
            qDebug() << "Json reply: " << jsonReply;
            return;
        }
        if (!jsonObject["response"].isArray()) {
            qDebug() << "Error: jsonObject['response'] is not array";
            qDebug() << "Json reply: " << jsonReply;
            return;
        }
        if (jsonObject["response"].toArray().size() == 0) {
            qDebug() << "Error: jsonObject['response'] array is 0-sized";
            qDebug() << "Json reply: " << jsonReply;
            return;
        }
        if (!jsonObject["response"].toArray()[0].isObject()) {
            qDebug() << "Error: jsonObject['response'].toArray()[0] is not object";
            qDebug() << "Json reply: " << jsonReply;
            return;
        }

        QJsonObject response = jsonObject["response"].toArray()[0].toObject();

        int currentProfileID = response["id"].toInt();

        Profile profile;

        profile.first_name = response["first_name"].toString();
        profile.last_name = response["last_name"].toString();
        profile.deactivated = response["deactivated"].toString();
        profile.hidden = response["hidden"].toInt() == 1;
        profile.bdate = response["bdate"].toString();

        QJsonObject cityObject(response["city"].toObject() );
        profile.city.id = cityObject["id"].toInt();
        profile.city.title = cityObject["title"].toString();
        profile.connections = response["connections"].toString();
        QJsonObject contactsObject(response["contacts"].toObject() );
        profile.contacts.mobile_phone = contactsObject["mobile_phone"].toString();
        profile.contacts.home_phone= contactsObject["home_phone"].toString();
        QJsonObject countryObject(response["country"].toObject() );
        profile.country.id = countryObject["id"].toInt();
        profile.country.title = countryObject["title"].toString();
        profile.is_friend = response["is_friend"].toInt() == 1;
        profile.is_favorite = response["is_favorite"].toInt() == 1;
        QJsonObject lastSeenObject(response["last_seen"].toObject() );
        profile.last_seen.time = lastSeenObject["time"].toInt();
        profile.last_seen.platform = lastSeenObject["platform"].toInt();
        profile.photo_100 = response["photo_100"].toString();
        profile.status = response["status"].toString();
        profile.sex = response["sex"].toInt();
        profile.online = response["online"].toInt() == 1;

        storage->addProfile(currentProfileID, profile);
        qDebug() << "Added profile:" << storage->getFullName(currentProfileID);

        //mainWindow->handleStorageUpdate(type, currentProfileID);
    }
    else if (type == Request::LOAD_DIALOGS)
    {
        if (!jsonObject.contains("response")) {
            qDebug() << "Error: jsonObject is not containing 'response'";
            return;
        }
        if (!jsonObject["response"].isObject()) {
            qDebug() << "Error: jsonObject is not object";
            return;
        }

        QJsonObject response = jsonObject["response"].toObject();
        unsigned int count = response["count"].toInt();
        qDebug() << "Count of dialogs" << count;
        if (count >= dialogsCount)
            qDebug() << "(!) Need implement continuous loading";

        if (!response.contains("items")) {
            qDebug() << "Error: response is not containing 'items'";
            return;
        }
        if (!response["items"].isArray()) {
            qDebug() << "Error: response['items'] is not array";
            return;
        }

        qDebug() << "\nCreating dialogs.\n";
        int profileID;
        QJsonArray dialogsArray = response["items"].toArray();
        foreach (QJsonValue const &dialogJson, dialogsArray) {
            Message dialog = createMessageFromJsonObject(
                        prepareDialogJsonObject(dialogJson.toObject()));

            if (dialog.isMultiDialog)
                profileID = dialog.chat_id;
            else
                profileID = dialog.user_id;
            storage->addMessage(profileID, dialog);
        }

        qDebug() << "Handled dialogs response";
    }
    else if (type == Request::LOAD_CONVERSATION)
    {
        if (!jsonObject.contains("response")) {
            qDebug() << "Error: jsonObject is not containing 'response'";
            return;
        }
        if (!jsonObject["response"].isObject()) {
            qDebug() << "Error: jsonObject is not object";
            return;
        }

        QJsonObject response = jsonObject["response"].toObject();
        unsigned int count = response["count"].toInt();
        qDebug() << "Count of messages" << count;
        if (count >= messagesCount)
            qDebug() << "(!) Need implement continuous loading";

        if (!response.contains("items")) {
            qDebug() << "Error: response is not containing 'items'";
            return;
        }
        if (!response["items"].isArray()) {
            qDebug() << "Error: response['items'] is not array";
            return;
        }

        qDebug() << "Deleting old messages";
        //storage->deleteAllMessages(request.profileID());

        qDebug() << "\nCreating messages.\n";

        QJsonArray messagesArray = response["items"].toArray();
        foreach (QJsonValue const &messageJson, messagesArray) {
            Message message = createMessageFromJsonObject(messageJson.toObject());
            storage->addMessage(request.profileID(), message);
        }

        qDebug() << "Handled conversation load response";
}
    else if (type == Request::SEND_MESSAGE)
    {
        if (!jsonObject.contains("response")) {
            qDebug() << "Error: jsonObject is not containing 'response'";
            return;
        }

        int sendMessageID = jsonObject["response"].toInt();

        qDebug() << "Sent message wit id" << sendMessageID;

        //mainWindow->updateMessagesWithCurrentUser();
    }
    else if (type == Request::GET_LONGPOLL_SERVER)
    {
        if (!jsonObject.contains("response")) {
            qDebug() << "Error: jsonObject is not containing 'response'";
            return;
        }
        if (!jsonObject["response"].isObject()) {
            qDebug() << "Error: jsonObject is not object";
            return;
        }

        QJsonObject response = jsonObject["response"].toObject();

        if (response.contains("key")
                && response.contains("server")
                && response.contains("ts"))
        {
            longpoll.key = response["key"].toString();
            longpoll.server = response["server"].toString();
            longpoll.ts = response["ts"].toInt();

            qDebug() << "Longpoll data:";
            qDebug() << longpoll.key;
            qDebug() << longpoll.server;
            qDebug() << longpoll.ts;

            longpoll.connected = true;

            qDebug() << "Connecting to longpoll server";
            makeRequest(Request::CONNECT_TO_LONGPOLL_SERVER);
        }
        else
        {
            longpoll.connected = false;
            qDebug() << "Failed to connect to longpoll server";
        }
    }
    else if (type == Request::CONNECT_TO_LONGPOLL_SERVER)
    {
        if (jsonObject.contains("ts")
                && jsonObject.contains("updates")
                && jsonObject["updates"].isArray())
        {
            longpoll.ts = jsonObject["ts"].toInt();
            qDebug() << "New ts is" << longpoll.ts;

            QJsonArray updates = jsonObject["updates"].toArray();

            if (updates.size() == 0)
                qDebug() << "No updates.";

            foreach (QJsonValue const &currrentUpdate, updates) {
                //unneccessary
                if (!currrentUpdate.isArray()) {
                    qDebug() << "Error: value in updates array is not array";
                    break;
                }
                handleLongpollUpdate(currrentUpdate.toArray());
            }
        }
        else //errors occured
        {
            if (!jsonObject.contains("failed")) {
                qDebug() << "Error: jsonObject is not containing 'ts'"
                            "nor 'updates' nor 'failed'";
                qDebug() << "Get new longpoll server";
                makeRequest(Request::GET_LONGPOLL_SERVER);
                return;
            }
            else //handling 'failed'
            {
                qDebug() << "Earned 'failed' state";
                int failedState = jsonObject["failed"].toInt();

                switch (failedState) {
                case 1:
                    if (!jsonObject.contains("ts")) {
                        qDebug() << "No new ts";
                        qDebug() << "Get new longpoll server";
                        makeRequest(Request::GET_LONGPOLL_SERVER);
                        return;
                    }
                    longpoll.ts = jsonObject["ts"].toInt();
                    break;

                case 2:
                    qDebug() << "Ts key expired";
                    qDebug() << "Get new longpoll server";
                    makeRequest(Request::GET_LONGPOLL_SERVER);
                    return;
                    break;

                case 3:
                    qDebug() << "User information is lost";
                    qDebug() << "Get new longpoll server";
                    makeRequest(Request::GET_LONGPOLL_SERVER);
                    return;
                    break;
                case 4:
                    qDebug() << "Wrong version in URL";
                    break;
                default:
                    qDebug() << "Strange failed state:"
                             << failedState;
                    qDebug() << "Get new longpoll server";
                    makeRequest(Request::GET_LONGPOLL_SERVER);
                    return;
                    break;
                }
            } // handling 'failed'
        } // errors occured

        qDebug() << "Connecting to longpoll server again";
        makeRequest(Request::CONNECT_TO_LONGPOLL_SERVER);
    }
    else
    {
        qDebug() << "Unknown method name";
    }
}

QJsonObject NetworkLogic::prepareDialogJsonObject(QJsonValue const &messageJsonValue)
{
//    qDebug() << "messageJsonValue: " << messageJsonValue;

    //ignoring in_read & out_read. To implement
    if (!messageJsonValue.isObject()) {
        qDebug() << "Error: messageJsonValue is not object";
        return QJsonObject();
    }
    if (!messageJsonValue.toObject().contains("message")) {
        qDebug() << "Error: messageJsonValue.toObject() is not contains 'message'";
        return QJsonObject();
    }
    if (!messageJsonValue.toObject()["message"].isObject()) {
        qDebug() << "Error: messageJsonValue.toObject()['message'] is not object";
        return QJsonObject();
    }

    return messageJsonValue.toObject()["message"].toObject();
}

Message NetworkLogic::createMessageFromJsonObject(QJsonObject const &messageJson)
{
    Message msg;
    msg.id = messageJson["id"].toInt();
    msg.user_id = messageJson["user_id"].toInt();
    if (!storage->containsProfile(msg.user_id))
        makeRequest(Request::GET_PROFILE_INFO, msg.user_id);

    msg.from_id = messageJson["from_id"].toInt();
    msg.date = messageJson["date"].toInt();
    msg.read_state = messageJson["read_state"].toInt() == 1;
    msg.out = messageJson["out"].toInt() == 1;
    msg.title = messageJson["title"].toString();
    msg.body = messageJson["body"].toString();

    QJsonObject geoObject(messageJson["geo"].toObject() );
    if (!geoObject.isEmpty()) {
        msg.geo.type = geoObject["type"].toString();
        msg.geo.coordinates = geoObject["coordinates"].toString();
        msg.geo.place = geoObject["place"].toString();
        msg.geo.id = geoObject["id"].toInt();
        msg.geo.title = geoObject["title"].toString();
        msg.geo.latitude = geoObject["latitude"].toString();
        msg.geo.longitude = geoObject["longitude"].toString();
        msg.geo.created = geoObject["created"].toString();
        msg.geo.icon = geoObject["icon"].toString();
        msg.geo.country = geoObject["country"].toString();
        msg.geo.city = geoObject["city"].toString();
    }

    //msg.attachments

    if (messageJson.contains("fwd_messages")) {
        qDebug() << "MessageJson constains 'fwd_messages'";

        if (!messageJson["fwd_messages"].isArray()) {
            qDebug() << "Error: messageJson['fwd_messages'] is not array";
            return Message();
        }
        QJsonArray messagesArray = messageJson["fwd_messages"].toArray();
        foreach (QJsonValue const &messageJson, messagesArray) {
            Message message = createMessageFromJsonObject(messageJson.toObject());
            msg.fwd_messages.push_back(message);
        }
    }

    msg.emoji = messageJson["emoji"].toInt() == 1;
    msg.important = messageJson["important"].toInt() == 1;
    msg.deleted = messageJson["deleted"].toInt() == 1;
    msg.random_id = messageJson["random_id"].toString();

    //multiDialog
    if (messageJson.contains("chat_id")) {
        msg.isMultiDialog = true;
        msg.chat_id = messageJson["chat_id"].toInt();
        msg.chat_active = messageJson["chat_active"].toString();
        msg.push_settings = messageJson["push_settings"].toString();
        msg.users_count = messageJson["users_count"].toInt();
        msg.admin_id = messageJson["admin_id"].toInt();
        msg.action = messageJson["action"].toString();
        msg.action_mid = messageJson["action_mid"].toInt();
        msg.action_email = messageJson["action_email"].toString();
        msg.action_text = messageJson["action_text"].toString();
        msg.photo_50 = messageJson["photo_50"].toString();
        msg.photo_100 = messageJson["photo_100"].toString();
        msg.photo_200 = messageJson["photo_200"].toString();


        Profile dialogProfile;
        dialogProfile.first_name = "Dialog";
        dialogProfile.last_name = QString::number(msg.chat_id);
        dialogProfile.photo_100 = msg.photo_100;
        storage->addProfile(msg.chat_id + dialogProfileConst, dialogProfile);
    }
    else {
        msg.isMultiDialog = false;
    }

    return msg;
}

void NetworkLogic::handleLongpollUpdate(QJsonArray const &update)
{
    static int updateType;

    updateType = update[0].toInt();
    qDebug() << "Handling longpol update of type:"
             << updateType;

    switch (updateType) {
    case 1: {//Замена флагов сообщения
        int messageID = update[1].toInt();
        int flags = update[2].toInt();
        //userID ignoring
        qDebug() << "Changing flags of message"
                 << messageID << "to" << flags;
        //storage->changeMessageFlags(messageID, flags);
        break;
    }case 2: {//Установка флагов сообщения (FLAGS|=$mask)
        int messageID = update[1].toInt();
        int mask = update[2].toInt();
        //peerID ignoring
        qDebug() << "Changing mask of flags of message"
                 << messageID << "to" << mask;
        //storage->changeMessageFlagsMask(messageID, mask);
        break;
    }case 3: {// Сброс флагов сообщения (FLAGS&=~$mask)
        int messageID = update[1].toInt();
        int mask = update[2].toInt();
        //peerID ignoring
        qDebug() << "Resetting mask of message flags"
                 << messageID << "to" << mask;
        //storage->resetMessageFlags(messageID, mask);
        break;
    }case 4: {// Добавление нового сообщения
        int messageID = update[1].toInt();
        //int flags = update[2].toInt();
        int fromID = update[3].toInt();
        int timestamp = update[4].toInt();
        QString subject = update[5].toString();
        QString text = update[6].toString();
        //[$attachments] (array)

        Message message;
        message.id = messageID;
        if (fromID > dialogProfileConst) //multidialog
            message.chat_id = fromID;
        else
            message.user_id = fromID;
        message.date = timestamp;
        message.title = subject;
        message.body = text;

        qDebug() << "Adding message from"
                 << storage->getFullName(fromID)
                 << "with text" << text;
        storage->addMessage(fromID, message);
        //storage->changeMessageFlags(messageID, flags);
        break;
    }case 6: {// Прочтение всех входящих сообщений
        int peerID = update[1].toInt();
        //int localID = update[2].toInt();

        qDebug() << "Reading all incoming messages with"
                 << storage->getFullName(peerID);

        //storage->setIncomingMessagesRead(peerID, localID);
        break;
    }case 7: {// Прочтение всех исходящих сообщений
        int peerID = update[1].toInt();
        //int localID = update[2].toInt();

        qDebug() << "Reading all outgoing messages with"
                 << storage->getFullName(peerID);

        //storage->setOutgoingMessagesRead(peerID, localID);
        break;
    }case 8: {// Друг стал онлайн
        int userID = -update[1].toInt();
        int extra = update[2].toInt();

        qDebug() << "Friend" << storage->getFullName(userID)
                 << "is online on";
        if (extra >= 1 && extra <= 5) // mobile
            qDebug() << "mobile";
        else
            qDebug() << "computer";

        //storage->profileSetOnline(userID, extra);
        break;
    }case 9: {// Друг стал оффлайн
        int userID = -update[1].toInt();
        int flags = update[2].toInt();

        qDebug() << "Friend" << storage->getFullName(userID)
                 << "is";
        if (flags == 0)
            qDebug() << "offline";
        else if (flags == 1)
            qDebug() << "away";

        //storage->profileSetOffline(userID, flags);
        break;
    }case 10: // флагu фильтрации по папкам для чата/собеседника
    case 11:
    case 12:
        qDebug() << "Not implemented: directories = $mask";
        break;
    case 51: {// Один из параметров (состав, тема) беседы были изменены.
        int chatID = update[1].toInt();
        //$self (integer)

        qDebug() << "Multidialog" << chatID << "need update";
        //storage->updateMultiDialog(chatID);
        break;
    }case 61: {// Пользователь начал набирать текст в диалоге.
        int userID = update[1].toInt();
        //$flags (integer)

        qDebug() << "User" << storage->getFullName(userID)
                 << "is typing";
        //storage->setProfileTyping();
        break;
    }case 62: {// Пользователь начал набирать текст в беседе
        int userID = update[1].toInt();
        int chatID = update[2].toInt();

        qDebug() << "User" << storage->getFullName(userID)
                 << "is typing in chat" << chatID;
        //storage->setProfileTyping(chatID);
        break;
    }case 70: {// Пользователь совершил звонок
        int userID = update[1].toInt();
        //$call_id (integer)

        qDebug() << "User" << storage->getFullName(userID)
                 << "is calling";
        break;
    }case 80: {// Новый счетчик непрочитанных в левом меню
        int count = update[1].toInt();

        qDebug() << "Unread messages counter =" << count;
        //mainWindow->setUnreadCount(count);
        break;
    }case 114: {// Изменились настройки оповещений
        //int peerID = update[1].toInt();
        //int sound = update[2].toInt();
        //int disabledUntil = update[3].toInt();

        qDebug() << "Notification settings are changed";
        break;
    }default:
        qDebug() << "Unknown update type:" << updateType;
        break;
    };
}

void NetworkLogic::makeRequest(Request::RequestType type,
                               int profileID,
                               QString requestData)
{
    QString finalURL;
    static const QString methodURL("https://api.vk.com/method/");

    if (type == Request::GET_PROFILE_INFO)
    {
        static const QString methodName("users.get");
        QString parameters("user_ids=");
        parameters.append(QString::number(profileID))
                .append("&fields="
                        "bdate,"
                        "city,"
                        "connections,"
                        "country,"
                        "is_favorite,"
                        "is_friend,"
                        "last_seen,"
                        "photo_100,"
                        "status,"
                        "sex,"
                        "online")
                .append("&access_token=")
                .append(accessToken)
                .append("&v=")
                .append(apiVersion);
        finalURL = QString().append(methodURL)
                .append(methodName)
                .append('?')
                .append(parameters);
    }
    else if (type == Request::GET_AVATAR) {
        static QString avatarURL;
        if (storage->containsProfile(profileID)) {
            avatarURL = storage->getProfile(profileID).photo_100;

            if (avatarURL.isEmpty()) {
                qDebug() << "ERROR: Requesting avatar with empty URL";
                return;
            }
            else {
                finalURL = storage->getProfile(profileID).photo_100;
            }
        }
        else {
            qDebug() << "Requesting avatar for non-existent user";
            return;
        }
    }
    else if (type == Request::LOAD_DIALOGS)
    {
        static const QString methodName("messages.getDialogs");
        QString parameters("user_ids=");
        parameters.append(QString::number(profileID))
                .append("&offset=")
                .append(QString::number(dialogsOffset))
                .append("&count=")
                .append(QString::number(dialogsCount))
                .append("&preview_length=")
                .append(QString::number(dialogPreviewLength))
                .append("&access_token=")
                .append(accessToken)
                .append("&v=")
                .append(apiVersion);

        finalURL = QString().append(methodURL)
                .append(methodName)
                .append('?')
                .append(parameters);

    }
    else if (type == Request::LOAD_CONVERSATION)
    {
        static const QString methodName("messages.getHistory");
        QString parameters("user_id=");
        parameters.append(QString::number(profileID))
                .append("&offset=")
                .append(QString::number(messagesOffset))
                .append("&count=")
                .append(QString::number(messagesCount))
                .append("&rev=0") //in reverse chronological order
                .append("&access_token=")
                .append(accessToken)
                .append("&v=")
                .append(apiVersion);

        finalURL = QString().append(methodURL)
                .append(methodName)
                .append('?')
                .append(parameters);
    }
    else if (type == Request::SEND_MESSAGE)
    {
        static const QString methodName("messages.send");

        int randomID = qrand() % maxRandID;

        QString parameters("user_id=");
        parameters.append(QString::number(profileID))
                .append("&random_id=")
                .append(QString::number(randomID))
                .append("&message=")
                .append(requestData)
                .append("&access_token=")
                .append(accessToken)
                .append("&v=")
                .append(apiVersion);

        finalURL = QString().append(methodURL)
                .append(methodName)
                .append('?')
                .append(parameters);
    }
    else if (type == Request::GET_LONGPOLL_SERVER)
    {
        static const QString methodName("messages.getLongPollServer");
        QString parameters("use_ssl=");
        parameters.append(QString::number(longpoll.useSsl))
                .append("&need_pts=")
                .append(QString::number(longpoll.needPts))
                .append("&access_token=")
                .append(accessToken)
                .append("&v=")
                .append(apiVersion);

        finalURL = QString().append(methodURL)
                .append(methodName)
                .append('?')
                .append(parameters);
    }
    else if (type == Request::CONNECT_TO_LONGPOLL_SERVER)
    {
        QString protocol("https://");

        finalURL = protocol.append(longpoll.server)
                .append("?act=a_check&key=")
                .append(longpoll.key)
                .append("&ts=")
                .append(QString::number(longpoll.ts))
                .append("&wait=")
                .append(QString::number(longpoll.wait))
                .append("&mode=")
                .append(QString::number(longpoll.mode));
    }
    else
    {
        qDebug() << "Unknown type to create request"
                 << type;
        return;
    }

    enqueueRequest(Request(type, profileID, finalURL));
}

void NetworkLogic::requestOwnInfo()
{
    makeRequest(Request::GET_PROFILE_INFO, ownProfileID);
}

QPixmap NetworkLogic::getAvatar(int profileID)
{
    if (storage->hasAvatar(profileID)) {
        return storage->getAvatar(profileID);
    }
    else {
        qDebug() << "Requesting avatar for"
                 << storage->getFullName(profileID);
        makeRequest(Request::GET_AVATAR, profileID);

        return QPixmap(":/images/empty_avatar.gif");
    }
}
