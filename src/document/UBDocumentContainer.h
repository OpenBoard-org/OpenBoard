/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

    private:
        UBDocumentProxy* mCurrentDocument;
        QList<const QPixmap*>  mDocumentThumbs;

   
    protected:
        void deleteThumbPage(int index);
        void updateThumbPage(int index);
        void insertThumbPage(int index);
        void reloadThumbnails();

    signals:
        void documentSet(UBDocumentProxy* document);
        void documentPageUpdated(int index);
        void documentThumbnailsUpdated(UBDocumentContainer* source);
};


#endif /* UBDOCUMENTPROXY_H_ */
