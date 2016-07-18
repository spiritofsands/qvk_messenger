#ifndef SHAREDCOOKIEJAR
#define SHAREDCOOKIEJAR

#include <QNetworkCookieJar>
#include <QNetworkCookie>

class SharedCookieJar : public QNetworkCookieJar
{
public:
     QList<QNetworkCookie> getCookieList() { return allCookies(); }
     void setCookieJar(QList<QNetworkCookie> c) { setAllCookies(c); }
};

#endif // SHAREDCOOKIEJAR

