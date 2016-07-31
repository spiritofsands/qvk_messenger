#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    vkLogic(new NetworkLogic(this)),
    companyName("SpiritOfSands"),
    appName("qVK Messenger"),

    myRedirectURI("https://oauth.vk.com/blank.html"),
    myAppID(5519853),
    myScope(+4096),
    myDisplay("mobile"),
    myApiVersion("5.52"),
    myResponseType("token"),

    settingsManager( new SettingsManager(
                        companyName, appName,
                        this, vkLogic) )
{
    ui->setupUi(this);

    //---design improvements---
    ui->windowStackedWidget->setContentsMargins(0, 0, 0, 0);
    ui->contentStackedWidget->setContentsMargins(0, 0, 0, 0);
    ui->userInfoTextBrowser->viewport()->setAutoFillBackground(false);

    connect(vkLogic->storage, SIGNAL(storageUpdate(Storage::UpdateType,int,int)),
            this, SLOT(handleStorageUpdate(Storage::UpdateType,int,int)));

    //---dialogs---
    connect(ui->dialogsWidget, SIGNAL(clicked(QModelIndex)),
            this, SLOT(showMessagesWith(QModelIndex)));

    //---webview---
    connect(ui->webEngine, SIGNAL(urlChanged(QUrl)),
            this, SLOT(authURLUpdated()));

    ui->webEngine->setContextMenuPolicy(Qt::NoContextMenu);

    connect(ui->reloadButton, SIGNAL(clicked(bool)),
            this, SLOT(showAuthPageAndLoadURL()));

    //---buttons---
    connect(ui->backToDialogsButton, &QPushButton::clicked,this,
            [this]{ ui->contentStackedWidget->setCurrentIndex(DIALOGS_PAGE); });

    //temporary: need to implement actual 'back' functional
    connect(ui->backFromUserPageButton, &QPushButton::clicked, this,
            [this]{ ui->contentStackedWidget->setCurrentIndex(DIALOGS_PAGE); });

    connect(ui->userPageButton, &QPushButton::clicked, this,
        [this]{ showProfileInfo(vkLogic->getOwnProfileID()); });

    connect(ui->conversationUserPageButton, &QPushButton::clicked, this,
        [this]{ showProfileInfo(displayingConversationWithUser); });


    connect(ui->sendMessageButton, SIGNAL(clicked(bool)),
            this, SLOT(sendMessage()));

    ui->dialogsWidget->setItemDelegate(&dialogDelegate);


    settingsManager->readSettings();
    vkLogic->checkAuth();

    ui->contentStackedWidget->setCurrentIndex(DIALOGS_PAGE);

//    displayProfilesList();

    if (vkLogic->isAuthorized())
    {
        vkLogic->makeRequest(Request::GET_LONGPOLL_SERVER);

        requestAndShowDialogs();
    }
    else
        qDebug() << "Not authorized";
}

MainWindow::~MainWindow()
{
    delete ui;
    delete vkLogic;
    delete settingsManager;
}

//-----------------------auth page

void MainWindow::authURLUpdated()
{
    static QString url;
    ui->addressEdit->setText( ui->webEngine->url().toString() );
    ui->addressEdit->setCursorPosition(0);

    url = ui->webEngine->url().toString();
    if (url.startsWith(myRedirectURI))
    {
        qDebug() << "Auth URL starts with redirect URL";

        vkLogic->parseAuthURL(url);
    }
}

void MainWindow::showAuthPageAndLoadURL()
{
    setWindowPage(AUTH_PAGE);

    qDebug() << "Showing auth page";

    static const QString baseURL("https://oauth.vk.com/authorize?");
    static const QString clientID(QString("client_id=")
                    .append(QString::number(myAppID)));
    static const QString scope(QString("&scope=")
                    .append(QString::number(myScope)));
    static const QString redirectURI(QString("&redirect_uri=")
                    .append(myRedirectURI));
    static const QString display(QString("&display=")
                    .append(myDisplay));
    static const QString apiVersion(QString("&v=")
                    .append(myApiVersion));
    static const QString responseType(QString("&response_type=")
                    .append(myResponseType));
    QString authURL = baseURL + clientID + scope + redirectURI
            + display + apiVersion + responseType;

    ui->webEngine->load(QUrl(authURL));
}

//-----------------------end of auth page

void MainWindow::setWindowPage(MainPages index)
{
    ui->windowStackedWidget->setCurrentIndex(index);
}

void MainWindow::showContentPage()
{
    setWindowPage(CONTENT_PAGE);
    qDebug() << "Showing content page";

    int myID = vkLogic->getOwnProfileID();
    if (vkLogic->storage->containsProfile(myID))
    {
        qDebug() << "Loading own profile from storage";

        updateOwnNameAndAvatar();
    } else {
        qDebug() << "Requesting own profile";
        vkLogic->makeRequest(Request::GET_PROFILE_INFO, myID);
    }
}

void MainWindow::updateOwnNameAndAvatar()
{
    int id = vkLogic->getOwnProfileID();
    ui->userPageButton->setText(
                vkLogic->storage->getFullName(id));

    ui->ownUserpicLabel->setPixmap(vkLogic->getAvatar(id));

    qDebug() << "Owner info is updated onscreen";
}

