/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#include "UBIntranetPodcastPublisher.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBStringUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"

#include "network/UBServerXMLHttpRequest.h"
#include "network/UBNetworkAccessManager.h"

#include "gui/UBMainWindow.h"

#include "core/memcheck.h"


UBIntranetPodcastPublisher::UBIntranetPodcastPublisher(QObject* pParent)
    : QObject(pParent)
    , mPublishingRequest(0)
{
    // NOOP
}


UBIntranetPodcastPublisher::~UBIntranetPodcastPublisher()
{
    // NOOP
}


void UBIntranetPodcastPublisher::publishVideo(const QString& videoFilePath, long durationInMs)
{
    mVideoFilePath = videoFilePath;
    mVideoDurationInMs = durationInMs;

    UBIntranetPodcastPublishingDialog pub(mVideoFilePath, UBApplication::mainWindow);

    pub.title->setText(QFileInfo(mVideoFilePath).completeBaseName());

    QString defaultAuthor = UBSettings::settings()->intranetPodcastAuthor->get().toString();
    pub.author->setText(defaultAuthor);

    if (pub.exec() == QDialog::Accepted)
    {
        mTitle = pub.title->text();
        mDescription = pub.description->toPlainText();
        mAuthor  = pub.author->text();

        UBSettings::settings()->intranetPodcastAuthor->set(mAuthor);

        postVideoPublishingRequest();
    }
    else
    {
        deleteLater();
    }
}


void UBIntranetPodcastPublisher::postVideoPublishingRequest()
{
    UBSetting *urlSetting = UBSettings::settings()->intranetPodcastPublishingUrl;

    QString publishingUrl = urlSetting->get().toString();

    if (publishingUrl.length() == 0)
    {
        UBApplication::showMessage(QString("Publishing video to the intranet failed, the target URL is not set ( %1 )").arg(urlSetting->path()), true);
        deleteLater();
        return;
    }

    QUrl url(publishingUrl);

    /*
    QFile videoFile(mVideoFilePath);

    if(!videoFile.open(QIODevice::ReadOnly))
    {
        qWarning() << "Cannot read file" << mVideoFilePath << "for publishing to intranet";
    }
    */

    UBApplication::showMessage("Publishing video to the intranet", true);

    mPublishingRequest = new UBServerXMLHttpRequest(UBNetworkAccessManager::defaultAccessManager());

    connect(mPublishingRequest, SIGNAL(progress(qint64, qint64)), this,  SLOT(progress(qint64, qint64)));
    connect(mPublishingRequest, SIGNAL(finished(bool, const QByteArray&)), this, SLOT(postVideoPublishingResponse(bool, const QByteArray&)));

    QByteArray payload;

    payload.append(metadata().toUtf8());

    mPublishingRequest->setVerbose(true);

    mPublishingRequest->post(url, payload);
}


QString UBIntranetPodcastPublisher::metadata()
{
    QStringList metadatas;

    /*
        <podcast>
          <title>{title}</title>
          <description>{description}</description>
          <author>{author}</author>
          <pubDate>{publicationISODateTime}</pubDate>
          <duration>{duration}</duration>
          <fileName>{fileName}</fileName>
          <computerName>{computerName}</computerName>
          <fileSize>{fileSize}</fileSize>
        </podcast>
     */

    QString date = UBStringUtils::toUtcIsoDateTime(QDateTime::currentDateTime());
    QString duration = QString("%1").arg((mVideoDurationInMs / 1000) + 1);

    QFileInfo fi(mVideoFilePath);
    QString fileName = fi.fileName();
    QString computerName =  UBPlatformUtils::computerName();
    QString fileSize = QString("%1").arg(fi.size());

    QString templatePath = UBPlatformUtils::applicationResourcesDirectory() + "/template/intranet-podcast-metadata.template";
    QString templateContent = UBFileSystemUtils::readTextFile(templatePath);

    return templateContent.replace("{title}", mTitle)
                          .replace("{description}", mDescription)
                          .replace("{author}", mAuthor)
                          .replace("{publicationISODateTime}", date)
                          .replace("{duration}", duration)
                          .replace("{fileName}", fileName)
                          .replace("{computerName}", computerName)
                          .replace("{fileSize}", fileSize)
                          .replace("{filePath}", mVideoFilePath);

}


void UBIntranetPodcastPublisher::postVideoPublishingResponse(bool success, const QByteArray& pPayload)
{
    mPublishingRequest->deleteLater();

    if(success)
    {
        UBApplication::showMessage("The video has been published to the intranet", false);
    }
    else
    {
        qWarning() << "error publishing video to intranet" << QString::fromUtf8(pPayload);

        UBApplication::showMessage(tr("Error while publishing video to intranet (%1)").arg(QString::fromUtf8(pPayload)), false);
    }

    deleteLater();
}


void UBIntranetPodcastPublisher::progress(qint64 bytesSent, qint64 bytesTotal)
{
    int percentage = (((qreal)bytesSent / (qreal)bytesTotal ) * 100);

    UBApplication::showMessage(tr("Publishing to Intranet in progress %1 %").arg(percentage), percentage < 100);
}


UBIntranetPodcastPublishingDialog::UBIntranetPodcastPublishingDialog(const QString& videoFilePath, QWidget *parent)
    : QDialog(parent)
{
    Q_UNUSED(videoFilePath);

    Ui::IntranetPodcastPublishingDialog::setupUi(this);

    connect(dialogButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialogButtons, SIGNAL(rejected()), this, SLOT(reject()));

    connect(title, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));
    connect(description, SIGNAL(textChanged()), this, SLOT(updateUIState()));
    connect(author, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));

    dialogButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    dialogButtons->button(QDialogButtonBox::Ok)->setText(tr("Publish"));
}


void UBIntranetPodcastPublishingDialog::updateUIState(const QString& string)
{
    Q_UNUSED(string);

    bool ok = title->text().length() > 0
                &&  description->toPlainText().length() > 0
                &&  author->text().length() > 0;

    dialogButtons->button(QDialogButtonBox::Ok)->setEnabled(ok);
}
