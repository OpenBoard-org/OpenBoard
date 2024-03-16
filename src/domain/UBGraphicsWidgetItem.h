/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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
#include <QGraphicsProxyWidget>

#include "core/UB.h"

#include "UBItem.h"
#include "UBResizableGraphicsItem.h"

class QWebChannel;

class UBWidgetUniboardAPI;
class UBGraphicsScene;
class UBW3CWidgetAPI;
class UBW3CWidgetWebStorageAPI;
class UBGraphiscItem;
class UBGraphiscItemDelegate;
class UBWebEngineView;

struct UBWidgetType
{
    enum Enum
    {
        W3C = 0, Apple, Other
    };
};

class UBGraphicsWidgetItem : public QGraphicsProxyWidget, public UBItem, public UBResizableGraphicsItem, public UBGraphicsItem
{
    Q_OBJECT

    public:
        UBGraphicsWidgetItem(const QUrl &pWidgetUrl = QUrl(), QGraphicsItem *parent = 0);
        ~UBGraphicsWidgetItem();

        enum { Type = UBGraphicsItemType::GraphicsWidgetItemType };

        virtual int type() const override { return Type; }

        virtual void initialize();

        virtual void resize(qreal w, qreal h) override;
        virtual void resize(const QSizeF & size) override;
        virtual QSizeF size() const override;

        QUrl mainHtml() const;
        void loadMainHtml();
        void load(QUrl url);
        QUrl widgetUrl() const;
        void widgetUrl(const QUrl &url) { mWidgetUrl = url; }
        QString mainHtmlFileName() const;

        bool canBeContent() const;
        bool canBeTool() const;
        void setCanBeTool(bool tool);

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

        void runScript(const QString& script);
        virtual void removeScript();

        bool processDropEvent(QGraphicsSceneDragDropEvent *event);
        bool isDropableData(const QMimeData *data) const;

        virtual QUrl getOwnFolder() const;
        virtual void setOwnFolder(const QUrl &newFolder);
        virtual void setSnapshotPath(const QUrl &newFilePath);
        virtual QUrl getSnapshotPath() const;

        virtual void clearSource() override;

        virtual void setUuid(const QUuid &pUuid) override;

        QSize nominalSize() const;

        bool hasLoadedSuccessfully() const;

        bool freezable() const;
        bool resizable() const;
        bool isFrozen() const;
        void setFreezable(bool freezable);
        bool isWebActive() const;

        const QPixmap& snapshot() const;
        void setSnapshot(const QPixmap& pix, bool frozen);
        const QPixmap& takeSnapshot();
        void saveSnapshot() const;

        void updatePosition();

        virtual UBItem* deepCopy() const override = 0;
        virtual std::shared_ptr<UBGraphicsScene> scene() override;

        static int widgetType(const QUrl& pUrl);
        static QString widgetName(const QUrl& pUrl);
        static QString iconFilePath(const QUrl& pUrl);

    public slots:
        void initAPI();
        void freeze();
        void unFreeze();
        void setWebActive(bool active);
        void inspectPage();
        void closeInspector();

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
        UBWebEngineView* mWebEngineView;
        QSize mNominalSize;
        QString mMainHtmlFileName;
        QUrl mMainHtmlUrl;
        QUrl mWidgetUrl;
        QMap<QString, QString> mDatastore;
        QMap<QString, QString> mPreferences;
#ifndef Q_OS_WIN
        /*
         * workaround for QTBUG-79216 - to be removed when bug is fixed
        */
        Qt::Key mLastDeadKey{Qt::Key_unknown};

        static const QMap<Qt::Key, QString> sDeadKeys;
        static const QMap<QString, QString> sAccentedCharacters;

        QString getAccentedLetter(Qt::Key deadKey, const QString& letter) const;
#endif

        virtual bool event(QEvent *event) override;
        virtual void dropEvent(QGraphicsSceneDragDropEvent *event) override;
        virtual void keyPressEvent(QKeyEvent *event) override;
        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event) override;
        virtual void hoverEnterEvent(QGraphicsSceneHoverEvent *event) override;
        virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent *event) override;
        virtual void sendJSEnterEvent();
        virtual void sendJSLeaveEvent();
        virtual void injectInlineJavaScript();
        virtual void wheelEvent(QGraphicsSceneWheelEvent *event) override;
        virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value) override;
        virtual void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = 0) override;
        virtual bool eventFilter(QObject *obj, QEvent *ev) override;

    protected slots:
        void geometryChangeRequested(const QRect& geom);
        virtual void registerAPI();
        void mainFrameLoadFinished(bool ok);

    private:
        bool mIsFrozen;
        bool mIsWebActive;
        bool mShouldMoveWidget;
        QWebChannel* mWebChannel;
        UBWidgetUniboardAPI* mUniboardAPI;
        QPixmap mSnapshot;
        QPointF mLastMousePos;
        QUrl mOwnFolder;
        QUrl mSnapshotFile;

        static bool sInlineJavaScriptLoaded;
        static QStringList sInlineJavaScripts;
};

// NOTE @letsfindaway obsolete
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
        QMap<QString, PreferenceValue> preferences() const;
        Metadata metadatas() const;

        virtual void removeScript();
        virtual void sendJSEnterEvent();
        virtual void sendJSLeaveEvent();

        static QString createNPAPIWrapper(const QString& url, const QString& pMimeType = QString(), const QSize& sizeHint = QSize(300, 150), const QString& pName = QString());
        static QString createNPAPIWrapperInDir(const QString& url, const QDir& pDir, const QString& pMimeType = QString(), const QSize& sizeHint = QSize(300, 150), const QString& pName = QString());
        static QString createHtmlWrapperInDir(const QString& html, const QDir& pDir, const QSize& sizeHint,  const QString& pName);
        static bool hasNPAPIWrapper(const QString& pMimeType);

        Metadata mMetadatas;

    private slots:
        virtual void registerAPI();

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
