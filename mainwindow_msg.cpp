#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogdelegate.h"

//---dialog---
void MainWindow::loadDialogToListItem(Message const &dialog,
                                      QListWidgetItem *listItem)
{

    QString name(vkLogic->storage->getFullName(dialog.user_id)),
            lastMessage(dialog.body);

    if (dialog.isMultiDialog)
        name.prepend(dialog.title + "\n");


    if (dialog.out)
        lastMessage.prepend(tr("You: "));

    int profileID;
    if (dialog.isMultiDialog)
        profileID = dialog.chat_id;
    else
        profileID = dialog.user_id;

    qDebug() << "Adding dialogitem with" << name;

    listItem->setData(Qt::DecorationRole,
                      vkLogic->getAvatar(profileID));
    listItem->setData(DialogDelegate::NAME_ROLE, name);
    listItem->setData(DialogDelegate::LAST_MESSAGE_ROLE, lastMessage);
    listItem->setData(DialogDelegate::PROFILE_ID_ROLE, profileID);
    listItem->setData(DialogDelegate::READ_STATE_ROLE, dialog.read_state);
}

void MainWindow::updateProfileInDialogs(int /*profileID*/)
{
    qDebug() << "Not implemented: updateProfileInDialogs";
}

void MainWindow::requestAndShowDialogs()
{
    ui->contentStackedWidget->setCurrentIndex(DIALOGS_PAGE);
    vkLogic->makeRequest(Request::LOAD_DIALOGS,
                         vkLogic->getOwnProfileID());
}

//---messages
void MainWindow::loadMessageToListItem(Message const &message, QListWidgetItem *listItem)
{
    QString body;

    if (message.out)
        body.append(tr("You:\n"));
    else if (message.user_id != vkLogic->getOwnProfileID())
        body.append(vkLogic->storage->getFullName(message.user_id))
                .append('\n');

    body.append(message.body).append('\n');

    body.append(QDateTime::fromTime_t(message.date).toString());

    listItem->setData(Qt::DisplayRole, body);
}

void MainWindow::displayProfileInConversation(int profileID)
{
    qDebug() << "Updating info of" << vkLogic->storage->getFullName(profileID)
             << "in conversation";

    ui->conversationUserAvatar->setPixmap(vkLogic->getAvatar(profileID));
    ui->conversationUserPageButton->setText(
                vkLogic->storage->getFullName(profileID));

    auto &&buddy = vkLogic->storage->getProfile(profileID);
    QString status = buddy.status;
    if (status.isEmpty())
        ui->conversationUserStatus->setHidden(true);
    else
    {
        ui->conversationUserStatus->setHidden(false);
        status.insert(0, "<i>");
        status.append("</i>");
        ui->conversationUserStatus->setText(status);
    }

    if (buddy.online)
        ui->conversationUserOnlineLabel->
                setText("<span style=\" vertical-align:super;\">online</span>");
    else
        ui->conversationUserOnlineLabel->
                setText("<span style=\" vertical-align:super;\">offline</span>");
}

void MainWindow::showConversationWith(QModelIndex index)
{
    int profileID = index.data(DialogDelegate::PROFILE_ID_ROLE).toInt();
    qDebug() << "Attempting to show conversation with"
             << vkLogic->storage->getFullName(profileID);

    ui->contentStackedWidget->setCurrentIndex(CONVERSATION_PAGE);
    if (displayingConversationWithUser == profileID) {
        //showing same conversation

        qDebug() << "Nothing to do: already loaded";
    } else {
        //showing new conversation

        qDebug() << "Loading";
        displayingConversationWithUser = profileID;
        ui->conversationWidget->clear();
        displayProfileInConversation(profileID);

        QList<Message> const &messages =
                vkLogic->storage->getMessagesList(profileID);
        if (!messages.isEmpty())
            for (int i = 0; i < messages.size(); ++i) {
                QListWidgetItem *messageItem = new QListWidgetItem(ui->conversationWidget);
                loadMessageToListItem(messages[i], messageItem);
            }
    }

    vkLogic->makeRequest(Request::LOAD_CONVERSATION, profileID);
}


void MainWindow::sendMessage()
{
    int sendTo = displayingConversationWithUser;
    qDebug() << "Sending message to"
             << vkLogic->storage->getFullName(sendTo);

    QString text = ui->messageTextEdit->toPlainText();

    if (!text.isEmpty()) {
        text.prepend('"');
        text.append('"');

        qDebug() << "Message:"
                 << qPrintable(text);

        ui->messageTextEdit->clear();

        vkLogic->makeRequest(Request::SEND_MESSAGE, sendTo,
                             qPrintable(text));
    }
}
