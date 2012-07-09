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

#include <QtGui>
#include <QtSvg>

#include "core/UBApplication.h"
#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsTextItemDelegate.h"
#include "UBGraphicsScene.h"
#include "gui/UBResources.h"

#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsDelegateFrame.h"
#include "core/UBSettings.h"

#include "board/UBBoardController.h"

#include "core/memcheck.h"

const int UBGraphicsTextItemDelegate::sMinPixelSize = 8;
const int UBGraphicsTextItemDelegate::sMinPointSize = 8;

UBGraphicsTextItemDelegate::UBGraphicsTextItemDelegate(UBGraphicsTextItem* pDelegated, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated,0, parent, true)
    , mLastFontPixelSize(-1)
    , delta(5)
{
    delegated()->setData(UBGraphicsItemData::ItemEditable, QVariant(true));
    delegated()->setPlainText("");

    QTextCursor curCursor = delegated()->textCursor();
    QTextCharFormat format;
    QFont font(createDefaultFont());
    font.setPointSize(UBSettings::settings()->fontPointSize());

    format.setFont(font);
    curCursor.mergeCharFormat(format);
    delegated()->setTextCursor(curCursor);
    delegated()->setFont(font);

    delegated()->adjustSize();
    delegated()->contentsChanged();

    // NOOP
}

UBGraphicsTextItemDelegate::~UBGraphicsTextItemDelegate()
{
    // NOOP
}

QFont UBGraphicsTextItemDelegate::createDefaultFont()
{
    QTextCharFormat textFormat;

    QString fFamily = UBSettings::settings()->fontFamily();
    if (!fFamily.isEmpty())
        textFormat.setFontFamily(fFamily);

    bool bold = UBSettings::settings()->isBoldFont();
    if (bold)
        textFormat.setFontWeight(QFont::Bold);

    bool italic = UBSettings::settings()->isItalicFont();
    if (italic)
        textFormat.setFontItalic(true);

    QFont font(fFamily, -1, bold ? QFont::Bold : -1, italic);
//    int pixSize = UBSettings::settings()->fontPixelSize();
//    if (pixSize > 0) {
//        mLastFontPixelSize = pixSize;
//        font.setPixelSize(pixSize);
//    }
    int pointSize = UBSettings::settings()->fontPointSize();
    if (pointSize > 0) {
//        mLastFontPixelSize = pointSize;
        font.setPointSize(pointSize);
    }

    return font;
}

