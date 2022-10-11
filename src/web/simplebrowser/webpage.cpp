/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "ui_certificateerrordialog.h"
#include "ui_passworddialog.h"
#include "webpage.h"
#include <QAuthenticator>
#include <QMessageBox>
#include <QStyle>
#include <QWebEngineCertificateError>

#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"
#include "web/UBWebController.h"

WebPage::WebPage(QWebEngineProfile *profile, QObject *parent)
    : QWebEnginePage(profile, parent)
{
    connect(this, &QWebEnginePage::authenticationRequired, this, &WebPage::handleAuthenticationRequired);
    connect(this, &QWebEnginePage::featurePermissionRequested, this, &WebPage::handleFeaturePermissionRequested);
    connect(this, &QWebEnginePage::proxyAuthenticationRequired, this, &WebPage::handleProxyAuthenticationRequired);
    connect(this, &QWebEnginePage::registerProtocolHandlerRequested, this, &WebPage::handleRegisterProtocolHandlerRequested);
#if !defined(QT_NO_SSL) || QTWEBENGINEWIDGETS_VERSION >= QT_VERSION_CHECK(5, 12, 0)
    connect(this, &QWebEnginePage::selectClientCertificate, this, &WebPage::handleSelectClientCertificate);
#endif
#if (QTWEBENGINEWIDGETS_VERSION >= QT_VERSION_CHECK(6, 0, 0))
    connect(this, &QWebEnginePage::certificateError, this, &WebPage::certificateError);
#endif
}

#if (QTWEBENGINEWIDGETS_VERSION >= QT_VERSION_CHECK(6, 0, 0))
void WebPage::handleCertificateError(QWebEngineCertificateError error)
{
    QWidget *mainWindow = UBApplication::mainWindow->centralWidget(); //view()->window();
    if (error.isOverridable()) {
        QDialog dialog(mainWindow);
        dialog.setModal(true);
        dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        Ui::CertificateErrorDialog certificateDialog;
        certificateDialog.setupUi(&dialog);
        certificateDialog.m_iconLabel->setText(QString());
        QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, mainWindow));
        certificateDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));
        certificateDialog.m_errorLabel->setText(error.description());
        dialog.setWindowTitle(tr("Certificate Error"));

        if (dialog.exec() == QDialog::Accepted) {
            error.acceptCertificate();
        } else {
            error.rejectCertificate();
        }
    }

    QMessageBox::critical(mainWindow, tr("Certificate Error"), error.description());
    error.rejectCertificate();
}
#else
bool WebPage::certificateError(const QWebEngineCertificateError &error)
{
    QWidget *mainWindow = UBApplication::mainWindow->centralWidget(); //view()->window();
    if (error.isOverridable()) {
        QDialog dialog(mainWindow);
        dialog.setModal(true);
        dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
        Ui::CertificateErrorDialog certificateDialog;
        certificateDialog.setupUi(&dialog);
        certificateDialog.m_iconLabel->setText(QString());
        QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxWarning, 0, mainWindow));
        certificateDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));
        certificateDialog.m_errorLabel->setText(error.errorDescription());
        dialog.setWindowTitle(tr("Certificate Error"));
        return dialog.exec() == QDialog::Accepted;
    }

    QMessageBox::critical(mainWindow, tr("Certificate Error"), error.errorDescription());
    return false;
}
#endif

void WebPage::handleAuthenticationRequired(const QUrl &requestUrl, QAuthenticator *auth)
{
    QWidget *mainWindow = UBApplication::mainWindow->centralWidget(); // view()->window();
    QDialog dialog(mainWindow);
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.m_iconLabel->setText(QString());
    QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow));
    passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

    QString introMessage(tr("Enter username and password for \"%1\" at %2")
                         .arg(auth->realm()).arg(requestUrl.toString().toHtmlEscaped()));
    passwordDialog.m_infoLabel->setText(introMessage);
    passwordDialog.m_infoLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.m_userNameLineEdit->text());
        auth->setPassword(passwordDialog.m_passwordLineEdit->text());
    } else {
        // Set authenticator null if dialog is cancelled
        *auth = QAuthenticator();
    }
}

