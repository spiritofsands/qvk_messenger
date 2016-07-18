#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QString>
#include <QStandardPaths>

class MainWindow;
class NetworkLogic;

class SettingsManager
{
public:
    SettingsManager(const QString &companyName,
                    const QString &appName,
                    MainWindow *mainWin, NetworkLogic *vkLogic);

    void readSettings();
    void writeSettings();

private:
    const QString appDataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    const QString companyName;
    const QString appName;
    MainWindow *mainWin;
    NetworkLogic *vkLogic;
};

#endif // SETTINGSMANAGER_H
