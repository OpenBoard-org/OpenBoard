#ifndef UBDOCUMENTPUBLISHER_H
#define UBDOCUMENTPUBLISHER_H

#include <QtGui>
#include <QtNetwork>

#include "ui_webPublishing.h"
#include "UBAbstractPublisher.h"

class UBDocumentProxy;
class UBServerXMLHttpRequest;
class UBGraphicsW3CWidgetItem;

class UBDocumentPublisher : public UBAbstractPublisher
{
    Q_OBJECT;

    public:
        explicit UBDocumentPublisher(UBDocumentProxy* sourceDocument, QObject *parent = 0);
        virtual ~UBDocumentPublisher();

        void publish();

    protected:

        // not needed as we do not publish svg file anymore
        //virtual void rasterizePDF();
        //virtual void rasterizeSVGImages();
        //virtual void updateSVGForWidget(int sceneIndex);

        virtual void updateGoogleMapApiKey();

        virtual void rasterizeScenes();

        virtual void upgradeDocumentForPublishing();

        virtual void generateWidgetPropertyScript(UBGraphicsW3CWidgetItem *widgetItem, int pageNumber);

        void sendZipToUniboardWeb(const QString& zipFile, const QUuid& publishingUuid);

    private slots:

        void postDocument(const QUuid& tokenUuid, const QString& encryptedBase64Token);
        void uploadProgress(qint64, qint64);
        void postZipUploadResponse(bool, const QByteArray&);

    private:

        UBDocumentProxy *mSourceDocument;
        UBDocumentProxy *mPublishingDocument;

        UBServerXMLHttpRequest* mUploadRequest;

        QString mTitle;
        QString mAuthor;
        QString mDescription;
        QString mEMail;
        bool mAttachPDF;
        bool mAttachUBZ;

        QUrl mPublishingUrl;

        QString mPublishingServiceUrl;
        QUuid mAuthenticationUuid;
        QString mAuthenticationBase64Token;

};


class UBDocumentPublishingDialog : public QDialog, public Ui::documentPublishingDialog
{
    Q_OBJECT;

    public:
        UBDocumentPublishingDialog(QWidget *parent = 0);
        ~UBDocumentPublishingDialog(){}

    private slots:
        void updateUIState(const QString& string);

};

#endif // UBDOCUMENTPUBLISHER_H
