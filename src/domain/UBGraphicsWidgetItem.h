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
#ifndef UBGRAPHICSWIDGETITEM_H
#define UBGRAPHICSWIDGETITEM_H

#include <QtGui>
#include <QtWebKit>

#include "core/UB.h"

#include "UBGraphicsWebView.h"
#include "UBW3CWidget.h"
#include "UBAppleWidget.h"

class UBWidgetUniboardAPI;
class UBGraphicsScene;
class UBAbstractWidget;
class UBW3CWidgetAPI;
class UBW3CWidgetWebStorageAPI;
class UBGraphiscItem;
class UBGraphiscItemDelegate;

class UBGraphicsWidgetItem : public UBGraphicsWebView
{
    Q_OBJECT

    public:
        UBGraphicsWidgetItem(QGraphicsItem *parent = 0, int widgetType = 0);
        ~UBGraphicsWidgetItem();

        virtual UBGraphicsScene* scene();

        virtual void initialize();

        virtual UBItem* deepCopy() const = 0;

        /* preferences */
        void setPreference(const QString& key, QString value);
        QString preference(const QString& key) const;
        QMap<QString, QString> preferences() const;
        void removePreference(const QString& key);
        void removeAllPreferences();

        /* datastore */

        void setDatastoreEntry(const QString& key, QString value);
        QString datastoreEntry(const QString& key) const;
        QMap<QString, QString> datastoreEntries() const;
        void removeDatastoreEntry(const QString& key);
        void removeAllDatastoreEntries();

        virtual UBGraphicsItemDelegate* Delegate() const {return mDelegate; }

        virtual void remove();
        void removeScript();

        QString downloadUrl(const QString &fileUrl, const QString &extention);
        QString downloadWeb(const QString &fileUrl);
        void processDropEvent(QDropEvent *event);
        bool isDropableData(const QMimeData *data) const;

        virtual void setOwnFolder(const QUrl &newFolder) {ownFolder = newFolder;}
        virtual QUrl getOwnFolder() const {return ownFolder;}
        virtual void setSnapshotPath(const QUrl &newFilePath){SnapshotFile = newFilePath;}
        virtual QUrl getSnapshotPath(){return SnapshotFile;}

        virtual void clearSource();
        virtual void setUuid(const QUuid &pUuid);

    /* from UBAbstractWidget */
        void loadMainHtml();

        QUrl mainHtml()
        {
            return mMainHtmlUrl;
        }

        QUrl widgetUrl()
        {
            return mWidgetUrl;
        }

        QString mainHtmlFileName()
        {
            return mMainHtmlFileName;
        }

        bool hasEmbededObjects();
        bool hasEmbededFlash();

        QSize nominalSize() const
        {
            return mNominalSize;
        }

        bool canBeContent();
        bool canBeTool();

        bool hasLoadedSuccessfully() const
        {
            return (mInitialLoadDone && !mLoadIsErronous);
        }

        bool freezable() 
        { 
            return mIsFreezable;
        }

        bool resizable()
        { 
            return mIsResizable;
        }

        static QString iconFilePath(const QUrl& pUrl);
        static QString widgetName(const QUrl& pUrl);
        static int widgetType(const QUrl& pUrl);

        bool isFrozen()
        { 
            return mIsFrozen;
        }

        QPixmap snapshot(){
            return mSnapshot;
        }

        void setSnapshot(const QPixmap& pix);

        QPixmap takeSnapshot();

    public slots:
        void freeze();
        void unFreeze();
    /* end from */

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverMoveEvent(QGraphicsSceneHoverEvent *event);


        virtual bool eventFilter(QObject *obj, QEvent *event);

        virtual void sendJSEnterEvent();
        virtual void sendJSLeaveEvent();

        QMap<QString, QString> mPreferences;
        QMap<QString, QString> mDatastore;

        /* from UBAbstractWidget*/
        bool mMouseIsPressed;
        bool mFirstReleaseAfterMove;
        QUrl mMainHtmlUrl;
        QString mMainHtmlFileName;
        QUrl mWidgetUrl;
        QSize mNominalSize;
        bool mIsResizable;
        bool mInitialLoadDone;
        bool mLoadIsErronous;

