/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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

#include "document/UBDocumentProxy.h"
#include "core/UBApplication.h"
#include "document/UBDocumentController.h"
#include "board/UBBoardController.h"
#include "document/UBDocumentProxy.h"
#include "customWidgets/UBGraphicsItemAction.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBPersistenceManager.h"
#include "core/UBTextTools.h"

#include "gui/UBCreateTablePalette.h"
#include "core/memcheck.h"

#include "QDomDocument"

QColor UBGraphicsTextItem::lastUsedTextColor;

UBGraphicsTextItem::UBGraphicsTextItem(QGraphicsItem * parent) :
    QGraphicsTextItem(parent)
    , UBGraphicsItem()
    , mMultiClickState(0)
    , mLastMousePressTime(QTime::currentTime())
    , mBackgroundColor(QColor(Qt::transparent))
    , mHtmlIsInterpreted(false)
{

    toolbarExtended = false;

    setDelegate(new UBGraphicsTextItemDelegate(this, 0));

    // TODO claudio remove this because in contrast with the fact the frame should be created on demand.
    Delegate()->setUBFlag(GF_FLIPPABLE_ALL_AXIS, false);
    Delegate()->setUBFlag(GF_REVOLVABLE, true);
    Delegate()->setUBFlag(GF_TOOLBAR_USED, true);
    Delegate()->createControls();
    //Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);

    /*setDelegate(new UBGraphicsTextItemDelegate(this));
    Delegate()->init();

    Delegate()->frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);
    Delegate()->setFlippable(false);
    Delegate()->setRotatable(true);*/
    Delegate()->setCanTrigAnAction(true);

    mTypeTextHereLabel = tr("<Type Text Here>");


    setData(UBGraphicsItemData::ItemLayerType, UBItemLayerType::Object);
    setData(UBGraphicsItemData::itemLayerType, QVariant(itemLayerType::ObjectItem)); //Necessary to set if we want z value to be assigned correctly


    setFlag(QGraphicsItem::ItemIsSelectable, true);
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

    setTextInteractionFlags(Qt::TextEditorInteraction);

    //issue 1554 - NNE - 20131008
    isActivatedTextEditor = true;

    setUuid(QUuid::createUuid());

    connect(document(), SIGNAL(contentsChanged()), Delegate(), SLOT(contentsChanged()));
    connect(document(), SIGNAL(undoCommandAdded()), this, SLOT(undoCommandAdded()));
    connect(this, SIGNAL(linkActivated(QString)), this, SLOT(loadUrl(QString)));


    connect(document()->documentLayout(), SIGNAL(documentSizeChanged(const QSizeF &)),
            this, SLOT(documentSizeChanged(const QSizeF &)));

    connect(UBApplication::boardController->controlView(), SIGNAL(clickOnBoard()), this, SLOT(changeHTMLMode()));

    connect(this, SIGNAL(linkHovered(QString)), this, SLOT(onLinkHovered(QString))); // ALTI/AOU - 20140602 : make possible to click on Links with Play tool


}

UBGraphicsTextItem::~UBGraphicsTextItem()
{
}

// Issue 16/03/2018 - OpenBoard - Two text-editor toolboards.
bool UBGraphicsTextItem::isToolbarExtended(){
    return toolbarExtended;
}

// Issue 16/03/2018 - OpenBoard - Two text-editor toolboards.
void UBGraphicsTextItem::setToolbarExtended(bool mode){
    toolbarExtended = mode;
}

void UBGraphicsTextItem::insertColumn(bool onRight)
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        //compute the new size of each column
        QVector<QTextLength> width = t->format().toTableFormat().columnWidthConstraints();

        //the reference size is the current column where the cursor is
        qreal widthColumn = width.at(t->cellAt(textCursor()).column()).rawValue();

        qreal w = 100 + widthColumn;

        QVector<QTextLength> newWidth;

        for(int i = 0; i < width.size(); i++){
            qreal currentWidth = width.at(i).rawValue();

            newWidth.push_back(QTextLength(QTextLength::PercentageLength, currentWidth / w * 100));
        }


        if (onRight){
            int pos = t->cellAt(textCursor()).column()+1;
            newWidth.insert(pos, QTextLength(QTextLength::PercentageLength, widthColumn / w *100));
            t->insertColumns(pos, 1);
        }else{
            int pos = t->cellAt(textCursor()).column();
            newWidth.insert(pos, QTextLength(QTextLength::PercentageLength, widthColumn / w *100));
            t->insertColumns(pos, 1);
        }

        //Apply the new constaints
        QTextTableFormat f = t->format();
        f.clearColumnWidthConstraints();
        f.setWidth(QTextLength(QTextLength::PercentageLength, 100));
        f.setColumnWidthConstraints(newWidth);
        t->setFormat(f);
    }
}

