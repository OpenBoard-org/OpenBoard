
/*
 * UBSoftwareUpdateController.h
 *
 *  Created on: May 11, 2009
 *      Author: Jerome Marchaud
 */

//#ifndef UBSOFTWAREUPDATECONTROLLER_H_
//#define UBSOFTWAREUPDATECONTROLLER_H_

//#include <QObject>
//#include <QUrl>

//class UBVersion;
//class UBSoftwareUpdate;
//class UBHttpGet;

//class UBSoftwareUpdateController : public QObject
//{
//    Q_OBJECT;

//    public:
//        UBSoftwareUpdateController(QObject *parent = 0);
//        virtual ~UBSoftwareUpdateController();

//        void beginRssDownload(const QUrl &url);
//        void beginInstallerDownload(const QUrl &url);

//        static const UBSoftwareUpdate* searchForMoreRecentAndAsStableSoftwareUpdate(const UBVersion &installedVersion,
//                const QList<UBSoftwareUpdate *> &softwareUpdates);

//    signals:
//        void softwareUpdateAvailable(const UBVersion &installedVersion, const UBSoftwareUpdate &softwareUpdate);
//        void noSoftwareUpdateAvailable();
//        void failedToRetrieveSoftwareUpdateInfo();

//    private slots:
//        void rssDownloadFinished(bool success, const QByteArray &payload);
//        void installerDownloadProgress(qint64 receivedBytes, qint64 bytesTotal);
//        void installerDownloadFinished(bool success, QUrl sourceUrl, QString header, QByteArray data);

//    private:
//        // Helpers
//        void parseRss(const QByteArray &rssContent);

//        UBHttpGet *mHttp;
//        qreal mLastDisplayedProgress;

//        static const int sMinDisplayedDownloadedSizeInBytes;
//        static const qreal sProgressPercentageStep;
//};

//#endif // UBSOFTWAREUPDATECONTROLLER_H_
