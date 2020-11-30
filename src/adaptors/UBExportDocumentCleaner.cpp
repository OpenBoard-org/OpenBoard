/*
 * Copyright (C) 2015-2020 Département de l'Instruction Publique (DIP-SEM)
 *
 */

#include <QFile>
#include <QDebug>
#include <QProcess>

#include "UBExportDocumentCleaner.h"

UBExportDocumentCleaner::UBExportDocumentCleaner()
{

}

bool UBExportDocumentCleaner::stripePdf(QString const &file, QList<int> const &pagesToKeep)
{
    QString const tempName = file+"_temp";
    QFile::rename(file, tempName);

    // Build a string list with all required pages.
    QString pages;
    foreach (int page, pagesToKeep)
    {
        if (pages.size() > 0)
            pages += ",";
        pages += QString::number(page);
    }

    // Try the 'release' config first. The binary is either side by side of OB, or in the system path.
    QString command = QString("%1 %2 --pages %2 %3 -- %4").arg(QPDF_BINARY).arg(tempName).arg(pages).arg(file);
    int result = QProcess::execute(command);
    if (result != 0)
    {
        // 'qpdf' not on the path? Then try for a debug configuration, so it ease debugging.
        command = QString("%1/%2 %3 --pages %3 %4 -- %5").arg(QPDF_DEBUG_BIN_DIR).arg(QPDF_BINARY).arg(tempName).arg(pages).arg(file);
        result = QProcess::execute(command);
    }

    if (result != 0) {
        // Can't stripe? Recover the original file, then display a warning.
        QFile::rename(tempName, file);
    }

    QFile::remove(tempName);

    return (result == 0);
}
