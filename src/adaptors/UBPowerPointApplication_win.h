/*
 * UBPowerPointApplication.h
 *
 *  Created on: Dec 4, 2008
 *      Author: Luc
 */

#ifndef UBPOWERPOINTAPPLICATIONWIN_H_
#define UBPOWERPOINTAPPLICATIONWIN_H_

#include <QtCore>
#include "UBImportAdaptor.h"

class UBPowerPointApplication : public UBImportAdaptor {

    Q_OBJECT;

    public:
        UBPowerPointApplication(QObject* parent = 0);
        virtual ~UBPowerPointApplication();

        bool isPowerPointInstalled();

        bool supportPptx();

        virtual QStringList supportedExtentions();
        virtual QString importFileFilter();
        virtual UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup);

        virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);

    private:

        bool generateImagesFromPptFile(const QString& pptFile, const QString&outputDir, const QString& imageFormat, const QSize& imageSize);
        bool generatePdfFromPptFile(const QString& pptFile, const QString& pOutputFile);
        bool catchAndProcessFile();

        void init();
        bool mInit;
        bool mHasException;
        bool mSupportPpt;
        bool mSupportPptX;


    private slots:
        void exception ( int code, const QString & source, const QString & desc, const QString & help);

};

#endif /* UBPOWERPOINTAPPLICATIONWIN_H_ */
