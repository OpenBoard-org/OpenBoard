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

//#include "frameworks/UBVersion.h"
//#include "frameworks/UBFileSystemUtils.h"
//#include "frameworks/UBPlatformUtils.h"

//#include "UBSoftwareUpdateController.h"
//#include "UBRssHandler.h"
//#include "UBSoftwareUpdate.h"

//#include "core/UBApplication.h"
//#include "core/UBSettings.h"
//#include "core/UBSetting.h"

//#include "board/UBBoardController.h"

//#include "document/UBDocumentProxy.h"

//#include "network/UBHttpGet.h"
//#include "network/UBServerXMLHttpRequest.h"
//#include "network/UBNetworkAccessManager.h"

//const qreal UBSoftwareUpdateController::sProgressPercentageStep = 1;
//const int UBSoftwareUpdateController::sMinDisplayedDownloadedSizeInBytes = 2 * 1024 * 1024;

//UBSoftwareUpdateController::UBSoftwareUpdateController(QObject *parent)
//    : QObject(parent)
//    , mHttp(0)
//{
//    // NOOP
//}


//UBSoftwareUpdateController::~UBSoftwareUpdateController()
//{
//    delete mHttp;
//}


//void UBSoftwareUpdateController::beginRssDownload(const QUrl &url)
//{
//    UBServerXMLHttpRequest * request = new UBServerXMLHttpRequest(UBNetworkAccessManager::defaultAccessManager());
//    connect(request, SIGNAL(finished(bool, const QByteArray &)), this, SLOT(rssDownloadFinished(bool, const QByteArray &)));

//    request->get(url);
//}


//void UBSoftwareUpdateController::beginInstallerDownload(const QUrl &url)
//{
//    delete mHttp;
//    mHttp = new UBHttpGet();

//    connect(mHttp, SIGNAL(downloadProgress(qint64, qint64)), this, SLOT(installerDownloadProgress(qint64, qint64)));
//    connect(mHttp, SIGNAL(downloadFinished(bool, QUrl, QString, QByteArray, QPointF, QSize, bool)),
//            this, SLOT(installerDownloadFinished(bool, QUrl, QString, QByteArray)));

//    mLastDisplayedProgress = 0;

//    UBApplication::showMessage(tr("Downloading software update (%1%)").arg(0), true);
//    mHttp->get(url, QPointF(), QSize());
//}


//void UBSoftwareUpdateController::rssDownloadFinished(bool success, const QByteArray &payload)
//{
//    if (!success)
//    {
//        qWarning() << "Failed to download RSS file.";
//        failedToRetrieveSoftwareUpdateInfo();
//        return;
//    }

//    parseRss(payload);
//}


//void UBSoftwareUpdateController::installerDownloadProgress(qint64 receivedBytes, qint64 bytesTotal)
//{
//    if (bytesTotal > sMinDisplayedDownloadedSizeInBytes)
//    {
//        qreal progress = ((qreal)(receivedBytes * 100) / bytesTotal);
//        if (progress >= mLastDisplayedProgress + sProgressPercentageStep || receivedBytes == bytesTotal)
//        {
//            mLastDisplayedProgress = progress;
//            UBApplication::showMessage(tr("Downloading software update (%1%)").arg(progress, 0, 'f', 0), true);
//        }
//    }
//}


//void UBSoftwareUpdateController::installerDownloadFinished(bool success, QUrl sourceUrl, QString contentTypeHeader, QByteArray data)
//{
//    Q_UNUSED(contentTypeHeader);

//    if (!success)
//    {
//        UBApplication::showMessage(tr("Downloading software update failed"));
//        return;
//    }

//    UBApplication::showMessage(tr("Download finished"));

//    QStringList urlParts = sourceUrl.toString().split("/");
//    QString installerFileName = urlParts.last();
//    QString tempDirPath = UBFileSystemUtils::createTempDir(UBFileSystemUtils::defaultTempDirName(), false);
//    QString installerFilePath = tempDirPath + "/" + installerFileName;

//    QFile file(installerFilePath);

//    if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate))
//    {
//        qCritical() << "Cannot open " << installerFilePath << " for writing...";
//        return;
//    }

//    file.write(data);
//    file.flush();
//    file.close();

//    if (QMessageBox::question(
//            QApplication::activeWindow(),
//            tr("Software Update"),
//            tr("Are you sure you want to install this new version of Uniboard now?\nThis session will close as soon as installation begins."),
//            QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes)
//    {

