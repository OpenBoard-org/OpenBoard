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
#ifndef UBCAPTUREPUBLISHER_H
#define UBCAPTUREPUBLISHER_H

#include <QtGui>

#include "ui_capturePublishing.h"

#include "UBAbstractPublisher.h"

class UBServerXMLHttpRequest;

class UBCapturePublisher : public UBAbstractPublisher
{
    Q_OBJECT

    public:
        explicit UBCapturePublisher(const QPixmap& pixmap, QObject *parent = 0);

        void publish();

    private slots:

        void publishImageOnWebUploadResponse(bool success, const QByteArray& payload);
        void postPixmap(const QUuid& tokenUuid, const QString& encryptedBase64Token);

    private:

        UBServerXMLHttpRequest *mPublishImageOnWebUploadRequest;
        QUuid mWebUploadPublishingUuid;
        QPixmap mPixmap;

};

class UBCapturePublishingDialog : public QDialog, public Ui::capturePublishingDialog
{
    Q_OBJECT;

    public:
        UBCapturePublishingDialog(QWidget *parent = 0);
        ~UBCapturePublishingDialog(){}

    private slots:
        void updateUIState(const QString& string);

};

#endif // UBCAPTUREPUBLISHER_H
