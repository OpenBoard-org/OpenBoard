
#ifndef UBIMPORTIMAGE_H_
#define UBIMPORTIMAGE_H_

#include <QtGui>
#include "UBImportAdaptor.h"
class UBDocumentProxy;

class UBImportImage : public UBImportAdaptor
{
    Q_OBJECT;

    public:
        UBImportImage(QObject *parent = 0);
        virtual ~UBImportImage();

        virtual QStringList supportedExtentions();
        virtual QString importFileFilter();

        virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);
};

#endif /* UBIMPORTIMAGE_H_ */
