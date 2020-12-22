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

#include <QDebug>

#include "UBExportDocumentCleanerQPDF.h"

#include <qpdf/QPDF.hh>
#include <qpdf/QPDFWriter.hh>
#include <qpdf/FileInputSource.hh>
#include <qpdf/QPDFPageDocumentHelper.hh>
#include <qpdf/ClosedFileInputSource.hh>
#include <qpdf/QPDFPageObjectHelper.hh>
#include <qpdf/QPDFPageLabelDocumentHelper.hh>
#include <qpdf/QTC.hh>

namespace {
    struct QPDFPageData
    {
        QPDFPageData(std::string const& filename, QPDF* qpdf, int page);
        QPDFPageData(QPDFPageData const& other, int page);

        std::string filename;
        QPDF* qpdf;
        std::vector<QPDFObjectHandle> orig_pages;
        int selected_page;
    };

    QPDFPageData::QPDFPageData(std::string const& filename,
                               QPDF* qpdf,
                               int page) :
        filename(filename),
        qpdf(qpdf),
        orig_pages(qpdf->getAllPages())
    {
        this->selected_page = page;
    }

    QPDFPageData::QPDFPageData(QPDFPageData const& other, int page) :
        filename(other.filename),
        qpdf(other.qpdf),
        orig_pages(other.orig_pages)
    {
        this->selected_page = page;
    }
}