        bool mIsFreezable;
        int mCanBeContent;
        int mCanBeTool;
        enum OSType
        {
            type_NONE = 0, // 0000
            type_WIN  = 1, // 0001
            type_MAC  = 2, // 0010
            type_UNIX = 4, // 0100
            type_ALL  = 7, // 0111
        };

        virtual void injectInlineJavaScript();
        virtual void paint( QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

        //virtual void dropEvent(QDropEvent *);
        /* end from */

    protected slots:
        void geometryChangeRequested(const QRect& geom);
        virtual void javaScriptWindowObjectCleared();
        /* from UBAbstractWidget*/
        void mainFrameLoadFinished(bool ok);
        /* end from */

    private:
        QPointF mLastMousePos;
        bool mShouldMoveWidget;
        UBWidgetUniboardAPI* mUniboardAPI;
        QUrl ownFolder;
        QUrl SnapshotFile;

        /* from UBAbstractWidget*/
        static QStringList sInlineJavaScripts;
        static bool sInlineJavaScriptLoaded;
        bool mIsFrozen;
        QPixmap mSnapshot;
        bool mIsTakingSnapshot;
        /* end from */
};

class UBGraphicsAppleWidgetItem : public UBGraphicsWidgetItem
{
    Q_OBJECT

    public:
        UBGraphicsAppleWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent = 0);
        ~UBGraphicsAppleWidgetItem();

        enum { Type = UBGraphicsItemType::AppleWidgetItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;
        virtual void setUuid(const QUuid &pUuid);

};

class UBGraphicsW3CWidgetItem : public UBGraphicsWidgetItem
{
    Q_OBJECT

    public:
        UBGraphicsW3CWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent = 0);
        ~UBGraphicsW3CWidgetItem();

        enum { Type = UBGraphicsItemType::W3CWidgetItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

        UBW3CWidget* w3cWidget() const;

        virtual void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget );
        virtual void setUuid(const QUuid &pUuid);

        static QString createNPAPIWrapper(const QString& url,
            const QString& pMimeType = QString(), const QSize& sizeHint = QSize(300, 150),
            const QString& pName = QString());

        static QString createNPAPIWrapperInDir(const QString& url, const QDir& pDir,
            const QString& pMimeType = QString(), const QSize& sizeHint = QSize(300, 150),
            const QString& pName = QString());

        static QString createHtmlWrapperInDir(const QString& html, const QDir& pDir,
            const QSize& sizeHint,  const QString& pName);

        static QString freezedWidgetPage();

        static bool hasNPAPIWrapper(const QString& pMimeType);

        class PreferenceValue
        {
            public:

                PreferenceValue()
                {
                    // NOOP
                }
                

                PreferenceValue(const QString& pValue, bool pReadonly)
                {
                    value = pValue;
                    readonly = pReadonly;
                }

                QString value;
                bool readonly;
         };

        class Metadata
        {
            public:
                QString id;
                QString name;
                QString description;
                QString author;
                QString authorEmail;
                QString authorHref;
                QString version;
        };

        QMap<QString, PreferenceValue> preferences()
        {
            return mPreferences;
        }

        Metadata metadatas() const
        {
            return mMetadatas;
        }

        Metadata mMetadatas;

    private slots:

        virtual void javaScriptWindowObjectCleared();

    private:

        QMap<QString, PreferenceValue> mPreferences;

        UBW3CWidgetAPI* mW3CWidgetAPI;

        static bool sTemplateLoaded;

        static QMap<QString, QString> sNPAPIWrapperTemplates;

        static QString sNPAPIWrappperConfigTemplate;

        static void loadNPAPIWrappersTemplates();

        static QString textForSubElementByLocale(QDomElement rootElement, QString subTagName, QLocale locale);


};

#endif // UBGRAPHICSWIDGETITEM_H
