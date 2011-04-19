/*
 * UBGraphicsTextItem.cpp
 *
 *  Created on: 30 mars 2009
 *      Author: Luc
 */

#include <QtGui>
#include "UBGraphicsTextItem.h"
#include "UBGraphicsTextItemDelegate.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsDelegateFrame.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "core/UBSettings.h"


QColor UBGraphicsTextItem::lastUsedTextColor;

UBGraphicsTextItem::UBGraphicsTextItem(QGraphicsItem * parent)
    : QGraphicsTextItem(parent)
    , mDelegate(0)
    , mMultiClickState(0)
    , mLastMousePressTime(QTime::currentTime())
{
    mDelegate = new UBGraphicsTextItemDelegate(this, 0);
    mDelegate->init();

    mDelegate->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);

    mTypeTextHereLabel = tr("<Type Text Here>");

    if (lastUsedTextColor.isValid())
    {
        setDefaultTextColor(lastUsedTextColor);
        setColorOnDarkBackground(lastUsedTextColor);
        setColorOnLightBackground(lastUsedTextColor);
    }
    else
    {
        QColor colorOnDarkBG = UBApplication::boardController->penColorOnDarkBackground();
        QColor colorOnLightBG = UBApplication::boardController->penColorOnLightBackground();

        setColorOnDarkBackground(colorOnDarkBG);
        setColorOnLightBackground(colorOnLightBG);
    }

    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setTextInteractionFlags(Qt::TextEditorInteraction);

    connect(document(), SIGNAL(contentsChanged()), mDelegate, SLOT(contentsChanged()));
    connect(document(), SIGNAL(undoCommandAdded()), this, SLOT(undoCommandAdded()));

    connect(document()->documentLayout(), SIGNAL(documentSizeChanged(const QSizeF &)), this, SLOT(documentSizeChanged(const QSizeF &)));

}


UBGraphicsTextItem::~UBGraphicsTextItem()
{
    if (mDelegate)
    {
        delete mDelegate;
    }
}


QVariant UBGraphicsTextItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (QGraphicsItem::ItemSelectedChange == change)
    {
        bool selected = value.toBool();

        if (selected)
        {
            setTextInteractionFlags(Qt::TextEditorInteraction);
        }
        else
        {
            QTextCursor tc = textCursor();
            tc.clearSelection();
            setTextCursor(tc);
            setTextInteractionFlags(Qt::NoTextInteraction);
        }
    }

    QVariant newValue = value;

    if(mDelegate)
        newValue = mDelegate->itemChange(change, value);

    return QGraphicsTextItem::itemChange(change, newValue);
}


void UBGraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    int elapsed = mLastMousePressTime.msecsTo(QTime::currentTime());

    if (elapsed < UBApplication::app()->doubleClickInterval())
    {
        mMultiClickState++;
        if (mMultiClickState > 3)
            mMultiClickState = 1;
    }
    else
    {
        mMultiClickState = 1;
    }

    mLastMousePressTime = QTime::currentTime();

    if (mMultiClickState == 1)
    {
        if (mDelegate)
            mDelegate->mousePressEvent(event);

        QGraphicsTextItem::mousePressEvent(event);
        setTextInteractionFlags(Qt::TextEditorInteraction);

        setFocus();
    }
    else if (mMultiClickState == 2)
    {
        QTextCursor tc= textCursor();
        tc.select(QTextCursor::WordUnderCursor);
        setTextCursor(tc);
    }
    else if (mMultiClickState == 3)
    {
        QTextCursor tc= textCursor();
        tc.select(QTextCursor::Document);
        setTextCursor(tc);
    }
    else
    {
        mMultiClickState = 0;
    }
}


void UBGraphicsTextItem::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (!mDelegate || !mDelegate->mouseMoveEvent(event))
    {
        QGraphicsTextItem::mouseMoveEvent(event);
    }
}


void UBGraphicsTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (mMultiClickState == 1)
    {
        if (mDelegate)
            mDelegate->mouseReleaseEvent(event);

        QGraphicsTextItem::mouseReleaseEvent(event);
    }
    else
    {
        event->accept();
    }
}


void UBGraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QColor color = UBSettings::settings()->isDarkBackground() ? mColorOnDarkBackground : mColorOnLightBackground;
    setDefaultTextColor(color);

    // Never draw the rubber band, we draw our custom selection with the DelegateFrame
    QStyleOptionGraphicsItem styleOption = QStyleOptionGraphicsItem(*option);
    styleOption.state &= ~QStyle::State_Selected;
    styleOption.state &= ~QStyle::State_HasFocus;

    QGraphicsTextItem::paint(painter, &styleOption, widget);

    if (widget == UBApplication::boardController->controlView()->viewport() &&
            !isSelected() && toPlainText().isEmpty())
    {
        painter->setFont(font());
        painter->setPen(UBSettings::paletteColor);
        painter->drawText(boundingRect(), Qt::AlignCenter, mTypeTextHereLabel);
    }
}


UBItem* UBGraphicsTextItem::deepCopy() const
{
    UBGraphicsTextItem* copy = new UBGraphicsTextItem();

    copy->setPlainText(this->toPlainText());
    copy->setPos(this->pos());
    copy->setZValue(this->zValue());
    copy->setTransform(this->transform());
    copy->setFlag(QGraphicsItem::ItemIsMovable, true);
    copy->setFlag(QGraphicsItem::ItemIsSelectable, true);
    copy->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
    copy->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
    copy->setDefaultTextColor(this->defaultTextColor());
    copy->setFont(this->font());
    copy->setColorOnDarkBackground(this->colorOnDarkBackground());
    copy->setColorOnLightBackground(this->colorOnLightBackground());
    copy->setTextWidth(this->textWidth());
    copy->setTextHeight(this->textHeight());

    copy->setSourceUrl(this->sourceUrl());

   // TODO UB 4.7 ... complete all members ?

   return copy;
}


QRectF UBGraphicsTextItem::boundingRect() const
{
    qreal width = textWidth();
    qreal height = textHeight();
    return QRectF(QPointF(), QSizeF(width, height));
}


QPainterPath UBGraphicsTextItem::shape() const
{
    QPainterPath path;
    path.addRect(boundingRect());
    return path;
}

void UBGraphicsTextItem::setTextWidth(qreal width)
{
    QFontMetrics fm(font());
    qreal strictMin = fm.height();
    qreal newWidth = qMax(strictMin, width);

    if (toPlainText().isEmpty())
    {
        qreal minWidth = fm.width(mTypeTextHereLabel);
        newWidth = qMax(minWidth, newWidth);
    }

    QGraphicsTextItem::setTextWidth(newWidth);
}


void UBGraphicsTextItem::setTextHeight(qreal height)
{
    QFontMetrics fm(font());
    qreal minHeight = fm.height() + document()->documentMargin() * 2;
    mTextHeight = qMax(minHeight, height);
    update();
}


qreal UBGraphicsTextItem::textHeight() const
{
    return mTextHeight;
}


void UBGraphicsTextItem::contentsChanged()
{
    if (scene())
    {
        scene()->setModified(true);
    }

    if (toPlainText().isEmpty())
    {
        setTextWidth(textWidth());
    }
}


UBGraphicsScene* UBGraphicsTextItem::scene()
{
    return static_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


void UBGraphicsTextItem::resize(qreal w, qreal h)
{
    setTextWidth(w);
    setTextHeight(h);

    if (mDelegate)
        mDelegate->positionHandles();
}


QSizeF UBGraphicsTextItem::size() const
{
    return QSizeF(textWidth(), textHeight());
}


void UBGraphicsTextItem::undoCommandAdded()
{
    emit textUndoCommandAdded(this);
}


void UBGraphicsTextItem::remove()
{
    if (mDelegate)
        mDelegate->remove(true);
}

void UBGraphicsTextItem::documentSizeChanged(const QSizeF & newSize)
{
    resize(newSize.width(), newSize.height());
}
