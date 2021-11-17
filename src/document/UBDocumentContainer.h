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




#ifndef UBDOCUMENTCONTAINER_H_
#define UBDOCUMENTCONTAINER_H_

#include <QtGui>
#include "UBDocumentProxy.h"

class UBDocumentContainer : public QObject
{
    Q_OBJECT

    public:
        UBDocumentContainer(QObject * parent = 0);
        virtual ~UBDocumentContainer();

        void setDocument(UBDocumentProxy* document, bool forceReload = false);
        void pureSetDocument(UBDocumentProxy *document) {mCurrentDocument = document;}

        UBDocumentProxy* selectedDocument(){return mCurrentDocument;}
        QList<std::shared_ptr<QPixmap>>& documentThumbs() { return mDocumentThumbs; }
        int pageCount() const{return mCurrentDocument->pageCount();}
        std::shared_ptr<QPixmap> pageAt(int index)
        {
            if (index < mDocumentThumbs.size())
                return mDocumentThumbs[index];
            else
            {
                return NULL;
            }
        }

        static int pageFromSceneIndex(int sceneIndex);
        static int sceneIndexFromPage(int sceneIndex);

        void duplicatePages(QList<int>& pageIndexes);
        void deletePages(QList<int>& pageIndexes);


        void addPage(int index);
        void addPixmapAt(std::shared_ptr<QPixmap> pix, int index);

        virtual void reloadThumbnails();

        void clearThumbPage();
        void initThumbPage();
        void insertExistingThumbPage(int index, std::shared_ptr<QPixmap> thumbnailPixmap);
        void insertThumbPage(int index);
        void addEmptyThumbPage();
        void deleteThumbPage(int index);
        void updateThumbPage(int index);
        void moveThumbPage(int source, int target);

    private:
        UBDocumentProxy* mCurrentDocument;
        QList<std::shared_ptr<QPixmap>>  mDocumentThumbs;

    signals:
        void documentSet(UBDocumentProxy* document);
        void documentPageInserted(int index);
        void documentPageUpdated(int index);
        void documentPageRemoved(int index);
        void documentPageMoved(int from, int to);
        void documentThumbnailsUpdated(UBDocumentContainer* source);

        void initThumbnailsRequired(UBDocumentContainer* source);
        void addThumbnailRequired(UBDocumentContainer* source, int index);
        void removeThumbnailRequired(int index);
        void moveThumbnailRequired(int from, int to);
        void updateThumbnailsRequired();
};


#endif /* UBDOCUMENTPROXY_H_ */