void MainWindow::loadTextToProfileInfoPage(int profileID)
{
    Profile const profile = vkLogic->storage->getProfile(profileID);

    ui->contentStackedWidget->setEnabled(true);

    QString userInfo;
    QString newline("<br/><br/>");

    qDebug() << "Displaying info of profile"
        << vkLogic->storage->getFullName(profileID);

    userInfo = "<big><b>" + profile.first_name
            + " " + profile.last_name + "</b></big>";

    userInfo+=newline;
    if (!profile.deactivated.isEmpty())
        userInfo+=tr("Account is deactivated: %1%2").arg(profile.deactivated).arg(newline);
    else if (profile.hidden)
        userInfo+=tr("Account is hidden%1").arg(newline);
    else {
        if (!profile.status.isEmpty())
            userInfo+="<cite>" + profile.status + "</cite>" + newline;
        if (profile.is_friend)
            userInfo+=tr("Is your friend%1").arg(newline);
        else if (profile.is_favorite)
            userInfo+=tr("Is in your favorite list%1").arg(newline);

        if (!profile.online)
        {
            userInfo+=tr("Last seen: ");
            userInfo+=QDateTime::fromTime_t(profile.last_seen.time).toString();
            if (profile.last_seen.platform != 7)
                userInfo+=tr(" on mobile");
        }
        else
            userInfo+=tr("Online");

        userInfo+=newline;

        if (!profile.connections.isEmpty())
            userInfo+=tr("Has connections: %1%2")
                    .arg(profile.connections).arg(newline);
        if (!profile.contacts.mobile_phone.isEmpty())
            userInfo+=tr("Mobile phone: %1%2")
                    .arg(profile.contacts.mobile_phone).arg(newline);
        if (!profile.contacts.home_phone.isEmpty())
            userInfo+=tr("Home phone: %1%2")
                    .arg(profile.contacts.home_phone).arg(newline);

        if (profile.sex == 1)
            userInfo+=tr("Male%1").arg(newline);
        else if (profile.sex == 0)
            userInfo+=tr("Female%1").arg(newline);

        if (!profile.bdate.isEmpty()){
            userInfo+=tr("Birthday: ");
            userInfo+=profile.bdate;
            userInfo+=newline;
        }

        if (!profile.country.title.isEmpty())
            userInfo+=tr("Country: %1%2")
                    .arg(profile.country.title).arg(newline);

        if (!profile.city.title.isEmpty())
            userInfo+=tr("City: %1%2")
                    .arg(profile.city.title).arg(newline);
    }

    ui->userInfoTextBrowser->setHtml(userInfo);
}

void MainWindow::loadAvatarToProfileInfoPage(int profileID)
{
    ui->userPagePicLabel->setPixmap(
                vkLogic->getAvatar(profileID));
}


void MainWindow::showProfileInfo(int profileID)
{
    ui->contentStackedWidget->setCurrentIndex(PERSON_PAGE);
    displayingUserProfile = profileID;

    if (vkLogic->storage->containsProfile(profileID)){
        loadTextToProfileInfoPage(profileID);

        loadAvatarToProfileInfoPage(profileID);
    }
    else { //not in storage
        ui->contentStackedWidget->setEnabled(false);
        qDebug() << "Waiting for storage update for user" << profileID;
        vkLogic->makeRequest(Request::GET_PROFILE_INFO, profileID);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    settingsManager->writeSettings();
    event->accept();
}

void MainWindow::handleStorageUpdate(Storage::UpdateType type,
                                         int profileID,
                                         int messageID)
{
    if (type == Storage::PROFILE_UPDATE)
    {
        if (ui->contentStackedWidget->currentIndex() == PERSON_PAGE
            && profileID == displayingUserProfile)
        {
            qDebug() << "Updating person page text";

            loadTextToProfileInfoPage(profileID);
        }

        //update friend list, dialogs, current conversation

    }
    if (type == Storage::AVATAR_UPDATE)
    {
        if (ui->contentStackedWidget->currentIndex() == PERSON_PAGE
            && profileID == displayingUserProfile)
        {
            qDebug() << "Updating person page avatar";

            loadAvatarToProfileInfoPage(profileID);
        }
        else if (profileID == vkLogic->getOwnProfileID()) {
            updateOwnNameAndAvatar();
        }
        else if (ui->contentStackedWidget->currentIndex() == CONVERSATION_PAGE
                 && displayingConversationWithUser == profileID)
        {
            qDebug() << "Updating profile of"
                     << vkLogic->storage->getFullName(profileID)
                     << "on conversation page";
            updateProfileInConversation(profileID);
        }
        else// if (ui->contentStackedWidget->currentIndex() == DIALOGS_PAGE)
        {
            qDebug() << "Updating profile of"
                     << vkLogic->storage->getFullName(profileID)
                     << "on dialogs page";

            //updateProfileInDialogs(profileID);
        }
    }//==============================================================================
    else if (type == Storage::DIALOG_UPDATE) {
        qDebug() << "Updating dialog with " <<
                    vkLogic->storage->getFullName(profileID);

        QListWidgetItem *item;

        //if new item
        if (ui->dialogsWidget->count() <= profileID)
            item = new QListWidgetItem(ui->dialogsWidget);
        else //if existing item
            item = ui->dialogsWidget->item(profileID);

        loadDialogToListItem(profileID, item);
    }//==============================================================================
    else if (type == Storage::MESSAGE_UPDATE)
    {
        //in progress
    }
    else {
        qDebug() << "Unknown type of storage update";
    }
}

void MainWindow::displayProfilesList()
{
    foreach(int const &profileID, vkLogic->storage->getProfilesKeys()) {
        QListWidgetItem *listItem = new QListWidgetItem(ui->friendsListView);
        Profile const &&profile = vkLogic->storage->getProfile(profileID);
        QString text = profile.first_name + ' ' + profile.last_name;
        text.append(' ');
        if (profile.online)
            text.append(tr("[online]"));
        else
            text.append(tr("[offline]"));

        listItem->setData(Qt::DisplayRole, text);
    }
}
