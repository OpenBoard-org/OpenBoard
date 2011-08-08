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