void UBGraphicsTextItemDelegate::buildButtons()
{
    UBGraphicsItemDelegate::buildButtons();

    mFontButton = new DelegateButton(":/images/font.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mColorButton = new DelegateButton(":/images/color.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mDecreaseSizeButton = new DelegateButton(":/images/minus.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mIncreaseSizeButton = new DelegateButton(":/images/plus.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);

    connect(mFontButton, SIGNAL(clicked(bool)), this, SLOT(pickFont()));
    connect(mColorButton, SIGNAL(clicked(bool)), this, SLOT(pickColor()));
    connect(mDecreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(decreaseSize()));
    connect(mIncreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(increaseSize()));

    QList<QGraphicsItem*> itemsOnToolBar;
    itemsOnToolBar << mFontButton << mColorButton << mDecreaseSizeButton << mIncreaseSizeButton;
    mToolBarItem->setItemsOnToolBar(itemsOnToolBar);
    mToolBarItem->setShifting(true);
    mToolBarItem->setVisibleOnBoard(true);
}

void UBGraphicsTextItemDelegate::contentsChanged()
{
    positionHandles();
    delegated()->contentsChanged();
}

// This method is used to filter the available fonts. Only the web-compliant fonts
// will remain in the font list.
void UBGraphicsTextItemDelegate::customize(QFontDialog &fontDialog)
{
    fontDialog.setOption(QFontDialog::DontUseNativeDialog);

    if (UBSettings::settings()->isDarkBackground()) {
        fontDialog.setStyleSheet("background-color: white;");
    }

    QListView *fontNameListView;
    QList<QListView*> listViews = fontDialog.findChildren<QListView*>();
    if (listViews.count() > 0)
    {
        fontNameListView = listViews.at(0);
        foreach (QListView* listView, listViews)
        {
            if (listView->pos().x() < fontNameListView->pos().x())
                fontNameListView = listView;
        }
    }
    if (fontNameListView)
    {
        QStringListModel *stringListModel = dynamic_cast<QStringListModel*>(fontNameListView->model());
        if (stringListModel)
        {
            QStringList dialogFontNames = stringListModel->stringList();
            QStringList safeWebFontNames;
            safeWebFontNames.append("Arial");
            safeWebFontNames.append("Arial Black");
            safeWebFontNames.append("Comic Sans MS");
            safeWebFontNames.append("Courier New");
            safeWebFontNames.append("Georgia");
            safeWebFontNames.append("Impact");
            safeWebFontNames.append("Times New Roman");
            safeWebFontNames.append("Trebuchet MS");
            safeWebFontNames.append("Verdana");

            QStringList customFontList =  UBResources::resources()->customFontList();
            int index = 0;
            foreach (QString dialogFontName, dialogFontNames){
                if (safeWebFontNames.contains(dialogFontName, Qt::CaseInsensitive) || customFontList.contains(dialogFontName, Qt::CaseSensitive))
                    index++;
                else
                    stringListModel->removeRow(index);
            }
        }
    }
    QList<QComboBox*> comboBoxes = fontDialog.findChildren<QComboBox*>();
    if (comboBoxes.count() > 0)
        comboBoxes.at(0)->setEnabled(false);
}


void UBGraphicsTextItemDelegate::pickFont()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        QFontDialog fontDialog(delegated()->textCursor().charFormat().font(), mDelegated->scene()->views().at(0));
        customize(fontDialog);

        if (fontDialog.exec())
        {
            QFont selectedFont = fontDialog.selectedFont();
            UBSettings::settings()->setFontFamily(selectedFont.family());
            UBSettings::settings()->setBoldFont(selectedFont.bold());
            UBSettings::settings()->setItalicFont(selectedFont.italic());
            UBSettings::settings()->setFontPointSize(selectedFont.pointSize());

            //setting format for selected item
            QTextCursor curCursor = delegated()->textCursor();
            QTextCharFormat format;
            format.setFont(selectedFont);
            curCursor.mergeCharFormat(format);

            delegated()->setTextCursor(curCursor);
            delegated()->setFont(selectedFont);
            delegated()->setSelected(true);
            delegated()->document()->adjustSize();
            delegated()->contentsChanged();
        }
    }
}

void UBGraphicsTextItemDelegate::pickColor()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        QColorDialog colorDialog(delegated()->defaultTextColor(), mDelegated->scene()->views().at(0));
        colorDialog.setWindowTitle(tr("Text Color"));
        if (UBSettings::settings()->isDarkBackground())
        {
            colorDialog.setStyleSheet("background-color: white;");
        }

        if (colorDialog.exec())
        {
            QColor selectedColor = colorDialog.selectedColor();
            delegated()->setDefaultTextColor(selectedColor);
//            delegated()->setColorOnDarkBackground(selectedColor);
//            delegated()->setColorOnLightBackground(selectedColor);
            QTextCursor curCursor = delegated()->textCursor();
            QTextCharFormat format;
            format.setForeground(QBrush(selectedColor));
            curCursor.mergeCharFormat(format);
            delegated()->setTextCursor(curCursor);

            UBGraphicsTextItem::lastUsedTextColor = selectedColor;

            delegated()->setSelected(true);
            delegated()->contentsChanged();
        }
    }
}

void UBGraphicsTextItemDelegate::decreaseSize()
{
    ChangeTextSize(-delta);
}

void UBGraphicsTextItemDelegate::increaseSize()
{
   ChangeTextSize(delta);
}

