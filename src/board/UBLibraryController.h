#ifndef UBLIBRARYCONTROLLER_H_
#define UBLIBRARYCONTROLLER_H_

#include <QtGui>
#include <QtWebKit>

#include "web/UBWebPage.h"

#include "ui_library.h"

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

        QWidget* libraryWindow()
        {
            return mLibraryWindow;
        }

        static QStringList onlineLibraries();
        static void preloadFirstOnlineLibrary();

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

    signals:
        void dialogClosed(int state);
        void setResource(QString &pathResource,QString &mimetype);

    public slots:
        void removeBackground();


        void showLibraryDialog(bool show);

        void refreshShapeThumbnailsView();
        void refreshImageThumbnailsView();
        void refreshInteractiveThumbnailsView();
        void refreshVideoThumbnailsView();
        void refreshSoundThumbnailsView();

        void addShape();
        void setShapeAsBackground();

        void addToPage();
        void setAsBackground();


        void addImage();
        void addVideo();
        void addAudio();
        void addInteractiveToCurrentPage();

        void addImagesToCurrentPage(const QList<QUrl>& images);
        void addVideosToCurrentPage(const QList<QUrl>& videos);
        void addAudiosToCurrentPage(const QList<QUrl>& sounds);
        void addInteractivesToCurrentPage(const QList<QUrl>& interactiveWidgets);
        void setImageAsBackground();

        void closeWindow();

        void addObjectFromFilesystemToPage();

        void needRefreshOnNextDisplay()
        {
                mNeedRefreshOnNextDisplay = true;
        }

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

        QStringList addItemsToCurrentLibrary(const QDir& pSelectedFolder, const QStringList& pExtensions);

        UBLibElement* isOnFavoriteList(UBLibElement * element);

        void loadLibraries();

        QWidget *mParentWidget;
        UBBoardController *mBoardController;
        QDialog *mLibraryWindow;
        Ui::library *mLibraryUI;

        // TODO UB 4.x break this logic, by carrying the path within the thumb item (see Video items path)
        //
        QMap<QGraphicsSvgItem*, QString> mSvgItemToFilepath; //shape mapping
        QMap<QGraphicsSvgItem*, QString> mSvgImageItemToFilepath; // svg image mapping
        QMap<QGraphicsPixmapItem*, QString> mPixmapItemToFilepath; // other image mapping
        QMap<QGraphicsPixmapItem*, QString> mInteractiveItemToFilepath; // interactive widget mapping
        QMap<QGraphicsPixmapItem*, QString> mSoundItemToFilepath; // sounds mapping

        UBLibraryWebView *mImageWebView;
        UBLibraryWebView *mVideoWebView;
        UBLibraryWebView *mInteractiveWebView;

        QTreeWidgetItem *mImageOnlineTi;
        QTreeWidgetItem *mVideoOnlineTi;
        QTreeWidgetItem *mInteractiveOnlineTi;
        QTreeWidgetItem *mInteractiveUniboardTi;

        struct TabIndex
        {
            enum Enum
            {
                Gip = 0,
                Interactive = 1,
                Image = 2,
                Video = 3,
                Shape = 4,
                Sound = 5
            };
        };

        int mLastItemOffsetIndex;

        QStringList mLibraryFileToDownload;

        bool mNeedRefreshOnNextDisplay;


    private slots:

        void tabChanged(int value);

        void zoomSliderValueChanged(int value);

        void createNewFolder();
        void addInteractivesToLibrary();
        void addImagesToLibrary();

        void removeItemsFromLibrary(UBThumbnailWidget* pThumbnailView);
        void removeDir();
        void remove();

        void itemSelectionChanged();
        void itemChanged(QTreeWidgetItem * item, int column);

        void selectionChanged();

        void thumbnailViewResized();

        void getLibraryListResponse(bool, const QByteArray&);

};


class UBLibraryFolderItem : public QTreeWidgetItem
{

    public:

        UBLibraryFolderItem(const QDir& pDir, const QString& name, QTreeWidgetItem * parent,
            bool pCanWrite, const QStringList& pExtensionsToHide);

        virtual ~UBLibraryFolderItem(){}

        void refreshSubDirs();

        QDir dir()
        {
            return mDir;
        }

        void setDir(const QDir& dir)
        {
            mDir = dir;
        }

        bool canWrite()
        {
            return mCanWrite;
        }

        void setCanWrite(bool canWrite)
        {
            mCanWrite = canWrite;
        }

    private:
        const QStringList mExtensionsToHide;
        QDir mDir;
        bool mCanWrite;
};


class UBOnlineLibraryItem : public QTreeWidgetItem
{

    public:

        UBOnlineLibraryItem(const QUrl& pUrl, const QString& name, QTreeWidgetItem * parent)
            : QTreeWidgetItem(parent)
            , mUrl(pUrl)
        {
            setText(0, UBLibraryController::trUtf8(name.toUtf8()));
            setIcon(0, QWebSettings::iconForUrl(pUrl));
        }

        virtual ~UBOnlineLibraryItem(){}

        QUrl url() const
        {
            return mUrl;
        }

    private:

        QUrl mUrl;
};


class UBLibraryWebView : public QWebView
{
    Q_OBJECT;

    public:
        UBLibraryWebView(QWidget * parent = 0 );
        virtual ~UBLibraryWebView(){};

        virtual void mousePressEvent ( QMouseEvent * event)
        {
            QWebView::mousePressEvent(event);
        }

        virtual void mouseMoveEvent ( QMouseEvent * event)
        {
            QWebView::mouseMoveEvent(event);
        }

        virtual void mouseReleaseEvent ( QMouseEvent * event)
        {
            QWebView::mouseReleaseEvent(event);
        }

        void load ( const QUrl & url )
        {
            mCurrentLibraryItem = 0;
            QWebView::load(url);
        }

        void load ( const QUrl & url, UBOnlineLibraryItem* pLibraryItem)
        {
            mCurrentLibraryItem = pLibraryItem;
            QWebView::load(url);
        }

    protected:

        virtual QWebView * createWindow(QWebPage::WebWindowType type);

    private slots:

        void javaScriptWindowObjectCleared();

        void newIconAvailable();

        void loadFinished(bool ok);

    private:
        UBOnlineLibraryItem* mCurrentLibraryItem;


};


class UBLibraryPreloader : public QObject
{
    Q_OBJECT;

    public:
        UBLibraryPreloader(QObject* pParent);
        virtual ~UBLibraryPreloader(){};

    private slots:

        void loadLibrary();

        void getLibraryListResponse(bool ok, const QByteArray& replyContent);

        void loadFinished (bool ok);

    private:
        QWebView *mWebView;
};



#endif /* UBLIBRARYCONTROLLER_H_ */
