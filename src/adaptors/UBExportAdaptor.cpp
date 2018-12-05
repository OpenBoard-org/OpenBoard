/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
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


#include <QFileDialog>

#include "UBExportAdaptor.h"

#include "document/UBDocumentProxy.h"

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBApplication.h"

#include "gui/UBMainWindow.h"
#include "gui/UBMessagesDialog.h"

#include "core/memcheck.h"

UBExportAdaptor::UBExportAdaptor(QObject *parent)
    : QObject(parent)
    , mIsVerbose(true)
{
    // NOOP
}


UBExportAdaptor::~UBExportAdaptor()
{
    // NOOP
}

QString UBExportAdaptor::askForFileName(UBDocumentProxy* pDocument, const QString& pDialogTitle)
{
    QString defaultName;

    defaultName += pDocument->metaData(UBSettings::documentName).toString() + exportExtention();

    defaultName = UBFileSystemUtils::cleanName(defaultName);

    QString defaultPath = UBSettings::settings()->lastExportFilePath->get().toString() + "/" + defaultName;

    QString filename = QFileDialog::getSaveFileName(UBApplication::mainWindow, pDialogTitle, defaultPath, QString());

    if (filename.size() == 0)
    {
        return filename;
    }

    // add extension if needed
    QFileInfo fileNameInfo(filename);
    if (fileNameInfo.suffix() != exportExtention().mid(1, exportExtention().length() - 1))
    {
        filename += exportExtention();
    }
    UBSettings::settings()->lastExportFilePath->set(QVariant(fileNameInfo.absolutePath()));
    QApplication::processEvents();

    return filename;
}

QString UBExportAdaptor::askForDirName(UBDocumentProxy* pDocument, const QString& pDialogTitle)
{
    QString defaultPath = UBSettings::settings()->lastExportDirPath->get().toString();

    QString container = QFileDialog::getExistingDirectory(UBApplication::mainWindow, pDialogTitle, defaultPath);

    QString dirname;

    if (container.size() > 0)
    {
        UBSettings::settings()->lastExportDirPath->set(QVariant(container));

        QString docname;

        if (pDocument->metaData(UBSettings::documentGroupName).toString().length() > 0)
        {
            docname += pDocument->metaData(UBSettings::documentGroupName).toString() + QString(" ");
        }

        docname += pDocument->metaData(UBSettings::documentName).toString() + exportExtention();
        docname = UBFileSystemUtils::cleanName(docname);

        dirname = container + "/" + docname;
    }

    QApplication::processEvents();

    return dirname;
}

void UBExportAdaptor::persistLocally(UBDocumentProxy* pDocumentProxy, const QString& pDialogTitle)
{
    if (!pDocumentProxy)
        return;

    QString filename = askForFileName(pDocumentProxy, pDialogTitle);

    if (filename.length() > 0) {
        QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

        if (mIsVerbose)
            UBApplication::showMessage(tr("Exporting document..."));

        // Check that the location is writeable
        QFileInfo info(filename);
        info.setFile(info.absolutePath());

        if (!info.isWritable()) {
            QMessageBox errorBox;
            errorBox.setWindowTitle(tr("Export failed"));
            errorBox.setText(tr("Unable to export to the selected location. You do not have the permissions necessary to save the file."));
            errorBox.setIcon(QMessageBox::Critical);
            errorBox.exec();

            if (mIsVerbose)
                UBApplication::showMessage(tr("Export failed: location not writable"));

            QApplication::restoreOverrideCursor();

            return;
        }

        bool persisted = this->persistsDocument(pDocumentProxy, filename);

        if (mIsVerbose && persisted)
            UBApplication::showMessage(tr("Export successful."));

        QApplication::restoreOverrideCursor();
    }
}

bool UBExportAdaptor::persistsDocument(UBDocumentProxy* pDocument, const QString& filename)
{
    // Implemented in child classes

    Q_UNUSED(pDocument);
    Q_UNUSED(filename);
    return false;
}

void UBExportAdaptor::showErrorsList(QList<QString> errorsList)
{
    if (errorsList.count())
    {
        UBMessagesDialog *dialog = new UBMessagesDialog(tr("Warnings during export was appeared"), UBApplication::mainWindow);
        dialog->setMessages(errorsList);
        dialog->show();
    }
}
