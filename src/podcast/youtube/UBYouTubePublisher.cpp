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




#include "UBYouTubePublisher.h"



#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "gui/UBMainWindow.h"

#include "network/UBNetworkAccessManager.h"
#include "network/UBServerXMLHttpRequest.h"

#include "core/memcheck.h"

// API key linked to account dev.mnemis@gmail.com
const QString UBYouTubePublisher::sYouTubeDeveloperKey("AI39si62ga82stA4YBr5JjkfuRsFT-QyC4UYsFn7yYQFMe_dzg8xOc0r91BOhxSEhEr0gdWJGNnDsYbv9wvpyROd2Yre-6Zh7g");

UBYouTubePublisher::UBYouTubePublisher(QObject* pParent)
    : QObject(pParent)
    , mAuthRequest(0)
    , mUploadRequest(0)
{
    // NOOP
}


UBYouTubePublisher::~UBYouTubePublisher()
{
    // NOOP
}


void UBYouTubePublisher::uploadVideo(const QString& videoFilePath)
{
    mVideoFilePath = videoFilePath;

    UBYouTubePublishingDialog pub(videoFilePath, UBApplication::mainWindow);

    pub.title->setText(QFileInfo(mVideoFilePath).completeBaseName());
    pub.keywords->setText(qApp->applicationName());

    QString defaultEMail = UBSettings::settings()->youTubeUserEMail->get().toString();
    pub.email->setText(defaultEMail);

    QString defaultPassword = UBSettings::settings()->password(defaultEMail);
    pub.password->setText(defaultPassword);

    if (pub.exec() == QDialog::Accepted)
    {
        mTitle = pub.title->text();
        mDescription = pub.description->toPlainText();
        mCategories << pub.category->itemData(pub.category->currentIndex()).toString();
        mKeywords = pub.keywords->text();

        QString email = pub.email->text();
        UBSettings::settings()->youTubeUserEMail->set(email);

        QString password = pub.password->text();

        UBSettings::settings()->setPassword(email, password);

        postClientLoginRequest(email, password);
    }
    else
    {
        deleteLater();
    }
}


void UBYouTubePublisher::postClientLoginRequest(const QString& userName, const QString& password)
{
    QString mUserName = userName;

    QUrl url("https://www.google.com/youtube/accounts/ClientLogin");

    mAuthRequest = new UBServerXMLHttpRequest(UBNetworkAccessManager::defaultAccessManager()
                                              , "application/x-www-form-urlencoded"); // destroyed in postClientLoginResponse

    connect(mAuthRequest, SIGNAL(finished(bool, const QByteArray&)), this, SLOT(postClientLoginResponse(bool, const QByteArray&)));

    mAuthRequest->addHeader("X-GData-Key", sYouTubeDeveloperKey);

    QString payload = QString("Email=%1&Passwd=%2&service=youtube&source=%3")
            .arg(userName)
            .arg(password)
            .arg(qApp->applicationName());

    mAuthRequest->post(url, payload.toUtf8());

}


void UBYouTubePublisher::postClientLoginResponse(bool success, const QByteArray& pPayload)
{
    Q_UNUSED(success);
    mAuthToken = "";

    QString auth = QString::fromUtf8(pPayload);

    if (auth.contains("Auth="))
    {
        QStringList lines = auth.split("\n");

        foreach(QString line, lines)
        {
            if(line.startsWith("Auth="))
            {
                mAuthToken = line.replace("Auth=", "");
                break;
            }
        }
    }

    mAuthRequest->deleteLater();
    mAuthRequest = 0;

    if(mAuthToken.length() == 0)
    {
        UBApplication::showMessage(tr("YouTube authentication failed."));
        //        success = false;
        deleteLater();
    }
    else
    {
        qDebug() << "Retreived youtube auth token" << mAuthToken;
        postVideoUploadRequest();
    }
}


