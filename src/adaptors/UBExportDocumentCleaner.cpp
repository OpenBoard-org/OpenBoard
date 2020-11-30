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

void UBExportDocumentCleaner::stripePdf(QString const &file, QList<int> const &pagesToKeep)
{
    QString const tempName = file+"_temp";
    QFile::rename(file, tempName);

#if 0
    // Future qpdf call here.
    QString pages;
    foreach (int page, pagesToKeep)
    {
        if (pages.size() > 0)
            pages += ",";
        pages += QString::number(page);
    }

    // Successfully tested on Windows.
    QString command = QString("qpdf.exe %1 --pages %1 %2 -- %3").arg(tempName).arg(pages).arg(file);
    qDebug() << "UBExportDocumentCleaner::stripePdf command " << command;
    QProcess::execute(command);
#else
    Q_UNUSED(pagesToKeep)
    // NOOP, we rename the original file back.
    QFile::rename(tempName, file);
#endif

    QFile::remove(tempName);
}