//        UBDocumentProxy* document = UBApplication::boardController->activeDocument();
//        int sceneIndex = UBApplication::boardController->activeSceneIndex();

//        if (document)
//        {
//            UBSettings::settings()->appLastSessionDocumentUUID->set(UBStringUtils::toCanonicalUuid(document->uuid()));
//            UBSettings::settings()->appLastSessionPageIndex->set(sceneIndex);
//        }

//        UBSettings::settings()->appIsInSoftwareUpdateProcess->set(true);

//        UBPlatformUtils::runInstaller(installerFilePath);

//        UBApplication::quit();
//    }
//}


//void UBSoftwareUpdateController::parseRss(const QByteArray &rssContent)
//{
//    UBRssHandler rssHandler;
//    QXmlSimpleReader xmlReader;
//    xmlReader.setContentHandler(&rssHandler);
//    xmlReader.setErrorHandler(&rssHandler);
//    QXmlInputSource source;
//    source.setData(rssContent);

//    if (!xmlReader.parse(source))
//    {
//        failedToRetrieveSoftwareUpdateInfo();
//        return;
//    }

//    if (rssHandler.error().length() > 0)
//    {
//        qWarning() << "Failed to parse RSS file. Reason: " << rssHandler.error();
//        failedToRetrieveSoftwareUpdateInfo();
//        return;
//    }

//    if (rssHandler.softwareUpdates().isEmpty())
//    {
//        qWarning() << "RSS file does not contain software update info.";
//        failedToRetrieveSoftwareUpdateInfo();
//        return;
//    }

//    UBVersion installedVersion(qApp->applicationVersion());
//    if (!installedVersion.isValid())
//    {
//        qWarning() << "Failed to parse installed version format: " << qApp->applicationVersion();
//        failedToRetrieveSoftwareUpdateInfo();
//        return;
//    }

//    QList<UBSoftwareUpdate *> softwareUpdates = rssHandler.softwareUpdates();
//    const UBSoftwareUpdate *mostRecentSoftwareUpdate = searchForMoreRecentAndAsStableSoftwareUpdate(installedVersion, softwareUpdates);

//    if (mostRecentSoftwareUpdate)
//    {
//        emit softwareUpdateAvailable(installedVersion, *mostRecentSoftwareUpdate);
//    }
//    else
//    {
//        emit noSoftwareUpdateAvailable();
//    }
//}


//const UBSoftwareUpdate* UBSoftwareUpdateController::searchForMoreRecentAndAsStableSoftwareUpdate(const UBVersion &installedVersion, const QList<UBSoftwareUpdate *> &softwareUpdates)
//{
//    const UBSoftwareUpdate *mostRecentSoftwareUpdate = 0;
//    foreach (const UBSoftwareUpdate *softwareUpdate, softwareUpdates)
//    {
//        bool moreRecentAndAsStable = false;

//        if (Alpha == installedVersion.releaseStage())
//        {
//            if (installedVersion < softwareUpdate->version() &&
//                installedVersion.platformNumber() == softwareUpdate->version().platformNumber())
//            {
//                moreRecentAndAsStable = true;
//            }
//        }
//        else if (Beta == installedVersion.releaseStage())
//        {
//            if (Alpha != softwareUpdate->version().releaseStage() &&
//                installedVersion < softwareUpdate->version() &&
//                installedVersion.platformNumber() == softwareUpdate->version().platformNumber())
//            {
//                moreRecentAndAsStable = true;
//            }
//        }
//        else
//        {
//            if (ReleaseCandidate == softwareUpdate->version().releaseStage() &&
//                installedVersion < softwareUpdate->version() &&
//                installedVersion.platformNumber() == softwareUpdate->version().platformNumber())
//            {
//                moreRecentAndAsStable = true;
//            }
//        }

//        if (moreRecentAndAsStable)
//        {
//            if (mostRecentSoftwareUpdate)
//            {
//                if (mostRecentSoftwareUpdate->version() < softwareUpdate->version())
//                {
//                    mostRecentSoftwareUpdate = softwareUpdate;
//                }
//            }
//            else
//            {
//                mostRecentSoftwareUpdate = softwareUpdate;
//            }
//        }
//    }

//    return mostRecentSoftwareUpdate;
//}