void UBGraphicsTextItem::distributeColumn()
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        //compute the new size of each column
        QVector<QTextLength> width = t->format().toTableFormat().columnWidthConstraints();

        QVector<QTextLength> newWidth;

        const qreal w = 100.f / width.size();

        for(int i = 0; i < width.size(); i++){
            newWidth.push_back(QTextLength(QTextLength::PercentageLength, w));
        }

        //Apply the new constaints
        QTextTableFormat f = t->format();
        f.clearColumnWidthConstraints();
        f.setWidth(QTextLength(QTextLength::PercentageLength, 100));
        f.setColumnWidthConstraints(newWidth);
        t->setFormat(f);
    }
}

void UBGraphicsTextItem::insertRow(bool onRight)
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        if (onRight)
            t->insertRows(t->cellAt(textCursor()).row()+1, 1);
        else
            t->insertRows(t->cellAt(textCursor()).row(), 1);
    }
}

void UBGraphicsTextItem::deleteColumn()
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        //compute the new size of each column
        QVector<QTextLength> width = t->format().toTableFormat().columnWidthConstraints();

        //the reference size is the current column where the cursor is
        qreal widthColumn = width.at(t->cellAt(textCursor()).column()).rawValue();

        qreal w = 100 - widthColumn;

        QVector<QTextLength> newWidth;

        for(int i = 0; i < width.size(); i++){
            qreal currentWidth = width.at(i).rawValue();

            newWidth.push_back(QTextLength(QTextLength::PercentageLength, currentWidth / w * 100));
        }

        newWidth.remove(t->cellAt(textCursor()).column());

        t->removeColumns(t->cellAt(textCursor()).column(), 1);

        //Apply the new constaints
        QTextTableFormat f = t->format();
        f.clearColumnWidthConstraints();
        f.setWidth(QTextLength(QTextLength::PercentageLength, 100));
        f.setColumnWidthConstraints(newWidth);
        t->setFormat(f);
    }
}

void UBGraphicsTextItem::deleteRow()
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        t->removeRows(t->cellAt(textCursor()).row(), 1);
    }
}


