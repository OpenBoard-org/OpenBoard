#ifndef UBDOCUMENTPUBLISHER_H
#define UBDOCUMENTPUBLISHER_H

#include <QtGui>
#include <QtNetwork>

#include "ui_webPublishing.h"
#include "UBAbstractPublisher.h"

#define DOCPUBLICATION_URL     "http://sankore.devxwiki.com/xwiki/bin/view/CreateResources/UniboardUpload"
#define XWIKI_ORIGIN_HEADER    "http://sankore.devxwiki.com"

typedef struct
{
    QString title;
    QString description;
} sDocumentInfos;

class UBDocumentProxy;
class UBServerXMLHttpRequest;
class UBGraphicsW3CWidgetItem;
class QWebView;

class UBProxyLoginDlg : public QDialog
{
    Q_OBJECT
public:
    UBProxyLoginDlg(QWidget* parent=0, const char* name="ProxyLoginDlg");
    ~UBProxyLoginDlg();

    QString username(){return mpUsername->text();}
    QString password(){return mpPassword->text();}

private:
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpUserLayout;
    QHBoxLayout* mpPasswordLayout;
    QDialogButtonBox* mpButtons;
    QLabel* mpUserLabel;
    QLabel* mpPasswordLabel;
    QLineEdit* mpUsername;
    QLineEdit* mpPassword;
};

class UBPublicationDlg : public QDialog
{
    Q_OBJECT
public:
    UBPublicationDlg(QWidget* parent=0, const char* name="UBPublicationDlg");
    ~UBPublicationDlg();

    QString title(){return mpTitle->text();}
    QString description(){return mpDescription->document()->toPlainText();}

private slots:
    void onTextChanged();

private:
    QVBoxLayout* mpLayout;
    QHBoxLayout* mpTitleLayout;
    QLabel* mpTitleLabel;
    QLineEdit* mpTitle;
    QLabel* mpDescLabel;
    QTextEdit* mpDescription;
    QDialogButtonBox* mpButtons;
};


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
    void onProxyAuthenticationRequired(const QNetworkProxy & proxy, QAuthenticator * authenticator);


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
    sDocumentInfos mDocInfos;

};
#endif // UBDOCUMENTPUBLISHER_H
