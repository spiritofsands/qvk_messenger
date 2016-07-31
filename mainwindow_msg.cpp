#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "dialogdelegate.h"

//---dialog---
void MainWindow::loadDialogToListItem(int dialogID, QListWidgetItem *listItem)
{
    Message const &&dialog( vkLogic->storage->getDialog(dialogID) );

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

    qDebug() << "Loading dialog with" << name;

    listItem->setData(Qt::DecorationRole,
                      vkLogic->getAvatar(profileID));
    listItem->setData(DialogDelegate::NAME_ROLE, name);
    listItem->setData(DialogDelegate::LAST_MESSAGE_ROLE, lastMessage);
    listItem->setData(DialogDelegate::PROFILE_ID_ROLE, profileID);
    listItem->setData(DialogDelegate::READ_STATE_ROLE, dialog.read_state);
}

void MainWindow::updateProfileInDialogs(int profileID)
{
    //in progress
}

void MainWindow::requestAndShowDialogs()
{
    ui->contentStackedWidget->setCurrentIndex(DIALOGS_PAGE);
    vkLogic->makeRequest(Request::LOAD_DIALOGS,
                         vkLogic->getOwnProfileID());
}

//---messages
void MainWindow::loadMessageToListItem(Message const &&message, QListWidgetItem *listItem)
{
    QString body;

    if (message.out)
        body.append(tr("You:\n"));
    else
    if (message.user_id != vkLogic->getOwnProfileID())
        body.append(vkLogic->storage->getFullName(message.user_id))
                .append('\n');

    body.append(message.body).append('\n');

    body.append(QDateTime::fromTime_t(message.date).toString());

    listItem->setData(Qt::DisplayRole, body);
}

void MainWindow::updateProfileInConversation(int profileID)
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

void MainWindow::showMessagesWith(QModelIndex index)
{
    Message const &&msg = vkLogic->storage->getDialog(index.row());
    static int profileID;
    if (msg.isMultiDialog)
        profileID = msg.chat_id;
    else
        profileID = msg.user_id;

    qDebug() << "Attempting to show dialog with"
             << vkLogic->storage->getFullName(profileID);

    displayingConversationWithUser = profileID;
    ui->messagesWithUserWidget->clear();
    ui->contentStackedWidget->setCurrentIndex(CONVERSATION_PAGE);
    updateProfileInConversation(profileID);

    vkLogic->makeRequest(Request::LOAD_CONVERSATION, profileID);
}

void MainWindow::updateMessagesWithCurrentUser()
{
    qDebug() << "Updating conversation with current user";

    static int profileID;
    profileID = displayingConversationWithUser;

    ui->messagesWithUserWidget->clear();
    ui->contentStackedWidget->setCurrentIndex(CONVERSATION_PAGE);
    updateProfileInConversation(profileID);

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