void UBGraphicsTextItem::setCellWidth(int percent)
{
    QTextTable* t = textCursor().currentTable();
    if (t)
    {
        //compute the new size of each column
        QVector<QTextLength> width = t->format().toTableFormat().columnWidthConstraints();

        int column = t->cellAt(textCursor()).column();
        qreal remainingSpace = 100 - percent;
        qreal oldRemaingSpace = 100 - width.at(column).rawValue();

        QVector<QTextLength> newWidth;

        for(int i = 0; i < width.size(); i++){
            if(i != column){
                qreal currentWidth = width.at(i).rawValue();

                newWidth.push_back(QTextLength(QTextLength::PercentageLength, currentWidth / oldRemaingSpace * remainingSpace));
            }
        }

        newWidth.insert(column, QTextLength(QTextLength::PercentageLength, percent));

        QTextTableFormat f = t->format();
        f.clearColumnWidthConstraints();
        f.setWidth(QTextLength(QTextLength::PercentageLength, 100));
        f.setColumnWidthConstraints(newWidth);
        t->setFormat(f);
    }
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
    // It is a cludge...
    if (UBStylusTool::Play == UBDrawingController::drawingController()->stylusTool())
    {
        QGraphicsTextItem::mousePressEvent(event);
        event->accept();
        clearFocus();
        Delegate()->startUndoStep(); //Issue 1541 - AOU - 20131002
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
        else
        {
            Delegate()->getToolBarItem()->show();
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
        //issue 1554 - NNE - 20131008
        activateTextEditor(true);

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

void UBGraphicsTextItem::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{

}

void UBGraphicsTextItem::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    // scene()->itemAt(pos) returns 0 if pos is not over text, but over text item, but mouse press comes.
    // It is a cludge...
    if (UBStylusTool::Play == UBDrawingController::drawingController()->stylusTool())
    {
        QPointF distance = event->pos() - event->lastPos();
        if( fabs(distance.x()) < 1 && fabs(distance.y()) < 1
                && mCurrentLinkUrl.isEmpty()){ // ALTI/AOU - 20140602 : make possible to click on Links with Play tool
            if(Delegate()->action()){
                Delegate()->action()->play();
            }
            Delegate()->mouseReleaseEvent(event);
        }
        else{
            QGraphicsTextItem::mouseReleaseEvent(event);
        }

        event->accept();
        clearFocus();
        return;
    }

    if (mMultiClickState == 1)
    {
        if (Delegate())
            Delegate()->mouseReleaseEvent(event);

        QGraphicsTextItem::mouseReleaseEvent(event);
    }
    else
    {
        event->accept();
    }
}

void UBGraphicsTextItem::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    if (mBackgroundColor != Qt::transparent && !mHtmlIsInterpreted)
    {
        painter->setPen(Qt::transparent);
        painter->setBrush(mBackgroundColor);
        painter->drawRect(boundingRect());
    }

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

bool UBGraphicsTextItem::htmlMode() const
{
    return mHtmlIsInterpreted;
}

void UBGraphicsTextItem::setHtmlMode(const bool mode)
{
    mHtmlIsInterpreted = mode;
}

void UBGraphicsTextItem::loadUrl(QString url)
{
    UBApplication::loadUrl(url);
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
        if(htmlMode()){
            UBGraphicsTextItemDelegate* d = dynamic_cast<UBGraphicsTextItemDelegate*>(Delegate());
            if(d){
                d->alternHtmlMode();
            }
        }

        QString html = toHtml();

        cp->loadImages(html, true);

        cp->setHtml(html);

        cp->setPos(this->pos());
        cp->setTransform(this->transform());
        cp->setFlag(QGraphicsItem::ItemIsMovable, true);
        cp->setFlag(QGraphicsItem::ItemIsSelectable, true);
        cp->setData(UBGraphicsItemData::ItemLayerType, this->data(UBGraphicsItemData::ItemLayerType));
        cp->setData(UBGraphicsItemData::ItemLocked, this->data(UBGraphicsItemData::ItemLocked));
        cp->setData(UBGraphicsItemData::ItemEditable, data(UBGraphicsItemData::ItemEditable).toBool());
        cp->setTextWidth(this->textWidth());
        cp->setTextHeight(this->textHeight());

        if(mBackgroundColor != Qt::transparent)
            cp->setBackgroundColor(mBackgroundColor);

        // Issue 13/03/2018 - OpenBoard - Custom Widget.
        if(Delegate()->action()){
            if(Delegate()->action()->linkType() == eLinkToAudio){
                UBGraphicsItemPlayAudioAction* audioAction = dynamic_cast<UBGraphicsItemPlayAudioAction*>(Delegate()->action());
                UBGraphicsItemPlayAudioAction* action = new UBGraphicsItemPlayAudioAction(audioAction->fullPath());
                cp->Delegate()->setAction(action);
            }
            else
                cp->Delegate()->setAction(Delegate()->action());
        }
        // END Issue 13/03/2018 - OpenBoard - Custom Widget.
    }
}

QRectF UBGraphicsTextItem::boundingRect() const
{
    qreal width = textWidth();
    qreal height = textHeight();
    // Issue 16/03/2018 - OpenBoard - Fixed width of the TEXEDITOR WINDOW to avoid problems with no-hidden icons
    if((toolbarExtended == true) && (width < MIN_TEXT_WIDTH_EXTENDED) ) width=MIN_TEXT_WIDTH_EXTENDED;
    if((toolbarExtended == false) && (width < MIN_TEXT_WIDTH_REDUCED) ) width=MIN_TEXT_WIDTH_REDUCED;
    if(height < MIN_TEXT_HEIGHT) height=MIN_TEXT_HEIGHT;

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
    prepareGeometryChange();

    QFontMetrics fm(font());
    qreal minHeight = fm.height() + document()->documentMargin() * 2;
    mTextHeight = qMax(minHeight, height);
    setFocus();
}


qreal UBGraphicsTextItem::textHeight() const
{
    return mTextHeight;
}


/**
 * @brief Get the ratio between font size in pixels and points.
 * @return The ratio of pixel size to point size of the first character, or 0 if the text item is empty.
 *
 * Qt may display fonts differently on different platforms -- on the same display,
 * the same point size may be displayed at different pixel sizes. This function returns the
 * ratio of pixel size to point size, based on the first character in the text item.
 */
qreal UBGraphicsTextItem::pixelsPerPoint() const
{
    QTextCursor cursor = textCursor();
    if (cursor.isNull())
        return 0;

    cursor.movePosition(QTextCursor::Start, QTextCursor::MoveAnchor);
    cursor.movePosition(QTextCursor::NextCharacter, QTextCursor::KeepAnchor);

    QFont f = cursor.charFormat().font();
    qDebug() << "ppp. Font: " << f;
    QFontInfo fi(cursor.charFormat().font());

    qreal pixelSize = fi.pixelSize();
    qreal pointSize = fi.pointSizeF();

    //qDebug() << "Pixel size: " << pixelSize;
    //qDebug() << "Point size: " << pointSize;

    if (pointSize == 0)
        return 0;

    return pixelSize/pointSize;
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

void UBGraphicsTextItem::insertImage(QString src)
{
    //retour chariot avant de placer l'image
    textCursor().insertText("\n");

    QImage img = QImage(src);
    QString fileExtension = UBFileSystemUtils::extension(src);
    QString filename = UBPersistenceManager::imageDirectory + "/" + QUuid::createUuid().toString() + "." + fileExtension;
    QString dest = UBApplication::boardController->selectedDocument()->persistencePath() + "/" + filename;

    if (!UBFileSystemUtils::copy(src, dest, true))
        qDebug() << "UBFileSystemUtils::copy error";

    document()->addResource(QTextDocument::ImageResource, QUrl(filename), img);
    textCursor().insertImage(filename);
}

void UBGraphicsTextItem::insertTable(const int lines, const int columns)
{
    QTextCursor cursor = textCursor();

    QTextTableFormat format;
    format.clearColumnWidthConstraints();
    format.setWidth(QTextLength(QTextLength::PercentageLength, 100));
    QTextLength t = QTextLength(QTextLength::PercentageLength, 100/(float)columns);
    QVector<QTextLength> v;
    for (int i=0; i< columns; i++)
        v.push_back(t);
    format.setColumnWidthConstraints(v);
    format.merge(cursor.charFormat());
    cursor.insertTable(lines,columns,format);
}

void UBGraphicsTextItem::setBackgroundColor(const QColor& color)
{
    QTextBlockFormat format;
    format.setBackground(QBrush(color));

    QTextCursor cursor = textCursor();

    QTextTable* t = cursor.currentTable();
    int firstRow,numRows,firstCol,numCols;
    cursor.selectedTableCells(&firstRow,&numRows,&firstCol,&numCols);
    if (firstRow < 0)
    {
        if (t)
        {
            QTextTableCell c = t->cellAt(cursor);
            QTextCharFormat format;
            format.setBackground(QBrush(color));
            c.setFormat(format);
        }
        else
            mBackgroundColor = color;
    }
    else
    {
        for (int i = 0; i < numRows; i++)
        {
            for (int j = 0; j < numCols; j++)
            {
                QTextTableCell c = t->cellAt(firstRow+i,firstCol+j);
                QTextCharFormat format;
                format.setBackground(QBrush(color));
                c.setFormat(format);
            }
        }
    }

}

void UBGraphicsTextItem::setForegroundColor(const QColor& color)
{
    QTextBlockFormat format;
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
}

void UBGraphicsTextItem::setAlignmentToLeft()
{
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignLeft);
    QTextCursor cursor = textCursor();    
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
    setFocus();
}

// Issue 12/03/2018 - OpenBoard - TEXT EDITOR - NEW JUSTIFY BUTTON
void UBGraphicsTextItem::setAlignmentToJustify()
{
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignJustify);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
    setFocus();
}

