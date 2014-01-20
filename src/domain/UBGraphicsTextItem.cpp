/*
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




#include <QtGui>
#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsTextItem.h"
#include "UBGraphicsTextItemDelegate.h"
#include "UBGraphicsScene.h"
#include "UBGraphicsDelegateFrame.h"

#include "core/UBApplication.h"
#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBDrawingController.h"
#include "core/UBSettings.h"

#include "core/memcheck.h"
QColor UBGraphicsTextItem::lastUsedTextColor;

UBGraphicsTextItem::UBGraphicsTextItem(QGraphicsItem * parent) :
    QGraphicsTextItem(parent)
    , UBGraphicsItem()
    , mMultiClickState(0)
    , mLastMousePressTime(QTime::currentTime())
    , mTypeTextHereLabel(tr("<Type Text Here>"))
{
    mEmptyTextWidth = QFontMetrics(font()).width(mTypeTextHereLabel);
    setDelegate(new UBGraphicsTextItemDelegate(this, 0));

    // TODO claudio remove this because in contrast with the fact the frame should be created on demand.
    Delegate()->createControls();
    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);
    Delegate()->setUBFlag(GF_FLIPPABLE_ALL_AXIS, false);
    Delegate()->setUBFlag(GF_REVOLVABLE, true);

    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly

    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setTextInteractionFlags(Qt::TextEditorInteraction);

    setUuid(QUuid::createUuid());

    connect(document(), SIGNAL(contentsChanged()), Delegate(), SLOT(contentsChanged()));
    connect(document(), SIGNAL(undoCommandAdded()), this, SLOT(undoCommandAdded()));

    connect(document()->documentLayout(), SIGNAL(documentSizeChanged(const QSizeF &)),
            this, SLOT(documentSizeChanged(const QSizeF &)));

}

UBGraphicsTextItem::~UBGraphicsTextItem()
{
}

void UBGraphicsTextItem::setSelected(bool selected)
{
    if(selected){
        Delegate()->createControls();
        Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);
    }
    QGraphicsTextItem::setSelected(selected);
}

QVariant UBGraphicsTextItem::itemChange(GraphicsItemChange change, const QVariant &value)
{
    QVariant newValue = value;

    if(Delegate())
        newValue = Delegate()->itemChange(change, value);

    return QGraphicsTextItem::itemChange(change, newValue);
}

void UBGraphicsTextItem::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    setTextInteractionFlags(Qt::TextEditorInteraction);

    // scene()->itemAt(pos) returns 0 if pos is not over text, but over text item, but mouse press comes.
    // It is a cludge...
    if (UBStylusTool::Play == UBDrawingController::drawingController()->stylusTool())
    {
        QGraphicsTextItem::mousePressEvent(event);
        event->accept();
        clearFocus();
        return;
    }

    if (Delegate())
    {
        Delegate()->mousePressEvent(event);
        if (Delegate() && parentItem() && UBGraphicsGroupContainerItem::Type == parentItem()->type())
        {
            UBGraphicsGroupContainerItem *group = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(parentItem());
            if (group)
            {
                QGraphicsItem *curItem = group->getCurrentItem();
                if (curItem && this != curItem)
                {
                    group->deselectCurrentItem();
                }
                group->setCurrentItem(this);
                this->setSelected(true);
                Delegate()->positionHandles();
            }

        }
    }

    if (!data(UBGraphicsItemData::ItemEditable).toBool())
        return;

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
        QGraphicsTextItem::mousePressEvent(event);
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
    if (!Delegate() || !Delegate()->mouseMoveEvent(event))
    {
        QGraphicsTextItem::mouseMoveEvent(event);
    }
}

void UBGraphicsTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // scene()->itemAt(pos) returns 0 if pos is not over text, but over text item, but mouse press comes.
    // It is a cludge...
    if (UBStylusTool::Play == UBDrawingController::drawingController()->stylusTool())
    {
        event->accept();
        clearFocus();
        return;
    }

    if (mMultiClickState == 1)
    {
        QGraphicsTextItem::mouseReleaseEvent(event);

        if (Delegate())
            Delegate()->mouseReleaseEvent(event);
    }
    else
    {
        event->accept();
    }
}

void UBGraphicsTextItem::keyPressEvent(QKeyEvent *event)
{
    if (Delegate() && !Delegate()->keyPressEvent(event)) {
        qDebug() << "UBGraphicsTextItem::keyPressEvent(QKeyEvent *event) has been rejected by delegate. Don't call base class method";
        return;
    }

    QGraphicsTextItem::keyPressEvent(event);
}

void UBGraphicsTextItem::keyReleaseEvent(QKeyEvent *event)
{
    if (Delegate() && !Delegate()->keyReleaseEvent(event)) {
        qDebug() << "UBGraphicsTextItem::keyPressEvent(QKeyEvent *event) has been rejected by delegate. Don't call base class method";
        return;
    }

    QGraphicsTextItem::keyReleaseEvent(event);
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
        setTextInteractionFlags(Qt::NoTextInteraction);
    }

    Delegate()->postpaint(painter, option, widget);
}


UBItem* UBGraphicsTextItem::deepCopy() const
{
    UBGraphicsTextItem* copy = new UBGraphicsTextItem();

    copyItemParameters(copy);

   // TODO UB 4.7 ... complete all members ?

   return copy;
}

void UBGraphicsTextItem::copyItemParameters(UBItem *copy) const
{
    UBGraphicsTextItem *cp = dynamic_cast<UBGraphicsTextItem*>(copy);
    if (cp)
    {
        cp->setHtml(toHtml());
        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
        cp->setData(UBGraphicsItemData::ItemEditable, data(UBGraphicsItemData::ItemEditable).toBool());
        cp->setTextWidth(this->textWidth());
        cp->setTextHeight(this->textHeight());

        cp->setSourceUrl(this->sourceUrl());
    }
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
    qreal strictMin = 155; // the size of the font customization panel
    qreal newWidth = qMax(strictMin, width);

    QGraphicsTextItem::setTextWidth(newWidth);
}


void UBGraphicsTextItem::setTextHeight(qreal height)
{
    QFontMetrics fm(font());
    qreal minHeight = fm.height() + document()->documentMargin() * 2;
    mTextHeight = qMax(minHeight, height);
    update();
    setFocus();
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
        resize(textWidth(),textHeight());
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

    if (Delegate())
        Delegate()->positionHandles();
}


QSizeF UBGraphicsTextItem::size() const
{
    return QSizeF(textWidth(), textHeight());
}

void UBGraphicsTextItem::setUuid(const QUuid &pUuid)
{
    UBItem::setUuid(pUuid);
    setData(UBGraphicsItemData::ItemUuid, QVariant(pUuid)); //store item uuid inside the QGraphicsItem to fast operations with Items on the scene
}


void UBGraphicsTextItem::undoCommandAdded()
{
    emit textUndoCommandAdded(this);
}


void UBGraphicsTextItem::documentSizeChanged(const QSizeF & newSize)
{
    resize(newSize.width(), newSize.height());
}

void UBGraphicsTextItem::setHtml(const QString &text)
{
    QGraphicsTextItem::setHtml(text);
    setTextInteractionFlags(Qt::NoTextInteraction);
}
