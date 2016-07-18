#include "mainwindow.h"
#include "ui_mainwindow.h"

//---dialog---
void MainWindow::loadDialogToListItem(Message const &&dialog, QListWidgetItem *listItem)
{
    QString body;

    if (dialog.read_state == false)
        body.append("[*]");

    if (dialog.isMultiDialog)
        body.append(dialog.title)
            .append('\n');

    if (dialog.user_id != vkLogic->getOwnProfileID())
        body.append(vkLogic->storage->getFullName(dialog.user_id))
                .append('\n');

    if (dialog.out)
        body.append(tr("You: "));

    body.append(dialog.body);

    int profileID;
    if (dialog.isMultiDialog)
        profileID = dialog.chat_id;
    else
        profileID = dialog.user_id;

    qDebug() << "Loading dialog with" << vkLogic->storage->getFullName(profileID);

    listItem->setData(Qt::DecorationRole,
                      vkLogic->getAvatar(profileID));
    listItem->setData(Qt::DisplayRole, body);
}

void MainWindow::updateProfileInDialogs(int profileID)
{
    qDebug() << "Updating info of" << vkLogic->storage->getFullName(profileID)
             << "in dialogs";
    int rowsNumber = ui->dialogsWidget->count();
    for (int currentRow = 0; currentRow < rowsNumber; ++currentRow)
    {
        QListWidgetItem *currentListItem =  ui->dialogsWidget->item(currentRow);
        Message const &&dialog( vkLogic->storage->getDialog(currentRow) );

        if (dialog.user_id == profileID || dialog.chat_id == profileID)
            loadDialogToListItem(qMove(dialog), currentListItem);
    }
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
        text.insert(0, '"');
        text.append('"');

        qDebug() << "Message:"
                 << qPrintable(text);

        ui->messageTextEdit->clear();

        vkLogic->makeRequest(Request::SEND_MESSAGE, sendTo,
                             qPrintable(text));
    }
}
