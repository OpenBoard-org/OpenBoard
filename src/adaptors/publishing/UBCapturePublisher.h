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