void UBYouTubePublisher::postVideoUploadRequest()
{
    /*
     *  POST /feeds/api/users/default/uploads HTTP/1.1
        Host: gdata.youtube.com
        Authorization: GoogleLogin auth=<authentication_token>
        GData-Version: 2
        X-GData-Client: <client_id>
        X-GData-Key: key=<developer_key>
        Slug: <video_filename>
        Content-Type: multipart/related; boundary="<boundary_string>"
        Content-Length: <content_length>
        Connection: close

        --<boundary_string>
        Content-Type: application/atom+xml; charset=UTF-8

        API_XML_request
        --<boundary_string>
        Content-Type: <video_content_type>
        Content-Transfer-Encoding: binary

        <Binary File Data>
        --<boundary_string>--
     */

    QFile videoFile(mVideoFilePath);

    if(!videoFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot open file" << mVideoFilePath << "for upload to youtube";
        return;
    }

    QUrl url("http://uploads.gdata.youtube.com/feeds/api/users/default/uploads");

    QString boundary = "---------------------------f93dcbA3";
    QString contentType = QString("multipart/related; boundary=\"%1\"").arg(boundary);

    mUploadRequest = new UBServerXMLHttpRequest(UBNetworkAccessManager::defaultAccessManager()
                                                , contentType); // destroyed in postVideoUploadResponse

    mUploadRequest->setVerbose(true);
    connect(mUploadRequest, SIGNAL(progress(qint64, qint64)), this,  SLOT(progress(qint64, qint64)));

    connect(mUploadRequest, SIGNAL(finished(bool, const QByteArray&)), this, SLOT(postVideoUploadResponse(bool, const QByteArray&)));

    mUploadRequest->addHeader("X-GData-Key", "key=" + sYouTubeDeveloperKey);
    mUploadRequest->addHeader("Authorization", QString("GoogleLogin auth=%1").arg(mAuthToken));
    mUploadRequest->addHeader("GData-Version", "2");

    QFileInfo fi(mVideoFilePath);
    mUploadRequest->addHeader("Slug", fi.fileName());
    mUploadRequest->addHeader("Connection", "close"); // do we really ned that ?

    QByteArray payload;

    payload.append(QString("\n--" + boundary + "\n").toUtf8())
            .append(QString("Content-Type: application/atom+xml; charset=UTF-8\n\n").toUtf8())
            .append(youtubeMetadata().toUtf8());

    payload.append(QString("\n--" + boundary + "\n").toUtf8());

    QString videoMimeType = UBFileSystemUtils::mimeTypeFromFileName(mVideoFilePath);

    payload.append((QString("Content-Type: %1\n").arg(videoMimeType)).toUtf8())
            .append(QString("Content-Transfer-Encoding: binary\n\n").toUtf8());

    payload.append(videoFile.readAll());

    payload.append(QString("\n--" + boundary + "--\n").toUtf8());

    mUploadRequest->post(url, payload);
}


void UBYouTubePublisher::postVideoUploadResponse(bool success, const QByteArray& pPayload)
{
    mUploadRequest->deleteLater();

    if(success)
    {
        UBApplication::showMessage("The video has been uploaded to youtube", false);
    }
    else
    {
        qWarning() << "error uploading video to youtube" << QString::fromUtf8(pPayload);

        UBApplication::showMessage(tr("Error while uploading video to YouTube (%1)").arg(QString::fromUtf8(pPayload)), false);
    }

    deleteLater();
}


QString UBYouTubePublisher::youtubeMetadata()
{
    QString workingTitle;

    if (mTitle.length() > 0)
    {
        workingTitle = mTitle;
    }
    else
    {
        workingTitle = QFileInfo(mVideoFilePath).completeBaseName();
    }

    QString metadata;

    metadata += "<?xml version=\"1.0\"?>\n";
    metadata += "<entry xmlns=\"http://www.w3.org/2005/Atom\" xmlns:media=\"http://search.yahoo.com/mrss/\" xmlns:yt=\"http://gdata.youtube.com/schemas/2007\">\n";
    metadata += "  <media:group>\n";
    metadata += QString("    <media:title type=\"plain\">%1</media:title>\n").arg(workingTitle);

    QString workingDescription = mDescription;

    if (workingDescription.length() > 4900)
    {
        workingDescription = workingDescription.left(4900) + "...";
    }

    workingDescription += "\n\nhttp://www.openboard.org";

    if(workingDescription.length() == 0)
    {
        workingDescription = workingTitle;
    }

    metadata += QString("    <media:description type=\"plain\">%1</media:description>\n").arg(workingDescription);

    foreach(QString cat, mCategories)
    {
        metadata += QString("    <media:category scheme=\"http://gdata.youtube.com/schemas/2007/categories.cat\">%1</media:category>\n").arg(cat);
    }

    if (mKeywords.length() > 0)
    {
        metadata += QString("    <media:keywords>%1</media:keywords>\n").arg(mKeywords);
    }

    metadata += "  </media:group>\n";
    metadata += "</entry>";

    return metadata;

}


