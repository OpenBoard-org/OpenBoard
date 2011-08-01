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
    ~UBLibElement();

    static UBLibElement* trashElement();

    eUBLibElementType type(){return mType;}
    void setType(eUBLibElementType type) {mType = type;}
    QUrl path(){return mPath;}
    void setPath(QUrl path){mPath = path;}
    QImage* thumbnail(){return &mThumbnail;}
    void setThumbnail(QImage* pThumb){mThumbnail = *pThumb;}
    QString information(){return mInfo;}
    void setInformation(QString info){mInfo = info;}
    QString name(){return mName;}
    void setName(QString name){mName = name;}
    QString extension(){return mExtension;}
    void setExtension(QString &extension){ mExtension = extension;}
    bool isMoveable(){return mbMoveable;}
    void setMoveable(bool bState){mbMoveable = bState;}

private:
    eUBLibElementType mType;
    QUrl mPath;
    QImage mThumbnail;
    QString mInfo;
    QString mName;
    QString mExtension;
    bool mbMoveable;
};

class UBChainedLibElement
{
public:
    UBChainedLibElement(UBLibElement* pElem, UBChainedLibElement* pNextElem=NULL);
    ~UBChainedLibElement();

    UBChainedLibElement* nextElement(){return mpNextElem;}
    void setNextElement(UBChainedLibElement* nextElem);
    UBLibElement* element(){return mpElem;}

private:
    UBLibElement* mpElem;
    UBChainedLibElement* mpNextElem;
};

class UBLibraryController : public QObject
{
    Q_OBJECT;

    public:
        UBLibraryController(QWidget *parentWidget, UBBoardController *boardController);
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

    private:
        QList<UBLibElement*> rootCategoriesList();
        QList<UBLibElement*> listElementsInPath(const QString& pPath);
        QList<UBLibElement*> listElementsInVirtualForlder(UBLibElement* pElement);
        void userPath(QUrl &pPath);
        QImage* thumbnailForFile(UBLibElement* pPath);
        QImage* createThumbnail(UBLibElement* pPath);
        QList<UBLibElement*> addVirtualElementsForItemPath(const QString& pPath);

        void createInternalWidgetItems();
        void routeItem(QString& pItem, QString pMiddleDirectory = QString());
        void createDirectory(QUrl& pDirPath);

        QUrl mAudioStandardDirectoryPath;
        QUrl mVideoStandardDirectoryPath;
        QUrl mPicturesStandardDirectoryPath;
        QUrl mInteractiveUserDirectoryPath;
        QUrl mInteractiveCategoryPath;
        QUrl mAnimationUserDirectoryPath;

        QStringList addItemsToCurrentLibrary(const QDir& pSelectedFolder, const QStringList& pExtensions);

        UBLibElement* isOnFavoriteList(UBLibElement * element);

        QWidget *mParentWidget;
        UBBoardController *mBoardController;

        int mLastItemOffsetIndex;

};

#endif /* UBLIBRARYCONTROLLER_H_ */
