#ifndef UBIMPORTCFF_H
#define UBIMPORTCFF_H

#include <QtGui>
#include "UBImportAdaptor.h"

class UBDocumentProxy;

class UBImportCFF : public UBImportAdaptor
{
    Q_OBJECT;

    public:
        UBImportCFF(QObject *parent = 0);
        virtual ~UBImportCFF();

        virtual QStringList supportedExtentions();
        virtual QString importFileFilter();

        virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);

        //base class method override
        virtual UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup);

    private:

        virtual QString expandFileToDir(const QFile& pZipFile, const QString& pDir);
};

#endif // UBIMPORTCFF_H
