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
    ui->windowStackedWidget->setContentsMargins(0, 0, 0, 0);
    ui->contentStackedWidget->setContentsMargins(0, 0, 0, 0);

    ui->userInfoTextBrowser->viewport()->setAutoFillBackground(false);

    ui->webView->page()->setNetworkAccessManager( vkLogic->getNam() );

    //---dialogs---
    connect(ui->dialogsWidget, SIGNAL(clicked(QModelIndex)),
            this, SLOT(showMessagesWith(QModelIndex)));

    //---webview---
    connect(ui->webView, SIGNAL(urlChanged(QUrl)),
            this, SLOT(authURLUpdated()));

    ui->webView->page()->
            setLinkDelegationPolicy(QWebPage::DelegateAllLinks);
    ui->webView->setContextMenuPolicy(Qt::NoContextMenu);

    connect(ui->webView, &QWebView::linkClicked,
            this, [this](QUrl const &url){ ui->webView->load(url); });

    connect(ui->reloadButton, SIGNAL(clicked(bool)),
            this, SLOT(showAuthPageAndLoadURL()));

    //---buttons---

    connect(ui->backToDialogsButton, &QPushButton::clicked,this,
            [this]{ ui->contentStackedWidget->setCurrentIndex(DIALOGS_PAGE); });

    connect(ui->userPageButton, &QPushButton::clicked, this,
        [this]{ showProfileInfo(vkLogic->getOwnProfileID()); });

    connect(ui->conversationUserPageButton, &QPushButton::clicked, this,
        [this]{ showProfileInfo(displayingConversationWithUser); });


    connect(ui->sendMessageButton, SIGNAL(clicked(bool)),
            this, SLOT(sendMessage()));

    settingsManager->readSettings();
    vkLogic->checkAuth();

    ui->contentStackedWidget->setCurrentIndex(DIALOGS_PAGE);

    vkLogic->makeRequest(Request::GET_LONG_POLL_SERVER);

//    displayProfilesList();

//    if (vkLogic->isAuthorized())
//        requestAndShowDialogs();
//    else
//        qDebug() << "Not authorized";
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
    ui->addressEdit->setText( ui->webView->url().toString() );
    ui->addressEdit->setCursorPosition(0);

    url = ui->webView->url().toString();
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

    QString baseURL("https://oauth.vk.com/authorize?");
    QString clientID(QString("client_id=")
                    .append(QString::number(myAppID)));
    QString scope(QString("&scope=")
                    .append(QString::number(myScope)));
    QString redirectURI(QString("&redirect_uri=")
                    .append(myRedirectURI));
    QString display(QString("&display=")
                    .append(myDisplay));
    QString apiVersion(QString("&v=")
                    .append(myApiVersion));
    QString responseType(QString("&response_type=")
                    .append(myResponseType));
    QString authURL = baseURL + clientID + scope + redirectURI
            + display + apiVersion + responseType;

    ui->webView->load(QUrl(authURL));
}

//-----------------------end of auth page

void MainWindow::setWindowPage(windowIndexes index)
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

        updateOwnInfo();
    } else {
        qDebug() << "Requesting own profile";
        vkLogic->makeRequest(Request::GET_PROFILE_INFO, myID);
    }
}

void MainWindow::updateOwnInfo()
{
    int id = vkLogic->getOwnProfileID();
    ui->userPageButton->setText(
                vkLogic->storage->getFullName(id));
    QPixmap pix(vkLogic->getAvatar(id));

    ui->ownUserpicLabel->setPixmap(
                pix);

    qDebug() << "Owner info is updated onscreen";
}

void MainWindow::showProfileInfo(int profileID)
{
    ui->contentStackedWidget->setCurrentIndex(PERSON_PAGE);
    displayingUserProfile = profileID;

    if (vkLogic->storage->containsProfile(profileID)){
        Profile const profile = vkLogic->storage->getProfile(profileID);

        ui->contentStackedWidget->setEnabled(true);

        QString userInfo;
        QString newline("<br/><br/>");

        qDebug() << "Displaying info of user"
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
                userInfo+=tr("Is in your faforite list%1").arg(newline);

            userInfo+=tr("Last seen: ");
            userInfo+=QDateTime::fromTime_t(profile.last_seen.time).toString();
            if (profile.last_seen.platform != 7)
                userInfo+=tr(" on mobile");
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

            if (profile.sex == MALE)
                userInfo+=tr("Male%1").arg(newline);
            else if (profile.sex == FEMALE)
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

        ui->userPagePicLabel->setPixmap(
                    vkLogic->getAvatar(profileID));
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

void MainWindow::handleStorageUpdate(Request::RequestType type,
                                         int profileID)
{
    qDebug() << "Handling storage update of type"
             << Request::type(type);
    //===============================================================================
    if (type == Request::GET_PROFILE_INFO
        || type == Request::GET_AVATAR)
    {
        if (ui->contentStackedWidget->currentIndex() == PERSON_PAGE
            && profileID == displayingUserProfile)
        {
            qDebug() << "Updated onscreen info of"
                     << vkLogic->storage->getFullName(profileID);

            showProfileInfo(profileID);
        }
        else if (profileID == vkLogic->getOwnProfileID()) {
            updateOwnInfo();
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

            updateProfileInDialogs(profileID);
        }
    }//==============================================================================
    else if (type == Request::LOAD_DIALOGS) {
        int numberOfDialogs = vkLogic->storage->numberOfDialogs();
        qDebug() << "Loading" << numberOfDialogs << "dialogs";
        for (int dialogID = 0; dialogID < numberOfDialogs; ++dialogID)
        {
            QListWidgetItem *listItem = new QListWidgetItem(ui->dialogsWidget);
            Message const &&dialog( vkLogic->storage->getDialog(dialogID) );

            loadDialogToListItem(qMove(dialog), listItem);
        }
    }//==============================================================================
    else if (type == Request::LOAD_CONVERSATION)
    {
        int conversationSize = vkLogic->storage->numberOfMessages(profileID);
        qDebug() << "Loading" << conversationSize << "messages with"
                 << vkLogic->storage->getFullName(profileID);

        for (int messageNumber = 0; messageNumber < conversationSize; ++messageNumber)
        {
            QListWidgetItem *listItem = new QListWidgetItem;
            Message const &&message(vkLogic->storage->
                                    getMessage(profileID, messageNumber));
            loadMessageToListItem(qMove(message), listItem);
            ui->messagesWithUserWidget->insertItem(0, listItem);
        }
        ui->messagesWithUserWidget->scrollToBottom();
    }
    else {
        qDebug() << "Unknown type of storage update";
    }
}

void MainWindow::displayProfilesList()
{
    foreach(int const &profileID, vkLogic->storage->getProfilesKays()) {
        QListWidgetItem *listItem = new QListWidgetItem(ui->friendsListView);
        Profile const &&profile = vkLogic->storage->getProfile(profileID);
        QString text = profile.first_name + ' ' + profile.last_name;
        text.append(' ');
        if (profile.online)
            text.append(tr("[online]"));
        else
            text.append(tr("[onffline]"));

        listItem->setData(Qt::DisplayRole, text);
    }
}