void UBYouTubePublisher::progress (qint64 bytesSent, qint64 bytesTotal )
{
    int percentage = (((qreal)bytesSent / (qreal)bytesTotal ) * 100);

    UBApplication::showMessage(tr("Upload to YouTube in progress %1 %").arg(percentage), percentage < 100);
}



UBYouTubePublishingDialog::UBYouTubePublishingDialog(const QString& videoFilePath, QWidget *parent)
    : QDialog(parent)
{
    Q_UNUSED(videoFilePath);

    Ui::YouTubePublishingDialog::setupUi(this);

    QMap<QString, QString> cats = categories();

    category->clear();
    int index = 0;
    foreach(QString cat, cats.keys())
    {
        category->addItem(cats.value(cat), cat);
        if(cat == "Education")
            category->setCurrentIndex(index);

        index++;
    }

    connect(dialogButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialogButtons, SIGNAL(rejected()), this, SLOT(reject()));

    connect(title, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));
    connect(description, SIGNAL(textChanged()), this, SLOT(updateUIState()));
    connect(keywords, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));

    connect(email, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));
    connect(password, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));
    connect(youtubeCredentialsPersistence,SIGNAL(clicked()), this, SLOT(updateCredentialPersistenceState()));

    dialogButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    dialogButtons->button(QDialogButtonBox::Ok)->setText(tr("Upload"));

    UBSettings* settings = UBSettings::settings();

    email->setText(settings->youTubeUserEMail->get().toString());
    password->setText(settings->password(email->text()));

    youtubeCredentialsPersistence->setChecked(UBSettings::settings()->youTubeCredentialsPersistence->get().toBool());
    updatePersistanceEnableState();
}


void UBYouTubePublishingDialog::updateCredentialPersistenceState()
{
    UBSettings::settings()->youTubeCredentialsPersistence->set(QVariant(youtubeCredentialsPersistence->checkState()));
}

void UBYouTubePublishingDialog::updatePersistanceEnableState()
{
    bool enabled = email->text().length() || password->text().length();
    youtubeCredentialsPersistence->setEnabled(enabled);
    youtubeCredentialsPersistence->setStyleSheet(enabled ? "color:black;" : "color : lightgrey;");
}

void UBYouTubePublishingDialog::updateUIState(const QString& string)
{
    Q_UNUSED(string);

    bool ok = title->text().length() > 0
            &&  description->toPlainText().length() > 0
            &&  keywords->text().length() > 0
            &&  email->text().length() > 0
            &&  password->text().length() > 0;

    dialogButtons->button(QDialogButtonBox::Ok)->setEnabled(ok);
    updatePersistanceEnableState();
}


QMap<QString, QString> UBYouTubePublishingDialog::categories()
{
    // TODO UB 4.x download localized list from
    // http://code.google.com/apis/youtube/2.0/reference.html#Localized_Category_Lists

    QMap<QString, QString> cats;

    cats.insert("Autos", tr("Autos & Vehicles"));
    cats.insert("Music", tr("Music"));
    cats.insert("Animals", tr("Pets & Animals"));
    cats.insert("Sports", tr("Sports"));
    cats.insert("Travel", tr("Travel & Events"));
    cats.insert("Games", tr("Gaming"));
    cats.insert("Comedy", tr("Comedy"));
    cats.insert("People", tr("People & Blogs"));
    cats.insert("News", tr("News & Politics"));
    cats.insert("Entertainment", tr("Entertainment"));
    cats.insert("Education", tr("Education"));
    cats.insert("Howto", tr("Howto & Style"));
    cats.insert("Nonprofit", tr("Nonprofits & Activism"));
    cats.insert("Tech", tr("Science & Technology"));

    return cats;
}

