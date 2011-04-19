/*
 * UBDocumentManager.h
 *
 *  Created on: Feb 10, 2009
 *      Author: julienbachmann
 */

#ifndef UBDOCUMENTMANAGER_H_
#define UBDOCUMENTMANAGER_H_

#include <QtCore>

class UBExportAdaptor;
class UBImportAdaptor;
class UBDocumentProxy;


class UBDocumentManager : public QObject
{
    Q_OBJECT;

    public:
        static UBDocumentManager* documentManager();
        virtual ~UBDocumentManager();

        QString importFileFilter();
        QStringList importFileExtensions();

        UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup);

        bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);

        UBDocumentProxy* importDir(const QDir& pDir, const QString& pGroup);
        int addImageDirToDocument(const QDir& pDir, UBDocumentProxy* pDocument);

        int addImageAsPageToDocument(const QStringList& images, UBDocumentProxy* document);

        QList<UBExportAdaptor*> supportedExportAdaptors();
        void emitDocumentUpdated(UBDocumentProxy* pDocument);

    signals:
        void documentUpdated(UBDocumentProxy *pDocument);

    private:
        UBDocumentManager(QObject *parent = 0);
        QList<UBExportAdaptor*> mExportAdaptors;
        QList<UBImportAdaptor*> mImportAdaptors;

        static UBDocumentManager* sDocumentManager;
};

#endif /* UBDOCUMENTMANAGER_H_ */
