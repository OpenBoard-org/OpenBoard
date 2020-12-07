/*
 * Copyright (C) 2015-2020 Département de l'Instruction Publique (DIP-SEM)
 *
 */

#ifndef UBEXPORTDOCUMENTCLEANER_H
#define UBEXPORTDOCUMENTCLEANER_H

#include <QString>

class UBDocumentProxy;

class UBExportDocumentCleaner
{
public:
    UBExportDocumentCleaner();

    static bool StripeDocument(UBDocumentProxy* pDocumentProxy, const QString &filename);

private:
    //! Return 'true' is success, 'false' otherwise. Note the final archive is
    //! always available, but in case of failure, it is not stripped.
    static bool StripePdf(QString const &file, QList<int> const &pagesToKeep);
};

#endif // UBEXPORTDOCUMENTCLEANER_H
