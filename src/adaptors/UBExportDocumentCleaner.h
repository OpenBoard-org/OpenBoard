/*
 * Copyright (C) 2015-2020 Département de l'Instruction Publique (DIP-SEM)
 *
 */

#ifndef UBEXPORTDOCUMENTCLEANER_H
#define UBEXPORTDOCUMENTCLEANER_H

#include <QString>

class UBExportDocumentCleaner
{
public:
    UBExportDocumentCleaner();

    void stripePdf(QString const &file, QList<int> const &pagesToKeep);
private:

};

#endif // UBEXPORTDOCUMENTCLEANER_H
