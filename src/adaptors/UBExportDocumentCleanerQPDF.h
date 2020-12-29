/*
 * Copyright (C) 2015-2020 Département de l'Instruction Publique (DIP-SEM)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */


#ifndef UBEXPORTDOCUMENTCLEANERQPDF_H
#define UBEXPORTDOCUMENTCLEANERQPDF_H

#include <QString>
#include <QList>

class UBExportDocumentCleanerQPDF {
public:
    //! Read 'inputFile', remove all unwanted pages, which are replaced by the page 1 of 'pdfEmptyFileName', and save it to 'outputFile'.
    //! \return 0 if success, -1 otherwise.
    //! Note it might throw an exception due to the 'qpdf' underlying layer, so any exception caught has to be interpreted as a failure.
    static int Stripe(QString const &inputFile, const QString &outputFile, QList<int> const &pagesToKeep, QString const &pdfEmptyFileName) /* throw(std::exception) */;
private:
    //! Make sure this static class is not instanciated.
    UBExportDocumentCleanerQPDF() {}
};

#endif //UBEXPORTDOCUMENTCLEANERQPDF_H
