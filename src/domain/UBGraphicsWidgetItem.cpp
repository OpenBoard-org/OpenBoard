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

#include "UBGraphicsWidgetItem.h"

#include "api/UBWidgetUniboardAPI.h"
#include "api/UBW3CWidgetAPI.h"

#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsWidgetItemDelegate.h"
#include "UBGraphicsDelegateFrame.h"

#include "UBW3CWidget.h"
#include "UBGraphicsScene.h"
#include "UBAppleWidget.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/memcheck.h"

UBGraphicsWidgetItem::UBGraphicsWidgetItem(QGraphicsItem *parent, int widgetType)
    : UBGraphicsProxyWidget(parent)
    , mWebKitWidget(0)
    , mShouldMoveWidget(false)
    , mUniboardAPI(0)
{
    setAcceptDrops(false);
    UBGraphicsWidgetItemDelegate* delegate = new UBGraphicsWidgetItemDelegate(this, widgetType);
    delegate->init();
    setDelegate(delegate);
}


UBGraphicsWidgetItem::~UBGraphicsWidgetItem()
{
    // NOOP
}


void UBGraphicsWidgetItem::javaScriptWindowObjectCleared()
{
    if(!mUniboardAPI)
            mUniboardAPI = new UBWidgetUniboardAPI(scene(), this);

    mWebKitWidget->page()->mainFrame()->addToJavaScriptWindowObject("sankore", mUniboardAPI);

}


void UBGraphicsWidgetItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    UBGraphicsProxyWidget::mousePressEvent(event);

    // did webkit consume the mouse press ?
    mShouldMoveWidget = !event->isAccepted() && (event->buttons() & Qt::LeftButton);

    mLastMousePos = mapToScene(event->pos());

    event->accept();
}


void UBGraphicsWidgetItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mShouldMoveWidget = false;

    UBGraphicsProxyWidget::mouseReleaseEvent(event);
}


bool UBGraphicsWidgetItem::eventFilter(QObject *obj, QEvent *event)
{
    if (mShouldMoveWidget && obj == mWebKitWidget && event->type() == QEvent::MouseMove)
    {
        QMouseEvent *mouseMoveEvent = static_cast<QMouseEvent*>(event);

        if (mouseMoveEvent->buttons() & Qt::LeftButton)
        {
            QPointF scenePos = mapToScene(mouseMoveEvent->pos());

            QPointF newPos = pos() + scenePos - mLastMousePos;

            setPos(newPos);

            mLastMousePos = scenePos;

            event->accept();

            return true;
        }
    }

    //standard event processing
    return QObject::eventFilter(obj, event);
}


void UBGraphicsWidgetItem::resize ( qreal w, qreal h )
{
    UBGraphicsProxyWidget::resize(w, h);

    mWebKitWidget->resize(w, h);
}


void UBGraphicsWidgetItem::resize ( const QSizeF & size )
{
    resize(size.width(), size.height());
}


void UBGraphicsWidgetItem::geometryChangeRequested(const QRect& geom)
{
    resize(geom.width(), geom.height());
}


void UBGraphicsWidgetItem::initialize()
{
    connect(mWebKitWidget->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));

    QPalette palette = mWebKitWidget->page()->palette();
    palette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    mWebKitWidget->page()->setPalette(palette);

    UBGraphicsProxyWidget::setWidget(mWebKitWidget);

    mWebKitWidget->installEventFilter(this);

    UBGraphicsProxyWidget::setMinimumSize(mWebKitWidget->nominalSize());

    connect(mWebKitWidget, SIGNAL(geometryChangeRequested(const QRect&)), this, SLOT(geometryChangeRequested(const QRect&)));

    if (mDelegate && mDelegate->frame() && mWebKitWidget->resizable())
        mDelegate->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);
}


UBGraphicsScene* UBGraphicsWidgetItem::scene()
{
    return qobject_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


UBAbstractWidget* UBGraphicsWidgetItem::widgetWebView()
{
    return mWebKitWidget;
}


void UBGraphicsWidgetItem::setPreference(const QString& key, QString value)
{
    if (key == "" || (mPreferences.contains(key) && mPreferences.value(key) == value))
        return;

    mPreferences.insert(key, value);
    if (scene())
        scene()->setModified(true);
}


QString UBGraphicsWidgetItem::preference(const QString& key) const
{
    return mPreferences.value(key);
}


QMap<QString, QString> UBGraphicsWidgetItem::preferences() const
{
    return mPreferences;
}


void UBGraphicsWidgetItem::removePreference(const QString& key)
{
    mPreferences.remove(key);
}


void UBGraphicsWidgetItem::removeAllPreferences()
{
    mPreferences.clear();
}


void UBGraphicsWidgetItem::setDatastoreEntry(const QString& key, QString value)
{
    if (key == "" || (mDatastore.contains(key) && mDatastore.value(key) == value))
        return;

    mDatastore.insert(key, value);
    if (scene())
        scene()->setModified(true);
}


QString UBGraphicsWidgetItem::datastoreEntry(const QString& key) const
{
    if (mDatastore.contains(key))
        return mDatastore.value(key);
    else
        return "";
}


QMap<QString, QString> UBGraphicsWidgetItem::datastoreEntries() const
{
    return mDatastore;
}


void UBGraphicsWidgetItem::removeDatastoreEntry(const QString& key)
{
    mDatastore.remove(key);
}


void UBGraphicsWidgetItem::removeAllDatastoreEntries()
{
    mDatastore.clear();
}


void UBGraphicsWidgetItem::remove()
{

    if (mDelegate)
        mDelegate->remove();

}

void UBGraphicsWidgetItem::removeScript()
{
    if (mWebKitWidget && mWebKitWidget->page() && mWebKitWidget->page()->mainFrame())
    {
        mWebKitWidget->page()->mainFrame()->evaluateJavaScript("if(widget && widget.onremove) { widget.onremove();}");
    }
}

void UBGraphicsWidgetItem::clearSource()
{
    UBFileSystemUtils::deleteDir(getOwnFolder().toLocalFile());
    UBFileSystemUtils::deleteFile(getSnapshotPath().toLocalFile());
}

QString UBGraphicsWidgetItem::downloadUrl(const QString &fileUrl, const QString &extention)
{
    return mUniboardAPI->downloadUrl(fileUrl, extention);
}

UBGraphicsAppleWidgetItem::UBGraphicsAppleWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent)
    : UBGraphicsWidgetItem(parent)
{
    mWebKitWidget = new UBAppleWidget(pWidgetUrl, 0);
    initialize();
}


