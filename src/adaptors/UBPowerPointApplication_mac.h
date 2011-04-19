
#ifndef UBPOWERPOINTAPPLICATIONMAC_H_
#define UBPOWERPOINTAPPLICATIONMAC_H_

#include <QtCore>
#include "UBImportAdaptor.h"

class UBPowerPointApplication : public UBImportAdaptor
{
    Q_OBJECT

    public:
        UBPowerPointApplication(QObject* parent = 0);
        virtual ~UBPowerPointApplication();

        virtual QStringList supportedExtentions();
        virtual QString importFileFilter();
        virtual UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup);

        virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);

    private:
        bool generatePdfFromPptFile(const QString& pptFile, const QString& pOutputFile);


};

#endif /* UBPOWERPOINTAPPLICATIONMAC_H_ */