int UBExportDocumentCleanerQPDF::Stripe(QString const &inputFile, QString const &outputFile, QList<int> const &pagesToKeep, const QString &pdfEmptyFileName) throw(std::exception)
{
    // Note: The code below is basically the code from the qpdf.cc command line file, cleaned up. If
    // something misses (e.g. you want to add encrypted pdf), you can probably 'diff' both files and
    // find what's missing.

    QPDF pdfInput;
    FileInputSource* fisInput = new FileInputSource();
    fisInput->setFilename(inputFile.toStdString().c_str());
    PointerHolder<InputSource> isInput = fisInput;
    pdfInput.processInputSource(isInput, "" /* password */);
    QPDFPageDocumentHelper dhInput(pdfInput);
    int const totalPages = static_cast<int>(dhInput.getAllPages().size());
    if (totalPages == pagesToKeep.size())
    {
        // Nothing to stripe, all pages are kept.
        return 0;
    }

    if (totalPages < pagesToKeep.size()) {
        // More pages to keep than available, something must be wrong.
        return -1;
    }

    if (totalPages <= 0) {
        // No pages there, something must be wrong.
        return -1;
    }

    QPDF pdfOutput;
    QPDFPageDocumentHelper dhOutput(pdfOutput);
    pdfOutput.emptyPDF();

    QPDF pdfBlankPage;
    FileInputSource* fisBlankPage = new FileInputSource();
    fisBlankPage->setFilename(pdfEmptyFileName.toStdString().c_str());
    PointerHolder<InputSource> isBlankPage = fisBlankPage;
    pdfBlankPage.processInputSource(isBlankPage, "" /* password */);
    QPDFPageDocumentHelper dhBlank(pdfBlankPage);
    Q_ASSERT(dhBlank.getAllPages().size() == 1);
    std::map<std::string, QPDF*> page_spec_qpdfs;

    page_spec_qpdfs[pdfEmptyFileName.toStdString()] = &pdfBlankPage;
    page_spec_qpdfs[inputFile.toStdString()] = &pdfInput;

    std::vector<QPDFPageObjectHelper> orig_pages;
    std::vector<QPDFPageData> parsed_specs;

    // Create a QPDF object for each file that we may take pages from.
    std::map<std::string, ClosedFileInputSource*> page_spec_cfis;

    for (int i = 1; i <= totalPages; i++)
    {
        std::string filename;
        int page = -1;
        if (pagesToKeep.indexOf(i) == -1)
        {
            // The empty.pdf has only 1 page. We need to insert it as many times as required.
            filename = pdfEmptyFileName.toStdString();
            page = 1;
        } else {
            filename = inputFile.toStdString();
            page = i;
        }

        // Read original pages from the PDF, and parse the page range
        // associated with this occurrence of the file.
        parsed_specs.push_back(
            QPDFPageData(filename,
                         page_spec_qpdfs[filename],
                         page));
    }

    // Remove unused references.
    for (std::map<std::string, QPDF*>::iterator iter =
             page_spec_qpdfs.begin();
         iter != page_spec_qpdfs.end(); ++iter)
    {
        std::string const& filename = (*iter).first;
        ClosedFileInputSource* cis = 0;
        if (page_spec_cfis.count(filename))
        {
            cis = page_spec_cfis[filename];
            cis->stayOpen(true);
        }
        QPDF& other(*((*iter).second));

        QPDFPageDocumentHelper dh(other);
        dh.removeUnreferencedResources();
        if (cis)
        {
            cis->stayOpen(false);
        }
    }

    std::map<unsigned long long, std::set<QPDFObjGen> > copied_pages;

    // Add all the pages from all the files in the order specified.
    // Keep track of any pages from the original file that we are
    // selecting.
    std::set<int> selected_from_orig;
    std::vector<QPDFObjectHandle> new_labels;
    bool any_page_labels = false;
    int out_pageno = 0;

    for (std::vector<QPDFPageData>::iterator iter =
             parsed_specs.begin();
         iter != parsed_specs.end(); ++iter)
    {
        QPDFPageData& page_data = *iter;
        ClosedFileInputSource* cis = 0;
        if (page_spec_cfis.count(page_data.filename))
        {
            cis = page_spec_cfis[page_data.filename];
            cis->stayOpen(true);
        }
        QPDFPageLabelDocumentHelper pldh(*page_data.qpdf);
        if (pldh.hasPageLabels())
        {
            any_page_labels = true;
        }

        /* qDebug() << whoami << ": adding pages from "
                  << page_data.filename.c_str(); */

        // Pages are specified from 1 but numbered from 0 in the
        // vector
        int pageno = page_data.selected_page - 1;
        Q_ASSERT(pageno >= 0 && pageno < totalPages);

        pldh.getLabelsForPageRange(pageno, pageno, out_pageno,
                                   new_labels);
        ++out_pageno;

        QPDFPageObjectHelper to_copy =
            page_data.orig_pages.at(QIntC::to_size(pageno));
        QPDFObjGen to_copy_og = to_copy.getObjectHandle().getObjGen();
        unsigned long long from_uuid = page_data.qpdf->getUniqueId();
        if (copied_pages[from_uuid].count(to_copy_og))
        {
            QTC::TC("qpdf", "qpdf copy same page more than once",
                    (page_data.qpdf == &pdfOutput) ? 0 : 1);
            to_copy = to_copy.shallowCopyPage();
        }
        else
        {
            copied_pages[from_uuid].insert(to_copy_og);
        }
        dhOutput.addPage(to_copy, false);
        if (page_data.qpdf == &pdfOutput)
        {
            // This is a page from the original file. Keep track
            // of the fact that we are using it.
            selected_from_orig.insert(pageno);
        }

        if (cis)
        {
            cis->stayOpen(false);
        }
    }
    if (any_page_labels)
    {
        QPDFObjectHandle page_labels =
            QPDFObjectHandle::newDictionary();
        page_labels.replaceKey(
            "/Nums", QPDFObjectHandle::newArray(new_labels));
        pdfOutput.getRoot().replaceKey("/PageLabels", page_labels);
    }

    // Delete page objects for unused page in primary. This prevents
    // those objects from being preserved by being referred to from
    // other places, such as the outlines dictionary.
    for (size_t pageno = 0; pageno < orig_pages.size(); ++pageno)
    {
        if (selected_from_orig.count(QIntC::to_int(pageno)) == 0)
        {
            pdfOutput.replaceObject(
                orig_pages.at(pageno).getObjectHandle().getObjGen(),
                QPDFObjectHandle::newNull());
        }
    }

    QByteArray fileName = outputFile.toStdString().c_str();
    QPDFWriter w(pdfOutput, fileName.constData());
    w.write();

    return 0; // No exception thrown by qpdf so far, everything must be fine.
}