void UBGraphicsTextItem::setAlignmentToCenter()
{
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignCenter);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
    setFocus();
}

void UBGraphicsTextItem::setAlignmentToRight()
{
    QTextBlockFormat format;
    format.setAlignment(Qt::AlignRight);
    QTextCursor cursor = textCursor();
    cursor.mergeBlockFormat(format);
    setTextCursor(cursor);
    setFocus();
}

UBGraphicsScene* UBGraphicsTextItem::scene()
{
    return static_cast<UBGraphicsScene*>(QGraphicsItem::scene());
}


void UBGraphicsTextItem::resize(qreal width, qreal height)
{
    // Issue 16/03/2018 - OpenBoard - Fixed width of the TEXEDITOR WINDOW to avoid problems with no-hidden icons
    qreal w = width;
    qreal h = height;
    if(isToolbarExtended() == true && w < MIN_TEXT_WIDTH_EXTENDED ) w=MIN_TEXT_WIDTH_EXTENDED;
    if(isToolbarExtended() == false && w < MIN_TEXT_WIDTH_REDUCED ) w=MIN_TEXT_WIDTH_REDUCED;
    if(h < MIN_TEXT_HEIGHT) h=MIN_TEXT_HEIGHT;

    setTextWidth(w);
    setTextHeight(h);
    if (Delegate())
    {
        UBGraphicsTextItemDelegate* textDelegate = dynamic_cast<UBGraphicsTextItemDelegate*>(Delegate());
        if (textDelegate)
        {
            QSize tablePaletteSize = textDelegate->linkPalette()->size();
            textDelegate->tablePalette()->setPos(QPoint((w-tablePaletteSize.width())/2, (h-tablePaletteSize.height())/2));
            textDelegate->linkPalette()->setPos(QPoint((w-tablePaletteSize.width())/2, (h-tablePaletteSize.height())/2));
            textDelegate->cellPropertiesPalette()->setPos(QPoint((w-tablePaletteSize.width())/2, (h-tablePaletteSize.height())/2));
        }
        Delegate()->positionHandles();
    }

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

//issue 1554 - NNE - 20131009
void UBGraphicsTextItem::activateTextEditor(bool activateTextEditor)
{
    qDebug() << textInteractionFlags();
    this->isActivatedTextEditor = activateTextEditor;

    if(!activateTextEditor){
        //qWarning()<<activateTextEditor;
        setTextInteractionFlags(Qt::TextSelectableByMouse | Qt::TextBrowserInteraction);
        if(htmlMode()){
            UBGraphicsTextItemDelegate *d = dynamic_cast<UBGraphicsTextItemDelegate*>(Delegate());
            if(d) d->alternHtmlMode();
        }
    }else{
        setTextInteractionFlags(Qt::TextEditorInteraction | Qt::TextBrowserInteraction);
    }
}
//issue 1554 - NNE - 20131009 : END

//issue 1539 - NNE - 20131018
void UBGraphicsTextItem::keyPressEvent(QKeyEvent *event)
{
    if(event->matches(QKeySequence::Paste)){
        //if the data comes from the RTE, we accept the data
        //else we clean the html
        QClipboard *clipboard = QApplication::clipboard();
        const QMimeData *mimeData = clipboard->mimeData();

        if(mimeData->data("text/copySource").contains("Texte Enrichi.wgt")){
            QString html = mimeData->html();

            QMimeData *cpMime = new QMimeData();

            formatTable(html);

            removeTextBackgroundColor(html);

            loadImages(html);

            formatParagraph(html);

            formatList(html);

            cpMime->setHtml(html);

            clipboard->setMimeData(cpMime);
        }else{
            UBTextTools::cleanHtmlClipboard();
        }
    }
    QGraphicsTextItem::keyPressEvent(event);
}
//issue 1539 - NNE - 20131018 : END

void UBGraphicsTextItem::keyReleaseEvent(QKeyEvent *event)
{
    if (Delegate() && !Delegate()->keyReleaseEvent(event)) {
        qDebug() << "UBGraphicsTextItem::keyPressEvent(QKeyEvent *event) has been rejected by delegate. Don't call base class method";
        return;
    }

    QGraphicsTextItem::keyReleaseEvent(event);
}

//issue 1539 - NNE - 20131211
void UBGraphicsTextItem::contextMenuEvent(QGraphicsSceneContextMenuEvent *event)
{
    UBTextTools::cleanHtmlClipboard();
    QGraphicsTextItem::contextMenuEvent(event);
}

//N/C - NNE - 20140520
QString UBGraphicsTextItem::formatTable(QString& source)
{
    int index = 0;
    while((index = source.indexOf("<table", index)) != -1){
        source.insert(index + 6, " width=\"100%\" border=\"1\"");
        index += 6;
    }

    int currentPos = 0;
    while((currentPos = source.indexOf("<td", currentPos)) != -1){
        currentPos += 3;

        //Check for the width attribute
        //conversion from "width: xxx%" to "width='xxx%'"
        int widthIndex = source.indexOf("width: ", currentPos);
        int endTdTag = source.indexOf(">", currentPos);

        if(widthIndex != -1 && widthIndex < endTdTag){
            int j = widthIndex + 7;
            QString v;

            while(source.at(j).isDigit()){
                v += source.at(j);
                j++;
            }

            if(source.at(j) == '%'){
                QString insert = " width=\"" + v + "%\"";
                source.insert(currentPos, insert);
            }
        }

        //Check for the text-align attribute
        int textAlignIndex  = source.indexOf("text-align: ", currentPos);

        //update the position
        endTdTag = source.indexOf(">", currentPos);

        if(textAlignIndex != -1 && textAlignIndex < endTdTag){
            int endAttribute = source.indexOf(';', textAlignIndex);

            //prevent error if the ';' isn't find
            if(endAttribute < endTdTag){
                QString value = source.mid(textAlignIndex, endAttribute - textAlignIndex);

                value.replace("text-align: ", " align=\"");
                value += "\" ";

                source.insert(currentPos, value);
            }
        }
    }

    return source;
}

QString UBGraphicsTextItem::removeTextBackgroundColor(QString& source)
{
    int currentPos = 0;

    while((currentPos = source.indexOf("<span", currentPos)) != -1){
        int spanTagEnd = source.indexOf(">", currentPos + 1);

        QString spanContent = source.mid(currentPos, spanTagEnd);
        qDebug() << spanContent;

        int backgroundIdx;
        if((backgroundIdx = spanContent.indexOf("background-color")) != -1 && backgroundIdx < spanTagEnd){
            //remove the span tag
            int endBackground = spanContent.indexOf(";", backgroundIdx);

            for(int i = 0; i < endBackground - backgroundIdx + 1; i++){
                source[backgroundIdx + currentPos + i] =  ' ';
            }
        }

        currentPos += 5;
    }

    return source;
}

QString UBGraphicsTextItem::loadImages(QString& source, bool onlyLoad)
{
    int currentPos = 0;
    while((currentPos = source.indexOf("src", currentPos)) != -1){
        int startPath = source.indexOf("\"", currentPos) + 1;

        QString path;
        while(source.at(startPath) != '\"'){
            path += source.at(startPath);
            startPath++;
        }

        if(!path.isEmpty()){
            if(onlyLoad){
                QImage img = QImage(UBApplication::boardController->selectedDocument()->persistencePath() + "/" + path);
                document()->addResource(QTextDocument::ImageResource, QUrl(path), img);
            }else{
                QImage img = QImage(path);
                QString fileExtension = UBFileSystemUtils::extension(path);
                QString filename = UBPersistenceManager::imageDirectory + "/" + QUuid::createUuid().toString() + "." + fileExtension;
                QString dest = UBApplication::boardController->selectedDocument()->persistencePath() + "/" + filename;

                if (!UBFileSystemUtils::copy(path, dest, true))
                    qDebug() << "UBFileSystemUtils::copy error";

                document()->addResource(QTextDocument::ImageResource, QUrl(filename), img);
                source.replace(path, filename);
            }

        }

        currentPos += 3;
    }

    return source;
}

QString UBGraphicsTextItem::formatParagraph(QString& source)
{
    return findAndReplaceAttribute("p", "text-align", "align", source);
}

QString UBGraphicsTextItem::formatList(QString& source)
{
    findAndReplaceAttribute("li", "text-align", "align", source);

    return findAndReplaceAttribute("ol", "text-align", "align", source);
}


QString UBGraphicsTextItem::findAndReplaceAttribute(QString tag, QString oldAttribute, QString newAttribute, QString& source)
{
    int currentPos = 0;

    tag = "<" + tag;

    while((currentPos = source.indexOf(tag, currentPos)) != -1){
        currentPos += tag.size();

        int endTag = source.indexOf(">", currentPos);

        int attributeIndex = source.indexOf(oldAttribute + ": ", currentPos);

        if(attributeIndex < endTag){
            int endAttribute = source.indexOf(';', attributeIndex);

            if(endAttribute < endTag){
                QString value = source.mid(attributeIndex, endAttribute - attributeIndex);

                value.replace(oldAttribute+": ", " " + newAttribute + "=\"");
                value += "\"";

                source.insert(currentPos, value);
            }
        }
    }

    return source;
}

//N/C - NNE - 20140520

void UBGraphicsTextItem::changeHTMLMode()
{
    //qWarning()<<"changeHTMLMode";
    //qWarning()<<isActivatedTextEditor;
    if(isActivatedTextEditor)
        activateTextEditor(false);

    if(htmlMode()){
        UBGraphicsTextItemDelegate *d = dynamic_cast<UBGraphicsTextItemDelegate*>(Delegate());
        if(d) d->alternHtmlMode();
    }
}

void UBGraphicsTextItem::onLinkHovered(const QString & currentLinkUrl)  // ALTI/AOU - 20140602 : make possible to click on Links with Play tool
{
   mCurrentLinkUrl = currentLinkUrl; // used in mouseReleaseEvent()
}
