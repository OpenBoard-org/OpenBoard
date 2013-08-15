/*
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

        UBDocumentProxy* selectedDocument(){return mCurrentDocument;}
        int pageCount(){return mDocumentThumbs.size();}
        const QPixmap* pageAt(int index){return mDocumentThumbs[index];}

        static int pageFromSceneIndex(int sceneIndex);
        static int sceneIndexFromPage(int sceneIndex);

        void duplicatePages(QList<int>& pageIndexes);
        bool movePageToIndex(int source, int target);
        void deletePages(QList<int>& pageIndexes);
        void addPage(int index);
        void updatePage(int index);
        void addEmptyThumbPage();

        void insertThumbPage(int index);


    private:
        UBDocumentProxy* mCurrentDocument;
        QList<const QPixmap*>  mDocumentThumbs;


    protected:
        void deleteThumbPage(int index);
        void updateThumbPage(int index);
        void reloadThumbnails();

    signals:
        void documentSet(UBDocumentProxy* document);
        void documentPageUpdated(int index);
        void documentThumbnailsUpdated(UBDocumentContainer* source);
};


#endif /* UBDOCUMENTPROXY_H_ */
