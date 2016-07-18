#ifndef REQUEST_H
#define REQUEST_H

#include <QString>

class Request
{
public:
    enum RequestType{
        GET_PROFILE_INFO,
        GET_AVATAR,
        LOAD_DIALOGS,
        LOAD_CONVERSATION,
        SEND_MESSAGE
    };

    Request(RequestType new_type,
            int new_profileID,
            QString const &new_URL)
     : typeOfRequest(new_type),
       profileIDOfRequest(new_profileID),
       URLOfRequest(new_URL)
    {}

    RequestType const &type() const
    { return typeOfRequest; }

    int profileID() const
    { return profileIDOfRequest; }

    QString const &URL() const
    { return URLOfRequest; }

    static QString type(RequestType const &type)
    {
        if (type == GET_PROFILE_INFO)
            return "GET_PROFILE_INFO";
        else if (type == GET_AVATAR)
            return "GET_AVATAR";
        else if (type == LOAD_DIALOGS)
            return "LOAD_DIALOGS";
        else if (type == LOAD_CONVERSATION)
            return "LOAD_CONVERSATION";
        else if (type == SEND_MESSAGE)
            return "SEND_MESSAGE";
        else
            return "UNKNOWN_REQUEST_TYPE";
    }

private:
    RequestType typeOfRequest;
    int profileIDOfRequest;
    QString URLOfRequest;
};

#endif // REQUEST_H
