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




#ifndef UBGRAPHICSWIDGETITEM_H
#define UBGRAPHICSWIDGETITEM_H

#include <QtGui>
#include <QDomElement>
#include <QGraphicsWebView>

#include "core/UB.h"

#include "UBItem.h"
#include "UBResizableGraphicsItem.h"

class UBWidgetUniboardAPI;
class UBGraphicsScene;
class UBW3CWidgetAPI;
class UBW3CWidgetWebStorageAPI;
class UBGraphiscItem;
class UBGraphiscItemDelegate;

struct UBWidgetType
{
    enum Enum
    {
        W3C = 0, Apple, Other
    };
};

class UBGraphicsWidgetItem : public QGraphicsWebView, public UBItem, public UBResizableGraphicsItem, public UBGraphicsItem
{
    Q_OBJECT

    public:
        UBGraphicsWidgetItem(const QUrl &pWidgetUrl = QUrl(), QGraphicsItem *parent = 0);
        ~UBGraphicsWidgetItem();

        enum { Type = UBGraphicsItemType::GraphicsWidgetItemType };

        virtual int type() const { return Type; }

        virtual void initialize();

        virtual void resize(qreal w, qreal h);
        virtual void resize(const QSizeF & size);
        virtual QSizeF size() const;

        QUrl mainHtml();
        void loadMainHtml();
        QUrl widgetUrl();
        void widgetUrl(QUrl url) { mWidgetUrl = url; }
        QString mainHtmlFileName();

        bool canBeContent();
        bool canBeTool();

        QString preference(const QString& key) const;
        void setPreference(const QString& key, QString value);
        QMap<QString, QString> preferences() const;
        void removePreference(const QString& key);
        void removeAllPreferences();

        QString datastoreEntry(const QString& key) const;
        void setDatastoreEntry(const QString& key, QString value);
        QMap<QString, QString> datastoreEntries() const;
        void removeDatastoreEntry(const QString& key);
        void removeAllDatastoreEntries();

        void removeScript();

        void processDropEvent(QGraphicsSceneDragDropEvent *event);
        bool isDropableData(const QMimeData *data) const;

        virtual QUrl getOwnFolder() const;
        virtual void setOwnFolder(const QUrl &newFolder);
        virtual void setSnapshotPath(const QUrl &newFilePath);
        virtual QUrl getSnapshotPath();

        virtual void clearSource();

        virtual void setUuid(const QUuid &pUuid);

        QSize nominalSize() const;

        bool hasLoadedSuccessfully() const;

        bool freezable();
        bool resizable();
        bool isFrozen();

        QPixmap snapshot();
        void setSnapshot(const QPixmap& pix);
        QPixmap takeSnapshot();

        virtual UBItem* deepCopy() const = 0;
        virtual UBGraphicsScene* scene();

        static int widgetType(const QUrl& pUrl);
        static QString widgetName(const QUrl& pUrl);
        static QString iconFilePath(const QUrl& pUrl);

    public slots:
        void freeze();
        void unFreeze();

    protected:
        enum OSType
        {
            type_NONE = 0, // 0000
            type_WIN  = 1, // 0001
            type_MAC  = 2, // 0010
            type_UNIX = 4, // 0100
            type_ALL  = 7 // 0111
        };

        bool mFirstReleaseAfterMove;
        bool mInitialLoadDone;
        bool mIsFreezable;
        bool mIsResizable;
        bool mLoadIsErronous;
        bool mMouseIsPressed;
        int mCanBeContent;
        int mCanBeTool;
        QSize mNominalSize;
        QString mMainHtmlFileName;
        QUrl mMainHtmlUrl;
        QUrl mWidgetUrl;
        QMap<QString, QString> mDatastore;
        QMap<QString, QString> mPreferences;


        virtual bool event(QEvent *event);
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event);
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);
        virtual void sendJSEnterEvent();
        virtual void sendJSLeaveEvent();
        virtual void injectInlineJavaScript();
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event);
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0);

    protected slots:
        void geometryChangeRequested(const QRect& geom);
        virtual void javaScriptWindowObjectCleared();
        void mainFrameLoadFinished(bool ok);

    private slots:
        void onLinkClicked(const QUrl& url);
        void initialLayoutCompleted();

    private:
        bool mIsFrozen;
        bool mIsTakingSnapshot;
        bool mShouldMoveWidget;
        UBWidgetUniboardAPI* mUniboardAPI;
        QPixmap mSnapshot;
        QPointF mLastMousePos;
        QUrl ownFolder;
        QUrl SnapshotFile;

        static bool sInlineJavaScriptLoaded;
        static QStringList sInlineJavaScripts;
};

class UBGraphicsAppleWidgetItem : public UBGraphicsWidgetItem
{
    Q_OBJECT

    public:
        UBGraphicsAppleWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent = 0);
        ~UBGraphicsAppleWidgetItem();

        virtual void copyItemParameters(UBItem *copy) const;
        virtual void setUuid(const QUuid &pUuid);
        virtual UBItem* deepCopy() const;
};

class UBGraphicsW3CWidgetItem : public UBGraphicsWidgetItem
{
    Q_OBJECT

    public:
        class PreferenceValue
        {
            public:

                PreferenceValue()
                {
                    /* NOOP */
                }


                PreferenceValue(const QString& pValue, bool pReadonly)
                {
                    value = pValue;
                    readonly = pReadonly;
                }

                bool readonly;
                QString value;
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

        UBGraphicsW3CWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent = 0);
        ~UBGraphicsW3CWidgetItem();

        virtual void setUuid(const QUuid &pUuid);
        virtual UBItem* deepCopy() const;
        virtual void copyItemParameters(UBItem *copy) const;
        QMap<QString, PreferenceValue> preferences();
        Metadata metadatas() const;

        static QString freezedWidgetFilePath();
        static QString createNPAPIWrapper(const QString& url, const QString& pMimeType = QString(), const QSize& sizeHint = QSize(300, 150), const QString& pName = QString());
        static QString createNPAPIWrapperInDir(const QString& url, const QDir& pDir, const QString& pMimeType = QString(), const QSize& sizeHint = QSize(300, 150), const QString& pName = QString());
        static QString createHtmlWrapperInDir(const QString& html, const QDir& pDir, const QSize& sizeHint,  const QString& pName);
        static QString freezedWidgetPage();
        static bool hasNPAPIWrapper(const QString& pMimeType);

        Metadata mMetadatas;

    private slots:
        virtual void javaScriptWindowObjectCleared();

    private:
        static void loadNPAPIWrappersTemplates();
        static QString textForSubElementByLocale(QDomElement rootElement, QString subTagName, QLocale locale);

        UBW3CWidgetAPI* mW3CWidgetAPI;
        QMap<QString, PreferenceValue> mPreferences;

        static bool sTemplateLoaded;
        static QString sNPAPIWrappperConfigTemplate;
        static QMap<QString, QString> sNPAPIWrapperTemplates;
};

#endif // UBGRAPHICSWIDGETITEM_H
