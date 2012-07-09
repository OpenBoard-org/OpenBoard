/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

        bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile);

        UBDocumentProxy* importDir(const QDir& pDir, const QString& pGroup);
        int addImageDirToDocument(const QDir& pDir, UBDocumentProxy* pDocument);

        int addImageAsPageToDocument(const QStringList& images, UBDocumentProxy* document);

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
