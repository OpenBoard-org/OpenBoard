
#ifndef UBIMPORTPDF_H_
#define UBIMPORTPDF_H_

#include <QtGui>
#include "UBImportAdaptor.h"
class UBDocumentProxy;

class UBImportPDF : public UBImportAdaptor
{
    Q_OBJECT;

    public:
        UBImportPDF(QObject *parent = 0);
        virtual ~UBImportPDF();

        virtual QStringList supportedExtentions();
        virtual QString importFileFilter();

        virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);
};

#endif /* UBIMPORTPDF_H_ */
