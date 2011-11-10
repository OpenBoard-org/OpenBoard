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
#ifndef UBLIBRARYCONTROLLER_H_
#define UBLIBRARYCONTROLLER_H_

#include <QtGui>
#include <QtWebKit>

#include "web/UBWebPage.h"

class UBGraphicsScene;
class UBBoardController;
class QGraphicsSvgItem;
class UBLibraryWebView;
class UBWebView;

typedef enum
{
    eUBLibElementType_Category = 0,
    eUBLibElementType_VirtualFolder,
    eUBLibElementType_Folder,
    eUBLibElementType_InteractiveItem,
    eUBLibElementType_Item
}eUBLibElementType;


class UBLibElement
{

public:
    UBLibElement();
    UBLibElement(eUBLibElementType type, const QUrl& path, const QString& name);
    UBLibElement(UBLibElement* element);
    ~UBLibElement();

    static UBLibElement* trashElement();

    eUBLibElementType type(){return mType;}
    void setType(eUBLibElementType type) {mType = type;}
    QUrl path(){return mPath;}
    void setPath(QUrl path){mPath = path;}
    QImage* thumbnail(){return &mThumbnail;}
    void setThumbnail(QImage pThumb){mThumbnail = pThumb;}
    QString information(){return mInfo;}
    void setInformation(QString info){mInfo = info;}
    QString name(){return mName;}
    void setName(QString name){mName = name;}
    QString extension(){return mExtension;}
    void setExtension(QString &extension){ mExtension = extension;}
    bool isMoveable(){return mbMoveable;}
    void setMoveable(bool bState){mbMoveable = bState;}
    bool isDeletable() const {return mbDeletable;}
    void setDeletable(bool mState) {mbDeletable = mState;}

private:
    eUBLibElementType mType;
    QUrl mPath;
    QImage mThumbnail;
    QString mInfo;
    QString mName;
    QString mExtension;
    bool mbMoveable;
    bool mbDeletable;
};

class UBChainedLibElement
{
public:
    UBChainedLibElement(UBLibElement* pElem, UBChainedLibElement* pNextElem=NULL);
    ~UBChainedLibElement();

    UBChainedLibElement* nextElement(){return mpNextElem;}
    UBChainedLibElement* lastElement();
    void setNextElement(UBChainedLibElement* nextElem);
    UBLibElement* element(){return mpElem;}
    QUrl lastItemPath();

private:
    UBLibElement* mpElem;
    UBChainedLibElement* mpNextElem;
};

class UBLibraryController : public QObject
{
    Q_OBJECT;

    public:
        UBLibraryController(QWidget *parentWidget);
        virtual ~UBLibraryController();

        QList<UBLibElement*> getContent(UBLibElement* pElement);
        void moveContent(QList<UBLibElement*> sourceList, UBLibElement *pDestination);
        void trashElements(QList<UBLibElement*> trashList);
        void emptyElementsOnTrash(QList<UBLibElement*> elementsList);

        void addNativeToolToFavorites(const QUrl& url);

        void setItemAsBackground(UBLibElement* image);
        void addItemToPage(UBLibElement* item);

        void addToFavorite(QList<UBLibElement*> elementList);
        void removeFromFavorite(QList<UBLibElement*> elementList);

        void importItemOnLibrary(QString& pItemString);
        void importImageOnLibrary(QImage &pImage);

        QString favoritePath();

        void createNewFolder(QString name, UBLibElement* parentElem);
        bool canItemsOnElementBeDeleted(UBLibElement *pElement);
        void routeItem(QString& pItem, QString pMiddleDirectory = QString());

    signals:
        void dialogClosed(int state);
        void setResource(QString &pathResource,QString &mimetype);

    public slots:
        void removeBackground();
        void addImagesToCurrentPage(const QList<QUrl>& images);
        void addVideosToCurrentPage(const QList<QUrl>& videos);
        void addAudiosToCurrentPage(const QList<QUrl>& sounds);
        void addInteractivesToCurrentPage(const QList<QUrl>& interactiveWidgets);

    protected:

        UBGraphicsScene* activeScene();
        QRectF visibleSceneRect();
        QList<UBLibElement*> mFavoriteList;
        void persistFavoriteList();
        void readFavoriteList();
        QList<UBLibElement*> mInternalLibElements;
        QList<UBLibElement*> mElementsList;
        void cleanElementsList();

    private:
        QList<UBLibElement*> rootCategoriesList();
        QList<UBLibElement*> listElementsInPath(const QString& pPath);
        QList<UBLibElement*> listElementsInVirtualForlder(UBLibElement* pElement);
        void userPath(QUrl &pPath);
        QImage thumbnailForFile(UBLibElement* pPath);
        QImage createThumbnail(UBLibElement* pPath);
        QList<UBLibElement*> addVirtualElementsForItemPath(const QString& pPath);

        void createInternalWidgetItems();
        void createDirectory(QUrl& pDirPath);

        QUrl mAudioStandardDirectoryPath;
        QUrl mVideoStandardDirectoryPath;
        QUrl mPicturesStandardDirectoryPath;
        QUrl mInteractiveUserDirectoryPath;
        QUrl mInteractiveCategoryPath;
        QUrl mAnimationUserDirectoryPath;
        QUrl mSearchCategoryPath;

        QStringList addItemsToCurrentLibrary(const QDir& pSelectedFolder, const QStringList& pExtensions);

        UBLibElement* isOnFavoriteList(UBLibElement * element);

        QWidget *mParentWidget;
        UBBoardController *mBoardController;

        int mLastItemOffsetIndex;

};

#endif /* UBLIBRARYCONTROLLER_H_ */
