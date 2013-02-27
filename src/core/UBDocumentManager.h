/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBDOCUMENTMANAGER_H_
#define UBDOCUMENTMANAGER_H_

#include <QtCore>

class UBExportAdaptor;
class UBImportAdaptor;
class UBDocumentProxy;


class UBDocumentManager : public QObject
{
    Q_OBJECT;

    public:
        static UBDocumentManager* documentManager();
        virtual ~UBDocumentManager();

        QString importFileFilter();
        QStringList importFileExtensions();

        UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup);

        int addFilesToDocument(UBDocumentProxy* pDocument, QStringList fileNames);

        UBDocumentProxy* importDir(const QDir& pDir, const QString& pGroup);
        int addImageDirToDocument(const QDir& pDir, UBDocumentProxy* pDocument);

        QList<UBExportAdaptor*> supportedExportAdaptors();
        void emitDocumentUpdated(UBDocumentProxy* pDocument);

    signals:
        void documentUpdated(UBDocumentProxy *pDocument);

    private:
        UBDocumentManager(QObject *parent = 0);
        QList<UBExportAdaptor*> mExportAdaptors;
        QList<UBImportAdaptor*> mImportAdaptors;

        static UBDocumentManager* sDocumentManager;
};

#endif /* UBDOCUMENTMANAGER_H_ */
