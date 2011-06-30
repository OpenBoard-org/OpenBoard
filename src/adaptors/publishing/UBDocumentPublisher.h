#ifndef UBDOCUMENTPUBLISHER_H
#define UBDOCUMENTPUBLISHER_H

#include <QtGui>
#include <QtNetwork>

#include "ui_webPublishing.h"
#include "UBAbstractPublisher.h"

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


private:

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
    bool bCookieSet;

    void buildUbwFile();
    QString mTmpZipFile;

    UBDocumentProxy *mSourceDocument;
    UBDocumentProxy *mPublishingDocument;

};
#endif // UBDOCUMENTPUBLISHER_H