inline QString questionForFeature(QWebEnginePage::Feature feature)
{
    switch (feature) {
    case QWebEnginePage::Geolocation:
        return WebPage::tr("Allow %1 to access your location information?");
    case QWebEnginePage::MediaAudioCapture:
        return WebPage::tr("Allow %1 to access your microphone?");
    case QWebEnginePage::MediaVideoCapture:
        return WebPage::tr("Allow %1 to access your webcam?");
    case QWebEnginePage::MediaAudioVideoCapture:
        return WebPage::tr("Allow %1 to access your microphone and webcam?");
    case QWebEnginePage::MouseLock:
        return WebPage::tr("Allow %1 to lock your mouse cursor?");
    case QWebEnginePage::DesktopVideoCapture:
        return WebPage::tr("Allow %1 to capture video of your desktop?");
    case QWebEnginePage::DesktopAudioVideoCapture:
        return WebPage::tr("Allow %1 to capture audio and video of your desktop?");
    case QWebEnginePage::Notifications:
        return QString();
    }
    return QString();
}

void WebPage::handleFeaturePermissionRequested(const QUrl &securityOrigin, Feature feature)
{
    PermissionPolicy policy = UBApplication::webController->hasFeaturePermission(securityOrigin, feature);

    if (policy == PermissionUnknown)
    {
        QString title = tr("Permission Request");
        QString question = questionForFeature(feature).arg(securityOrigin.host());

        if (!question.isEmpty() && QMessageBox::question(UBApplication::mainWindow->centralWidget(), title, question) == QMessageBox::Yes)
            policy = PermissionGrantedByUser;
        else
            policy = PermissionDeniedByUser;

        UBApplication::webController->setFeaturePermission(securityOrigin, feature, policy);
    }

    setFeaturePermission(securityOrigin, feature, policy);
}

void WebPage::handleProxyAuthenticationRequired(const QUrl &, QAuthenticator *auth, const QString &proxyHost)
{
    QWidget *mainWindow = UBApplication::mainWindow->centralWidget(); //view()->window();
    QDialog dialog(mainWindow);
    dialog.setModal(true);
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);

    Ui::PasswordDialog passwordDialog;
    passwordDialog.setupUi(&dialog);

    passwordDialog.m_iconLabel->setText(QString());
    QIcon icon(mainWindow->style()->standardIcon(QStyle::SP_MessageBoxQuestion, 0, mainWindow));
    passwordDialog.m_iconLabel->setPixmap(icon.pixmap(32, 32));

    QString introMessage = tr("Connect to proxy \"%1\" using:");
    introMessage = introMessage.arg(proxyHost.toHtmlEscaped());
    passwordDialog.m_infoLabel->setText(introMessage);
    passwordDialog.m_infoLabel->setWordWrap(true);

    if (dialog.exec() == QDialog::Accepted) {
        auth->setUser(passwordDialog.m_userNameLineEdit->text());
        auth->setPassword(passwordDialog.m_passwordLineEdit->text());
    } else {
        // Set authenticator null if dialog is cancelled
        *auth = QAuthenticator();
    }
}

//! [registerProtocolHandlerRequested]
void WebPage::handleRegisterProtocolHandlerRequested(QWebEngineRegisterProtocolHandlerRequest request)
{
    auto answer = QMessageBox::question(
        UBApplication::mainWindow->centralWidget(), //view()->window(),
        tr("Permission Request"),
        tr("Allow %1 to open all %2 links?")
        .arg(request.origin().host())
        .arg(request.scheme()));
    if (answer == QMessageBox::Yes)
        request.accept();
    else
        request.reject();
}
//! [registerProtocolHandlerRequested]

#if !defined(QT_NO_SSL) || QTWEBENGINEWIDGETS_VERSION >= QT_VERSION_CHECK(5, 12, 0)
void WebPage::handleSelectClientCertificate(QWebEngineClientCertificateSelection selection)
{
    // Just select one.
    selection.select(selection.certificates().at(0));
}
#endif
