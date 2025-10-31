/*
 * Copyright (C) 2015-2024 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBThumbnail.h"

#include <QGraphicsPixmapItem>
#include <QSvgRenderer>

#include "board/UBDrawingController.h"
#include "document/UBDocument.h"
#include "domain/UBGraphicsScene.h"
#include "gui/UBThumbnailArranger.h"
#include "gui/UBThumbnailScene.h"
#include "gui/UBThumbnailTextItem.h"


constexpr int cSelectionWidth{8};
constexpr QMarginsF cSelectionMargins{QMarginsF{} + cSelectionWidth / 2 + 1};
constexpr int cLabelOffset{5};

UBThumbnail::UBThumbnail()
    : mPixmapItem{new QGraphicsPixmapItem{this}}
    , mTextItem{new UBThumbnailTextItem}
{
    mTextItem->setParentItem(this);

    // accept hover to show/hide UI buttons
    setAcceptHoverEvents(true);

    // make it selectable
    setFlag(QGraphicsItem::ItemIsSelectable, true);

    // arrange child items behind the transparent parent
    mPixmapItem->setFlag(QGraphicsItem::ItemStacksBehindParent);
    mTextItem->setFlag(QGraphicsItem::ItemStacksBehindParent);

    // smooth transformation
    mPixmapItem->setTransformationMode(Qt::SmoothTransformation);

    // create a reasonably sized empty pixmap, which is later replaced
    const auto pageSize = UBSettings::settings()->pageSize->get().toSize();
    const auto pixmapSize = pageSize.scaled(UBSettings::maxThumbnailWidth, 0, Qt::KeepAspectRatioByExpanding);
    QPixmap pixmap{pixmapSize};
    pixmap.fill(qApp->palette().color(QPalette::Mid));
    mPixmapItem->setPixmap(pixmap);

    // do not draw anything
    setPen(QPen(QColor(Qt::transparent)));
}

void UBThumbnail::setPixmap(const QPixmap& pixmap)
{
    const auto needsAdjust = pixmap.size() != mPixmapItem->pixmap().size();

    mPixmapItem->setPixmap(pixmap);
    mLoaded = true;

    if (needsAdjust)
    {
        adjustThumbnail();
    }
}

QPixmap UBThumbnail::pixmap() const
{
    return mPixmapItem->pixmap();
}

bool UBThumbnail::isLoaded() const
{
    return mLoaded;
}

void UBThumbnail::setSceneIndex(int sceneIndex)
{
    mIndex = sceneIndex;
    mTextItem->setPageNumber(sceneIndex + 1);
}

int UBThumbnail::sceneIndex() const
{
    return mIndex;
}

void UBThumbnail::setThumbnailSize(QSizeF size)
{
    // Set size of thumbnail rectangle
    setRect(QRectF{{0, 0}, size} + cSelectionMargins);

    // Calculate height of text and spacing between pixmap and text
    const auto labelHeight = labelSpacing();

    // Calculate remaining space for pixmap
    const auto heightForPixmap = std::max(size.height() - labelHeight, 0.);

    // Scale pixmap to fit into this space
    const auto pixmapSize = mPixmapItem->pixmap().size();
    const auto scaleWidth = double(size.width()) / pixmapSize.width();
    const auto scaleHeight = double(heightForPixmap) / pixmapSize.height();

    // bitmap should not be stretched
    const auto scaleFactor = std::min({scaleWidth, scaleHeight, 1.});

    QTransform transform;
    transform.scale(scaleFactor, scaleFactor);

    mPixmapItem->setTransform(transform);

    // center pixmap horizontally
    mPixmapItem->setPos((size.width() - pixmapSize.width() * scaleFactor) / 2, 0);

    // set text width based on thumbnail width
    mTextItem->setWidth(size.width());

    // position label below pixmap
    mTextItem->setPos(0, pixmapSize.height() * scaleFactor + cLabelOffset);
}

void UBThumbnail::setColumn(int column)
{
    mColumn = column;
}

void UBThumbnail::setRow(int row)
{
    mRow = row;
}

int UBThumbnail::column() const
{
    return mColumn;
}

int UBThumbnail::row() const
{
    return mRow;
}

void UBThumbnail::setPageScene(std::shared_ptr<UBGraphicsScene> pageScene)
{
    mPageScene = pageScene;
    adjustThumbnail();
}

void UBThumbnail::setDeletable(bool deletable)
{
    mDeletable = deletable;
}

void UBThumbnail::updatePixmap(const QRectF& region)
{
    auto pageScene = mPageScene.lock();

    if (pageScene && !mPixmapItem->pixmap().isNull())
    {
        QPixmap pixmap = this->pixmap();
        QRectF pixmapRect;
        const auto tool = UBDrawingController::drawingController()->stylusTool();
        const auto affectsWholeScene = tool == UBStylusTool::Play || tool == UBStylusTool::Selector;

        if (region.isNull() || affectsWholeScene)
        {
            // full update if region unknown or play/selector tool active, which may affect whole scene
            pixmapRect = pixmap.rect();
        }
        else
        {
            // only render affected region

            /*
             * To avoid problems with antialiaing we have to make sure that the
             * pixmapRect has an integer size and position and covers at least
             * the affected region. The following steps assure this.
             */
            pixmapRect = mTransform.mapRect(region); // translate to pixmap coordinates
            pixmapRect += QMarginsF(1, 1, 1, 1);     // add a margin
            pixmapRect = pixmapRect.toRect();        // cut to integer
            pixmapRect &= pixmap.rect();             // limit to pixmap area
        }

        if (pixmapRect.isValid())
        {
            QPainter painter{&pixmap};
            const auto sceneRect = mTransform.inverted().mapRect(pixmapRect);
            pageScene->render(&painter, pixmapRect, sceneRect);
            setPixmap(pixmap);
        }
    }
}

