#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QModelIndex>
#include <QListWidgetItem>

#include "networklogic.h"
#include "settingsmanager.h"
#include "request.h"
#include "message.h"

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
    enum contentIndexes
         {PERSON_PAGE,
          CONVERSATION_PAGE,
          DIALOGS_PAGE};

    enum windowIndexes
         {AUTH_PAGE,
          CONTENT_PAGE};

    enum Sex
        {NOT_LISTED_SEX,
        FEMALE,
        MALE};

    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void handleStorageUpdate(Request::RequestType type, int userID = 0);
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

    void updateProfileInConversation(int profileID);
    void updateProfileInDialogs(int profileID);
    void loadDialogToListItem(Message const &&message,
                               QListWidgetItem *listItem);
    void loadMessageToListItem(Message const &&message,
                               QListWidgetItem *listItem);
    void setWindowPage(windowIndexes index);
    void updateOwnInfo();
    void displayProfilesList();

};

#endif // MAINWINDOW_H
