/*
 * UBExportWeb
 *
 *  Created on: Feb 19, 2009
 *      Author: Luc
 */

#include "UBExportWeb.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBDocumentManager.h"
#include "core/UBApplication.h"

#include "document/UBDocumentProxy.h"

#include "quazip.h"
#include "quazipfile.h"

#include "core/memcheck.h"

UBExportWeb::UBExportWeb(QObject *parent)
    : UBExportAdaptor(parent)
{
    UBExportWeb::tr("Page"); // dummy slot for translation
}


UBExportWeb::~UBExportWeb()
{
    // NOOP
}


void UBExportWeb::persist(UBDocumentProxy* pDocumentProxy)
{
    if (!pDocumentProxy)
        return;

    QString dirName = askForDirName(pDocumentProxy, tr("Export as Web data"));

    if (dirName.length() > 0)
    {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
        UBApplication::showMessage(tr("Exporting document..."));

        if(UBFileSystemUtils::copyDir(pDocumentProxy->persistencePath(), dirName))
        {
            QString htmlPath = dirName + "/index.html";

            QFile html(":www/uniboard-web-player.html");
            html.copy(htmlPath);

            UBApplication::showMessage(tr("Export successful."));

            QDesktopServices::openUrl(QUrl::fromLocalFile(htmlPath));
        }
        else
        {
            UBApplication::showMessage(tr("Export failed."));
        }


        QApplication::restoreOverrideCursor();
    }
}


QString UBExportWeb::exportName()
{
    return tr("Export to Web Browser");
}
