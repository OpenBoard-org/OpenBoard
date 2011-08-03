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

#ifndef UBUNILPUBLISHER_H_
#define UBUNILPUBLISHER_H_

#include <QtGui>

#include "ui_intranetPodcastPublishingDialog.h"

class UBServerXMLHttpRequest;

class UBIntranetPodcastPublisher : public QObject
{
    Q_OBJECT;

    public:
        UBIntranetPodcastPublisher(QObject* pParent = 0);
        virtual ~UBIntranetPodcastPublisher();

    public slots:

        void publishVideo(const QString& videoFilePath, long durationInMs);

        void postVideoPublishingRequest();
        void postVideoPublishingResponse(bool success, const QByteArray& pPayload);
        void progress(qint64 bytesSent, qint64 bytesTotal);

    private:

        QString metadata();

        QString mVideoFilePath;
        QString mTitle;
        QString mDescription;
        QString mAuthor;
        long mVideoDurationInMs;

        UBServerXMLHttpRequest *mPublishingRequest;

};


class UBIntranetPodcastPublishingDialog : public QDialog, public Ui::IntranetPodcastPublishingDialog
{
    Q_OBJECT;

    public:
        UBIntranetPodcastPublishingDialog(const QString& videoFilePath, QWidget *parent = 0);
        ~UBIntranetPodcastPublishingDialog(){};

    private slots:
        void updateUIState(const QString& = QString(""));

};

#endif /* UBUNILPUBLISHER_H_ */
