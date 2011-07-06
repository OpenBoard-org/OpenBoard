#ifndef UBDOCUMENTPUBLISHER_H
#define UBDOCUMENTPUBLISHER_H

#include <QtGui>
#include <QtNetwork>

#include "ui_webPublishing.h"
#include "UBAbstractPublisher.h"

#define DOCPUBLICATION_URL     "http://sankore.devxwiki.com/xwiki/bin/view/CreateResources/UniboardUpload"
#define XWIKI_ORIGIN_HEADER    "http://sankore.devxwiki.com"

class UBDocumentProxy;
class UBServerXMLHttpRequest;
class UBGraphicsW3CWidgetItem;
class QWebView;

class UBDocumentPublisher : public UBAbstractPublisher
{
    Q_OBJECT;

public:
    explicit UBDocumentPublisher(UBDocumentProxy* sourceDocument, QObject *parent = 0);
    virtual ~UBDocumentPublisher();

    void publish();

signals:

    void loginDone();

protected:

    virtual void updateGoogleMapApiKey();
    virtual void rasterizeScenes();
    virtual void upgradeDocumentForPublishing();
    virtual void generateWidgetPropertyScript(UBGraphicsW3CWidgetItem *widgetItem, int pageNumber);

private slots:

    void onFinished(QNetworkReply* reply);
    void onLinkClicked(const QUrl& url);
    void onLoadFinished(bool result);
    void onLoginDone();


private:

    UBDocumentProxy *mSourceDocument;
    UBDocumentProxy *mPublishingDocument;
    void init();
    void sendUbw();
    QString getBase64Of(QString stringToEncode);

    QWebView* mpWebView;
    QHBoxLayout* mpLayout;
    QNetworkAccessManager* mpNetworkMgr;
    QNetworkCookieJar* mpCookieJar;
    QString mUsername;
    QString mPassword;
    QString mCrlf;
    bool bLoginCookieSet;

    void buildUbwFile();
    void login(QString username, QString password);
    QString mTmpZipFile;
    QList<QNetworkCookie> mCookies;

};
#endif // UBDOCUMENTPUBLISHER_H
