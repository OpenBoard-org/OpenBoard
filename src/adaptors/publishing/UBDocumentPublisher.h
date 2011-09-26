/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UBDOCUMENTPUBLISHER_H
#define UBDOCUMENTPUBLISHER_H

#include <QtGui>
#include <QtNetwork>

#include "ui_webPublishing.h"

#define DOCPUBLICATION_URL     "http://planete.sankore.org/xwiki/bin/view/CreateResources/UniboardUpload?xpage=plain&outputSyntax=plain"

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


class UBDocumentPublisher : public QObject
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

private:

    UBDocumentProxy *mSourceDocument;
    UBDocumentProxy *mPublishingDocument;
    void init();
    void sendUbw(QString username, QString password);
    QString getBase64Of(QString stringToEncode);

    QHBoxLayout* mpLayout;
    QNetworkAccessManager* mpNetworkMgr;
    QNetworkCookieJar* mpCookieJar;
    QString mUsername;
    QString mPassword;
    QString mCrlf;
    bool bLoginCookieSet;

    void buildUbwFile();
    QString mTmpZipFile;
    QList<QNetworkCookie> mCookies;
    sDocumentInfos mDocInfos;

};
#endif // UBDOCUMENTPUBLISHER_H
