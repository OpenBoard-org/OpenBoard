/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


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

    if (pDocument->metaData(UBSettings::documentGroupName).toString().length() > 0)
    {
        defaultName += pDocument->metaData(UBSettings::documentGroupName).toString() + QString(" ");
    }

    defaultName += pDocument->metaData(UBSettings::documentName).toString() + exportExtention();

    defaultName = UBFileSystemUtils::cleanName(defaultName);
    QString defaultPath = UBSettings::settings()->lastExportFilePath->get().toString() + "/" + defaultName;

    QString filename = QFileDialog::getSaveFileName(UBApplication::mainWindow, pDialogTitle, defaultPath);

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

void UBExportAdaptor::showErrorsList(QList<QString> errorsList)
{
    if (errorsList.count())
    {
        UBMessagesDialog *dialog = new UBMessagesDialog(tr("Warnings during export was appeared"), UBApplication::mainWindow);
        dialog->setMessages(errorsList);
        dialog->show();
    }
}