UBGraphicsTextItem* UBGraphicsTextItemDelegate::delegated()
{
    return static_cast<UBGraphicsTextItem*>(mDelegated);
}
void UBGraphicsTextItemDelegate::setEditable(bool editable)
{
    if (editable) {
        delegated()->setTextInteractionFlags(Qt::TextEditorInteraction);
        mDelegated->setData(UBGraphicsItemData::ItemEditable, QVariant(true));
    } else {
        QTextCursor cursor(delegated()->document());
        cursor.clearSelection();
        delegated()->setTextCursor(cursor);

        delegated()->setTextInteractionFlags(Qt::NoTextInteraction);
        mDelegated->setData(UBGraphicsItemData::ItemEditable, QVariant(false));
    }
}
bool UBGraphicsTextItemDelegate::isEditable()
{
    return mDelegated->data(UBGraphicsItemData::ItemEditable).toBool();
}
void UBGraphicsTextItemDelegate::decorateMenu(QMenu *menu)
{
    UBGraphicsItemDelegate::decorateMenu(menu);

    mEditableAction = menu->addAction(tr("Editable"), this, SLOT(setEditable(bool)));
    mEditableAction->setCheckable(true);
    mEditableAction->setChecked(isEditable());

}
void UBGraphicsTextItemDelegate::updateMenuActionState()
{
    UBGraphicsItemDelegate::updateMenuActionState();
}
void UBGraphicsTextItemDelegate::positionHandles()
{
    UBGraphicsItemDelegate::positionHandles();

    if (mDelegated->isSelected() || (mDelegated->parentItem() && UBGraphicsGroupContainerItem::Type == mDelegated->parentItem()->type())) 
    {
        if (mToolBarItem->isVisibleOnBoard())
        {
            qreal AntiScaleRatio = 1 / (UBApplication::boardController->systemScaleFactor() * UBApplication::boardController->currentZoom());    
            mToolBarItem->setScale(AntiScaleRatio);
            QRectF toolBarRect = mToolBarItem->rect();
            toolBarRect.setWidth(delegated()->boundingRect().width()/AntiScaleRatio);
            mToolBarItem->setRect(toolBarRect);           
            mToolBarItem->positionHandles();
            mToolBarItem->update();
            if (mToolBarItem->isShifting())
                mToolBarItem->setPos(0,-mToolBarItem->boundingRect().height()*AntiScaleRatio);
            else
                mToolBarItem->setPos(0, 0);

            UBGraphicsGroupContainerItem *group = qgraphicsitem_cast<UBGraphicsGroupContainerItem*>(mDelegated->parentItem());

            mToolBarItem->hide();
            if (mToolBarItem->parentItem() && !mToolBarItem->parentItem()->data(UBGraphicsItemData::ItemLocked).toBool())
            {
                if (group && group->getCurrentItem() == mDelegated && group->isSelected())
                    mToolBarItem->show();

                if (!group)
                     mToolBarItem->show();
            }

        }
    }
    else
    {
        mToolBarItem->hide();
    }

    setEditable(isEditable());
}

void UBGraphicsTextItemDelegate::ChangeTextSize(int delta)
{
    if (0 == delta)
        return;

    QTextCursor cursor = delegated()->textCursor();
    QTextCharFormat textFormat;

    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();

    if (0 == anchorPos-cursorPos)
    {
        cursor.setPosition (0, QTextCursor::MoveAnchor);
        cursor.setPosition (cursor.document()->characterCount()-1, QTextCursor::KeepAnchor);
    }

    int startPos = qMin(cursor.anchor(), cursor.position());
    int endPos = qMax(cursor.anchor(), cursor.position());

    QFont curFont;
    bool bEndofTheSameBlock;
    int iBlockLen;
    int iPointSize;
    int iNextPointSize;
    int iCursorPos = startPos;

   // we search continuous blocks of the text with the same PointSize and allpy new settings for them.
    cursor.setPosition (startPos, QTextCursor::MoveAnchor);
    while(iCursorPos < endPos)
    {   
        bEndofTheSameBlock = false;
        iBlockLen = 0; 

        cursor.setPosition (iCursorPos+1, QTextCursor::KeepAnchor);
        iPointSize = cursor.charFormat().font().pointSize();

        cursor.setPosition (iCursorPos, QTextCursor::KeepAnchor);

        curFont = cursor.charFormat().font();

        do
        {
            iBlockLen++;

            cursor.setPosition (iCursorPos+iBlockLen+1, QTextCursor::KeepAnchor);
            iNextPointSize = cursor.charFormat().font().pointSize();

            cursor.setPosition (iCursorPos+iBlockLen, QTextCursor::KeepAnchor);
            if ((iPointSize != iNextPointSize)||(iCursorPos+iBlockLen >= endPos))
                bEndofTheSameBlock = true;

        }while(!bEndofTheSameBlock);


        //setting new parameners
        int iNewPointSize = iPointSize + delta;
        curFont.setPointSize( (iNewPointSize > 0)?iNewPointSize:1);
        textFormat.setFont(curFont);
        cursor.mergeCharFormat(textFormat);

        iCursorPos += iBlockLen;
        cursor.setPosition (iCursorPos, QTextCursor::MoveAnchor);
    }

    //delegated()->document()->adjustSize();
    delegated()->setFont(curFont);
    UBSettings::settings()->setFontPointSize(iPointSize);
    //returning initial selection
    cursor.setPosition (anchorPos, QTextCursor::MoveAnchor);
    cursor.setPosition (cursorPos, QTextCursor::KeepAnchor);

    delegated()->setTextCursor(cursor);
}
