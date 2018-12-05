/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBNetworkAccessManager.h"

#include <QtGui>
#include <QtNetwork>
#include <QMessageBox>
#include <QPushButton>

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"

#include "ui_passworddialog.h"
#include "ui_proxy.h"

#include "UBCookieJar.h"


#include "core/memcheck.h"

UBNetworkAccessManager *UBNetworkAccessManager::sNetworkAccessManager = 0;

UBNetworkAccessManager *UBNetworkAccessManager::defaultAccessManager()
{
    if (!sNetworkAccessManager) {
        sNetworkAccessManager = new UBNetworkAccessManager(qApp);
        sNetworkAccessManager->setCookieJar(new UBCookieJar(sNetworkAccessManager));
    }
    return sNetworkAccessManager;
}

UBNetworkAccessManager::UBNetworkAccessManager(QObject *parent)
    : QNetworkAccessManager(parent)
      , mProxyAuthenticationCount(0)
{
    connect(this, SIGNAL(authenticationRequired(QNetworkReply*, QAuthenticator*)),
            SLOT(authenticationRequired(QNetworkReply*,QAuthenticator*)));
    connect(this, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)),
            SLOT(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));
    connect(this, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError>&)),
            SLOT(sslErrors(QNetworkReply*, const QList<QSslError>&)));

    QNetworkProxy* proxy = UBSettings::settings()->httpProxy();

    if (proxy)
    {
        setProxy(*proxy);
    }
    else
    {
        QNetworkProxyFactory::setUseSystemConfiguration(true);
    }

    QNetworkDiskCache *diskCache = new QNetworkDiskCache(this);
    QString location = UBSettings::userDataDirectory() + "/web-cache";
    diskCache->setCacheDirectory(location);
    setCache(diskCache);
}

QNetworkReply* UBNetworkAccessManager::createRequest(Operation op, const QNetworkRequest & req, QIODevice * outgoingData)
{
    QNetworkRequest request = req; // copy so we can modify
    // this is a temporary hack until we properly use the pipelining flags from QtWebkit
    // pipeline everything! :)
    request.setAttribute(QNetworkRequest::HttpPipeliningAllowedAttribute, true);

    QNetworkReply* reply = QNetworkAccessManager::createRequest(op, request, outgoingData);

    return reply;
}

QNetworkReply *UBNetworkAccessManager::get(const QNetworkRequest &request)
{
    QTime loadStartTime;
    loadStartTime.start();
    QNetworkReply *networkReply = QNetworkAccessManager::get(request);
    return networkReply;
}

void UBNetworkAccessManager::authenticationRequired(QNetworkReply *reply, QAuthenticator *auth)
{
    QWidget *mainWindow = QApplication::activeWindow();

    QDialog dialog(mainWindow);
    dialog.setWindowFlags(Qt::Sheet);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.iconLabel->setText(QString());
    passwordDialog.iconLabel->setPixmap(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow).pixmap(32, 32));

    QString introMessage = tr("<qt>Enter username and password for \"%1\" at %2</qt>");
    introMessage = introMessage.arg((reply->url().toString()).toHtmlEscaped()).arg((reply->url().toString()).toHtmlEscaped());
    passwordDialog.introLabel->setText(introMessage);
    passwordDialog.introLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted)
    {
        if(auth && passwordDialog.userNameLineEdit)
            auth->setUser(passwordDialog.userNameLineEdit->text());
        if(auth && passwordDialog.passwordLineEdit)
            auth->setPassword(passwordDialog.passwordLineEdit->text());
    }

}

void UBNetworkAccessManager::proxyAuthenticationRequired(const QNetworkProxy &proxy, QAuthenticator *auth)
{
    Q_UNUSED(proxy);

    QString username = UBSettings::settings()->proxyUsername();
    QString password = UBSettings::settings()->proxyPassword();

    if (username.length() > 0 || password.length() > 0)
    {
        auth->setUser(username);
        auth->setPassword(password);
    }

    mProxyAuthenticationCount++;

    if (mProxyAuthenticationCount == 3)
    {
        UBApplication::showMessage(tr("Failed to log to Proxy"));
        disconnect(SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*))
                , this, SLOT(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

    }

    return;

}

void UBNetworkAccessManager::sslErrors(QNetworkReply *reply, const QList<QSslError> &error)
{
    // check if SSL certificate has been trusted already
    QString replyHost = reply->url().host() + ":" + reply->url().port();
    if(!sslTrustedHostList.contains(replyHost))
    {
        QWidget *mainWindow = QApplication::activeWindow();

        QStringList errorStrings;
        for (int i = 0; i < error.count(); ++i)
            errorStrings += error.at(i).errorString();

        QString errors = errorStrings.join(QLatin1String("\n"));

        QMessageBox messageBox;
        messageBox.setParent(mainWindow);
        messageBox.setWindowFlags(Qt::Dialog);
        messageBox.setWindowTitle(QCoreApplication::applicationName());
        messageBox.setText(tr("SSL Errors:\n\n%1\n\n%2\n\n"
                              "Do you want to ignore these errors for this host?").arg(reply->url().toString()).arg(errors));
        QPushButton* yesButton = messageBox.addButton(tr("Yes"),QMessageBox::YesRole);
        messageBox.addButton(tr("No"),QMessageBox::NoRole);
        messageBox.setIcon(QMessageBox::Question);
        messageBox.exec();

        if(messageBox.clickedButton() == yesButton) {
            reply->ignoreSslErrors();
            sslTrustedHostList.append(replyHost);
        }
    }
}
