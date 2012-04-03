#include "UBExportCFF.h"
#include "UBCFFAdaptor.h"
#include "document/UBDocumentProxy.h"
#include "core/UBDocumentManager.h"
#include "core/UBApplication.h"


UBExportCFF::UBExportCFF(QObject *parent)
: UBExportAdaptor(parent)
{

}

UBExportCFF::~UBExportCFF()
{

}
QString UBExportCFF::exportName()
{
    return tr("Export to IWB");
}

QString UBExportCFF::exportExtention()
{
    return QString(".iwb");
}

void UBExportCFF::persist(UBDocumentProxy* pDocument)
{
    QString src = pDocument->persistencePath();

    if (!pDocument)
        return;

    QString filename = askForFileName(pDocument, tr("Export as IWB File"));

    if (filename.length() > 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        if (mIsVerbose)
            UBApplication::showMessage(tr("Exporting document..."));

            UBCFFAdaptor toIWBExporter;
            if (toIWBExporter.convertUBZToIWB(src, filename))
            {
                if (mIsVerbose)
                    UBApplication::showMessage(tr("Export successful."));
            }
            else 
                if (mIsVerbose)
                    UBApplication::showMessage(tr("Export failed."));


        QApplication::restoreOverrideCursor();

    }

    
}