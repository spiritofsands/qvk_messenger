#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QModelIndex>
#include <QListWidgetItem>

#include "networklogic.h"
#include "settingsmanager.h"
#include "storage.h"
#include "message.h"
#include "dialogdelegate.h"

class SettingsManager;
class NetworkLogic;
class Request;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class SettingsManager;

public:
    enum ContentPages
         {PERSON_PAGE,
          CONVERSATION_PAGE,
          DIALOGS_PAGE};

    enum MainPages
         {AUTH_PAGE,
          CONTENT_PAGE};

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void showContentPage();
    void updateMessagesWithCurrentUser();

protected:
    void closeEvent(QCloseEvent *event);

public slots:
    void showAuthPageAndLoadURL();
    void showMessagesWith(QModelIndex index);

private slots:
    void authURLUpdated();
    void showProfileInfo(int userID);
    void requestAndShowDialogs();
    void sendMessage();
    void handleStorageUpdate(Storage::UpdateType type,
                             int, int = 0);

private:
    Ui::MainWindow *ui;
    NetworkLogic *vkLogic;

    const QString companyName;
    const QString appName;
    const QString myRedirectURI;
    const int myAppID,
              myScope;
    const QString myDisplay,
        myApiVersion,
        myResponseType;

    SettingsManager *settingsManager;

    int displayingUserProfile = 0;
    int displayingConversationWithUser = 0;

    QVector<QListWidgetItem *> dialogItems;
    DialogDelegate dialogDelegate;


    void updateProfileInConversation(int profileID);
    void updateProfileInDialogs(int profileID);
    void loadDialogToListItem(int profileID,
                               QListWidgetItem *listItem);
    void loadMessageToListItem(Message const &&message,
                               QListWidgetItem *listItem);
    void setWindowPage(MainPages index);
    void updateOwnNameAndAvatar();
    void displayProfilesList();

    void loadTextToProfileInfoPage(int profileID);
    void loadAvatarToProfileInfoPage(int profileID);

};

#endif // MAINWINDOW_H
