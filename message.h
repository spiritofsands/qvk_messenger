#ifndef MESSAGE
#define MESSAGE

#include <QString>
#include <QVector>
#include <QPixmap>

struct Message
{
    int id;	//идентификатор сообщения (не возвращается для пересланных сообщений).
    int user_id;	//идентификатор пользователя, в диалоге с которым находится сообщение.
    int from_id;	//идентификатор автора сообщения.
    int date;	//дата отправки сообщения в формате unixtime.
    bool read_state;	//статус сообщения (0 — не прочитано, 1 — прочитано, не возвращается для пересланных сообщений).
    bool out;	//тип сообщения (0 — полученное, 1 — отправленное, не возвращается для пересланных сообщений).
    QString title;	//заголовок сообщения или беседы.
    QString body;	//текст сообщения.
    struct Geo{	//информация о местоположении , содержит поля:
        QString type;// — тип места;
        QString coordinates; //— координаты места;
        QString place; //— описание места (если оно добавлено), объект с полями:
        int id; //— идентификатор места (если назначено);
        QString title; //— название места (если назначено);
        QString latitude; //— географическая широта;
        QString longitude; // — географическая долгота;
        QString created; // — дата создания (если назначено);
        QString icon; // — url изображения-иконки;
        QString country; // — название страны;
        QString city; // — название города;
    } geo;
    struct Attachments {	//массив медиа-вложений (см. Описание формата медиа-вложений).

    } attachments;
    QVector<Message> fwd_messages;	//массив пересланных сообщений (если есть).
    bool emoji;	//содержатся ли в сообщении emoji-смайлы.
    bool important;	//является ли сообщение важным.
    bool deleted;	//удалено ли сообщение.
    QString random_id;	//идентификатор, используемый при отправке сообщения. Возвращается только для исходящих сообщений.

    //Дополнительные поля в сообщениях из мультидиалогов

    int chat_id;	//идентификатор беседы.
    QString chat_active;	//идентификаторы авторов последних сообщений беседы.
    QString push_settings;	//настройки уведомлений для беседы, если они есть. sound и disabled_until
    int users_count;	//количество участников беседы.
    int admin_id;	//идентификатор создателя беседы.
    QString action;	//поле передано, если это служебное сообщение, может быть chat_photo_update или chat_photo_remove, а с версии 5.14 еще и chat_create, chat_title_update, chat_invite_user, chat_kick_user
    int action_mid;	//идентификатор пользователя (если > 0) или email (если < 0), которого пригласили или исключили, для служебных сообщений с action равным chat_invite_user или chat_kick_user
    QString action_email;	//email, который пригласили или исключили, строка, для служебных сообщений с action равным chat_invite_user или chat_kick_user и отрицательным action_mid
    QString action_text;	//название беседы, строка, для служебных сообщений с action равным chat_create или chat_title_update
    QString photo_50;	//url копии фотографии беседы шириной 50px.
    QString photo_100;	//url копии фотографии беседы шириной 100px.
    QString photo_200;	//url копии фотографии беседы шириной 200px.

    bool isMultiDialog;
};

#endif // MESSAGE

