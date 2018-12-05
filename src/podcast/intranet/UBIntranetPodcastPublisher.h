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
