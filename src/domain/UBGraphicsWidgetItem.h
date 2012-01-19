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
#ifndef UBGRAPHICSWIDGETITEM_H
#define UBGRAPHICSWIDGETITEM_H

#include <QtGui>
#include <QtWebKit>

#include "core/UB.h"

#include "UBGraphicsProxyWidget.h"
#include "UBW3CWidget.h"
#include "UBAppleWidget.h"

class UBWidgetUniboardAPI;
class UBGraphicsScene;
class UBAbstractWidget;
class UBW3CWidgetAPI;
class UBW3CWidgetWebStorageAPI;
class UBGraphiscItem;
class UBGraphiscItemDelegate;

class UBGraphicsWidgetItem : public UBGraphicsProxyWidget
{
    Q_OBJECT

    public:
        UBGraphicsWidgetItem(QGraphicsItem *parent = 0, int widgetType = 0);
        ~UBGraphicsWidgetItem();

        virtual UBGraphicsScene* scene();

        UBAbstractWidget* widgetWebView();

        virtual void initialize();

        virtual UBItem* deepCopy() const = 0;

        virtual void resize ( qreal w, qreal h );
        virtual void resize ( const QSizeF & size );

        void setPreference(const QString& key, QString value);
        QString preference(const QString& key) const;
        QMap<QString, QString> preferences() const;
        void removePreference(const QString& key);
        void removeAllPreferences();

        void setDatastoreEntry(const QString& key, QString value);
        QString datastoreEntry(const QString& key) const;
        QMap<QString, QString> datastoreEntries() const;
        void removeDatastoreEntry(const QString& key);
        void removeAllDatastoreEntries();
        virtual UBGraphicsItemDelegate* Delegate() const {return mDelegate;}

        virtual void remove();
        void removeScript();
        QString downloadUrl(const QString &fileUrl, const QString &extention);

        virtual void setOwnFolder(const QUrl &newFolder) {ownFolder = newFolder;}
        virtual QUrl getOwnFolder() const {return ownFolder;}
        virtual void setSnapshotPath(const QUrl &newFilePath){SnapshotFile = newFilePath;}
        virtual QUrl getSnapshotPath(){return SnapshotFile;}

        virtual void clearSource();

    protected:

        virtual void mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual bool eventFilter(QObject *obj, QEvent *event);

        UBAbstractWidget* mWebKitWidget;

        QMap<QString, QString> mPreferences;
        QMap<QString, QString> mDatastore;

    protected slots:
        void geometryChangeRequested(const QRect& geom);
        virtual void javaScriptWindowObjectCleared();

    private:
        QPointF mLastMousePos;
        bool mShouldMoveWidget;
        UBWidgetUniboardAPI* mUniboardAPI;
        QUrl ownFolder;
        QUrl SnapshotFile;
};

class UBGraphicsAppleWidgetItem : public UBGraphicsWidgetItem
{
    Q_OBJECT

    public:
        UBGraphicsAppleWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent = 0);
        UBGraphicsAppleWidgetItem(UBAppleWidget *appleWidget, QGraphicsItem *parent = 0);
        ~UBGraphicsAppleWidgetItem();

        enum { Type = UBGraphicsItemType::AppleWidgetItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

};

class UBGraphicsW3CWidgetItem : public UBGraphicsWidgetItem
{
    Q_OBJECT

    public:
        UBGraphicsW3CWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent = 0, int widgetType = UBGraphicsItemType::W3CWidgetItemType);
        UBGraphicsW3CWidgetItem(UBW3CWidget *w3cWidget, QGraphicsItem *parent = 0, int widgetType = UBGraphicsItemType::W3CWidgetItemType);
        ~UBGraphicsW3CWidgetItem();

        enum { Type = UBGraphicsItemType::W3CWidgetItemType };

        virtual int type() const
        {
            return Type;
        }

        virtual UBItem* deepCopy() const;

        UBW3CWidget::Metadata metadatas() const;

        UBW3CWidget* w3cWidget() const;

        virtual void paint ( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget );

    private slots:

        virtual void javaScriptWindowObjectCleared();

    private:

        UBW3CWidget* mW3CWidget;
        UBW3CWidgetAPI* mW3CWidgetAPI;


};

#endif // UBGRAPHICSWIDGETITEM_H
