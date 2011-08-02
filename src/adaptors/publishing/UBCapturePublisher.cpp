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
#include "UBCapturePublisher.h"

#include "frameworks/UBStringUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "gui/UBMainWindow.h"

#include "board/UBBoardController.h"

#include "network/UBServerXMLHttpRequest.h"
#include "network/UBNetworkAccessManager.h"

#include "domain/UBGraphicsScene.h"

#include "core/memcheck.h"

UBCapturePublisher::UBCapturePublisher(const QPixmap& pixmap, QObject *parent)
    : UBAbstractPublisher(parent)
    , mPixmap(pixmap)
{
    connect(this, SIGNAL(authenticated(const QUuid&, const QString&))
            , this, SLOT(postPixmap(const QUuid&, const QString&)));
}


void UBCapturePublisher::publish()
{
    UBAbstractPublisher::authenticate();
}


void UBCapturePublisher::postPixmap(const QUuid& tokenUuid, const QString& encryptedBase64Token)
{
    UBCapturePublishingDialog dialog(UBApplication::mainWindow);

    QString defaultEMail = UBSettings::settings()->uniboardWebEMail->get().toString();
    dialog.email->setText(defaultEMail);

    QString defaultAuthor = UBSettings::settings()->uniboardWebAuthor->get().toString();
    dialog.author->setText(defaultAuthor);

    if (dialog.exec() == QDialog::Accepted)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        UBApplication::showMessage(tr("Preparing capture for upload..."), true);

        QString title = dialog.title->text();
        QString description = dialog.description->toPlainText();
        QString email = dialog.email->text();
        QString author = dialog.author->text();

        QPixmap pix(mPixmap);

        if (mPixmap.hasAlpha())
        {
            if (UBApplication::boardController->activeScene()->isDarkBackground())
                pix.fill(Qt::black);
            else
                pix.fill(Qt::white);

            QPainter p(&pix);
            p.drawPixmap(0, 0, mPixmap);
        }

        QByteArray bytes;
        QBuffer buffer(&bytes);
        buffer.open(QIODevice::WriteOnly);
        pix.save(&buffer, "JPG", 80);
        buffer.close();

        QUrl publishingEndpoint = QUrl(UBSettings::settings()->capturesPublishingUrl);

        mPublishImageOnWebUploadRequest = new UBServerXMLHttpRequest(UBNetworkAccessManager::defaultAccessManager()
            , "application/octet-stream");

        mPublishImageOnWebUploadRequest->setVerbose(true);

        connect(mPublishImageOnWebUploadRequest, SIGNAL(finished(bool, const QByteArray&)), this, SLOT(publishImageOnWebUploadResponse(bool, const QByteArray&)));

        mWebUploadPublishingUuid = QUuid::createUuid();

        mPublishImageOnWebUploadRequest->addHeader("Publishing-UUID", UBStringUtils::toCanonicalUuid(mWebUploadPublishingUuid));
        mPublishImageOnWebUploadRequest->addHeader("Document-Title", title);
        mPublishImageOnWebUploadRequest->addHeader("Document-Author", author);
        mPublishImageOnWebUploadRequest->addHeader("Document-AuthorEMail", email);
        mPublishImageOnWebUploadRequest->addHeader("Document-Description", description);
        mPublishImageOnWebUploadRequest->addHeader("Deletion-Token", UBStringUtils::toCanonicalUuid(QUuid::createUuid()));
        mPublishImageOnWebUploadRequest->addHeader("Token-UUID", UBStringUtils::toCanonicalUuid(tokenUuid));
        mPublishImageOnWebUploadRequest->addHeader("Token-Encrypted", encryptedBase64Token);

        mPublishImageOnWebUploadRequest->post(publishingEndpoint, bytes);
    }
    else
    {
        UBApplication::showMessage(tr("Publication canceled ..."));
        QApplication::restoreOverrideCursor();
    }
}


void UBCapturePublisher::publishImageOnWebUploadResponse(bool success, const QByteArray& payload)
{
    QUrl url(QString::fromUtf8(payload));

    if (success && url.isValid())
    {
        UBApplication::showMessage(tr("Capture Published to the Web."));
    }
    else
    {
        UBApplication::showMessage(tr("Error Publishing Capture to the Web: %1").arg(QString::fromUtf8(payload)));
    }

    if (mPublishImageOnWebUploadRequest)
    {
        mPublishImageOnWebUploadRequest->deleteLater();
        mPublishImageOnWebUploadRequest = 0;
    }

    QApplication::restoreOverrideCursor();
}


UBCapturePublishingDialog::UBCapturePublishingDialog(QWidget *parent)
    : QDialog(parent)
{
    Ui::capturePublishingDialog::setupUi(this);

    connect(dialogButtons, SIGNAL(accepted()), this, SLOT(accept()));
    connect(dialogButtons, SIGNAL(rejected()), this, SLOT(reject()));

    connect(title, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));
    connect(email, SIGNAL(textChanged(const QString&)), this, SLOT(updateUIState(const QString&)));

    dialogButtons->button(QDialogButtonBox::Ok)->setEnabled(false);
    dialogButtons->button(QDialogButtonBox::Ok)->setText(tr("Publish"));
}


void UBCapturePublishingDialog::updateUIState(const QString& string)
{
    Q_UNUSED(string);

    bool ok = title->text().length() > 0
                    &&  email->text().length() > 0;

    dialogButtons->button(QDialogButtonBox::Ok)->setEnabled(ok);
}