void UBThumbnail::adjustThumbnail()
{
    auto pageScene = mPageScene.lock();

    if (pageScene)
    {
        // get the used area of the scene
        auto mSceneRect = pageScene->normalizedSceneRect();

        // expand the scene rect so that it has the aspect ratio of the pixmap
        const QSizeF pSize = pixmap().size();
        const QPointF center = mSceneRect.center();
        const QSizeF sceneSize = pSize.scaled(mSceneRect.size(), Qt::KeepAspectRatioByExpanding);
        mSceneRect.setSize(sceneSize);
        mSceneRect.moveCenter(center);

        // create a transformation from scene to pixmap coordinates
        const QRectF pixRect{{0, 0}, pSize};
        QPolygonF p1{mSceneRect};
        QPolygonF p2{pixRect};

        p1.removeLast();
        p2.removeLast();

        QTransform::quadToQuad(p1, p2, mTransform);

        // re-paint the pixmap
        updatePixmap();
    }
}

/**
 * @brief Compute height of the thumbnail item given a width.
 *
 * The height is calculated by computing the heigth of the pixmap using a
 * fixed aspect ratio and ading the height of a spacer below the pixmap
 * and the caption below that.
 *
 * @param width Width of thumbnail.
 * @return Total size of the thumbnail including caption.
 */
double UBThumbnail::heightForWidth(double width)
{
    const auto height = std::ceil(width / UBSettings::minScreenRatio);

    return height + labelSpacing();
}

QVariant UBThumbnail::itemChange(GraphicsItemChange change, const QVariant& value)
{
    if (change == QGraphicsItem::ItemSelectedHasChanged)
    {
        const auto penColor = isSelected() ? UBSettings::treeViewBackgroundColor : Qt::transparent;
        setPen({penColor, cSelectionWidth});
    }

    return QGraphicsRectItem::itemChange(change, value);
}

void UBThumbnail::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    auto thumbnailScene = dynamic_cast<UBThumbnailScene*>(scene());

    if (thumbnailScene)
    {
        mEditable = thumbnailScene->currentThumbnailArranger()->isUIEnabled();
    }

    QGraphicsRectItem::hoverEnterEvent(event);
}

void UBThumbnail::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    mEditable = false;
    QGraphicsRectItem::hoverLeaveEvent(event);
}

void UBThumbnail::mousePressEvent(QGraphicsSceneMouseEvent* event)
{
    auto thumbnailScene = dynamic_cast<UBThumbnailScene*>(scene());

    if (thumbnailScene)
    {
        using namespace UBThumbnailUI;

        const auto p = event->pos() - mPixmapItem->pos();

        if (mDeletable && getIcon("close")->triggered(p))
        {
            // deleting must be performed later, don't kill yourself
            QTimer::singleShot(0, [this, thumbnailScene]() { thumbnailScene->document()->deletePages({mIndex}); });
            return;
        }
        else if (getIcon("duplicate")->triggered(p))
        {
            thumbnailScene->document()->duplicatePage(mIndex);
            return;
        }
    }

    event->ignore();
}