UBGraphicsAppleWidgetItem::UBGraphicsAppleWidgetItem(UBAppleWidget *appleWidget, QGraphicsItem *parent)
    : UBGraphicsWidgetItem(parent)

{
    mWebKitWidget = appleWidget;

    initialize();
}


UBGraphicsAppleWidgetItem::~UBGraphicsAppleWidgetItem()
{
    // NOOP
}


UBItem* UBGraphicsAppleWidgetItem::deepCopy() const
{
    UBGraphicsAppleWidgetItem *appleWidget = new UBGraphicsAppleWidgetItem(mWebKitWidget->widgetUrl(), parentItem());

    foreach(QString key, mPreferences.keys())
    {
        appleWidget->setPreference(key, mPreferences.value(key));
    }

    foreach(QString key, mDatastore.keys())
    {
        appleWidget->setDatastoreEntry(key, mDatastore.value(key));
    }

    appleWidget->setSourceUrl(this->sourceUrl());

    return appleWidget;

}


UBGraphicsW3CWidgetItem::UBGraphicsW3CWidgetItem(const QUrl& pWidgetUrl, QGraphicsItem *parent, int widgetType)
    : UBGraphicsWidgetItem(parent, widgetType)
    , mW3CWidgetAPI(0)
{
    mW3CWidget = new UBW3CWidget(pWidgetUrl, 0);
    mWebKitWidget = mW3CWidget;
    initialize();
}

UBGraphicsW3CWidgetItem::UBGraphicsW3CWidgetItem(UBW3CWidget *w3cWidget, QGraphicsItem *parent, int widgetType)
    : UBGraphicsWidgetItem(parent)
    , mW3CWidget(w3cWidget)
    , mW3CWidgetAPI(0)
{
    Q_UNUSED(widgetType);
    mWebKitWidget = mW3CWidget;
    initialize();
}


UBGraphicsW3CWidgetItem::~UBGraphicsW3CWidgetItem()
{
    // NOOP
}


void UBGraphicsW3CWidgetItem::paint(QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget)
{
    UBGraphicsScene::RenderingContext rc = UBGraphicsScene::Screen;

    if (scene())
      rc =  scene()->renderingContext();

    if ((!w3cWidget()->hasLoadedSuccessfully()) && (rc == UBGraphicsScene::NonScreen || rc == UBGraphicsScene::PdfExport))
    {
        if (!w3cWidget()->snapshot().isNull())
        {
           painter->drawPixmap(0, 0, w3cWidget()->snapshot());
        }
    }
    else
    {
        UBGraphicsProxyWidget::paint(painter, option, widget);
    }
}

void UBGraphicsW3CWidgetItem::javaScriptWindowObjectCleared()
{
    UBGraphicsWidgetItem::javaScriptWindowObjectCleared();

    if(!mW3CWidgetAPI)
        mW3CWidgetAPI = new UBW3CWidgetAPI(this);

    mWebKitWidget->page()->mainFrame()->addToJavaScriptWindowObject("widget", mW3CWidgetAPI);

}


UBW3CWidget::Metadata UBGraphicsW3CWidgetItem::metadatas() const
{
    return mW3CWidget->metadatas();
}


UBW3CWidget* UBGraphicsW3CWidgetItem::w3cWidget() const
{
    return mW3CWidget;
}


UBItem* UBGraphicsW3CWidgetItem::deepCopy() const
{
    UBGraphicsW3CWidgetItem *copy = new UBGraphicsW3CWidgetItem(mWebKitWidget->widgetUrl(), parentItem());

    copy->setPos(this->pos());
    UBGraphicsItem::assignZValue(copy, this->zValue());
    copy->setTransform(this->transform());
    copy->setFlag(QGraphicsItem::ItemIsMovable, true);
    copy->setFlag(QGraphicsItem::ItemIsSelectable, true);
    copy->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    copy->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
    copy->setUuid(this->uuid()); // this is OK for now as long as Widgets are imutable
    copy->setSourceUrl(this->sourceUrl());

    copy->resize(this->size());

    foreach(QString key, mPreferences.keys())
    {
        copy->setPreference(key, mPreferences.value(key));
    }

    foreach(QString key, mDatastore.keys())
    {
        copy->setDatastoreEntry(key, mDatastore.value(key));
    }

    return copy;
}