void UBThumbnail::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    // paint a selection frame but only around the pixmap
    if (isSelected())
    {
        const auto selectionRect = mapRectFromItem(mPixmapItem, mPixmapItem->boundingRect()) + cSelectionMargins;
        painter->setPen(pen());
        painter->drawRect(selectionRect);
    }

    // paint the buttons
    if (mEditable)
    {
        using namespace UBThumbnailUI; //should be reworked so icons' scale adapts to what's needed
        const UBThumbnailUIIcon& closeThumbnailIcon = *UBThumbnailUI::getIcon(mDeletable ? "close" : "closeDisabled");
        const UBThumbnailUIIcon& duplicateThumbnailIcon = *UBThumbnailUI::getIcon("duplicate");
        painter->drawPixmap(mPixmapItem->x() + (closeThumbnailIcon.pos() * (ICONSIZE + ICONSPACING)), 0, ICONSIZE, ICONSIZE, closeThumbnailIcon);
        painter->drawPixmap(mPixmapItem->x() + (duplicateThumbnailIcon.pos() * (ICONSIZE + ICONSPACING)), 0, ICONSIZE, ICONSIZE, duplicateThumbnailIcon);
    }

    // do not call superclass - do not paint the rectangle itself or the default dashed selection rectangle
}

int UBThumbnail::labelSpacing()
{
    // identical and static all over the application
    static int labelSpacing{UBSettings::thumbnailSpacing + QFontMetrics{QFont{}}.height()};

    return labelSpacing;
}

/* ----- UBThumbnailUI ----- */

UBThumbnailUI::UBThumbnailUIIcon::UBThumbnailUIIcon(const QString& filename, int pos)
    : QPixmap{QSize{ICONSIZE, ICONSIZE}}
    , mPos{pos}
{
    QSvgRenderer svgRenderer{filename};
    QPainter painter;
    fill(Qt::transparent);
    painter.begin(this);
    svgRenderer.render(&painter);
    painter.end();
}

int UBThumbnailUI::UBThumbnailUIIcon::pos() const
{
    return mPos;
}

bool UBThumbnailUI::UBThumbnailUIIcon::triggered(QPointF p) const
{
    QRect iconRect{mPos * (ICONSIZE + ICONSPACING), 0, ICONSIZE, ICONSIZE};
    return iconRect.contains(p.toPoint());
}

UBThumbnailUI::UBThumbnailUIIcon* UBThumbnailUI::addIcon(const QString& thumbnailIcon, int pos)
{
    QString thumbnailIconPath = ":images/" + thumbnailIcon + ".svg";
    UBThumbnailUIIcon* newIcon = new UBThumbnailUIIcon(thumbnailIconPath, pos);

    using namespace UBThumbnailUI::_private;

    if (!newIcon)
    {
        qDebug() << "cannot add Icon : check path : " + thumbnailIconPath;
    }
    else
    {
        catalog.insert(thumbnailIcon, newIcon);
    }

    return newIcon;
}

UBThumbnailUI::UBThumbnailUIIcon* UBThumbnailUI::getIcon(const QString& thumbnailIcon)
{
    using namespace UBThumbnailUI::_private;

    if (!catalog.contains(thumbnailIcon))
    {
        qDebug() << "cannot get Icon: check path ':images/" + thumbnailIcon + ".svg'";
    }

    return catalog.value(thumbnailIcon, nullptr);
}

void UBThumbnailUI::draw(QPainter* painter, const UBThumbnailUIIcon& thumbnailIcon)
{
    using namespace UBThumbnailUI;

    painter->drawPixmap(thumbnailIcon.pos() * (ICONSIZE + ICONSPACING), 0, ICONSIZE, ICONSIZE, thumbnailIcon);
}

void UBThumbnailUI::_private::initCatalog()
{
    using namespace UBThumbnailUI;
    using namespace UBThumbnailUI::_private;

    addIcon("close", 0);
    addIcon("closeDisabled", 0);

    addIcon("duplicate", 1);

    addIcon("moveUp", 2);
    addIcon("moveUpDisabled", 2);

    addIcon("moveDown", 3);
    addIcon("moveDownDisabled", 3);
}
