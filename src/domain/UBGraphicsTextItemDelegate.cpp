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
#include <QtSvg>

#include "core/UBApplication.h"
#include "UBGraphicsGroupContainerItem.h"
#include "UBGraphicsTextItemDelegate.h"
#include "UBGraphicsScene.h"
#include "gui/UBResources.h"
#include "gui/UBMainWindow.h"

#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsDelegateFrame.h"
#include "domain/UBGraphicsProxyWidget.h"

#include "core/UBSettings.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"

#include "core/memcheck.h"

const int UBGraphicsTextItemDelegate::sMinPixelSize = 8;
const int UBGraphicsTextItemDelegate::sMinPointSize = 8;

UBGraphicsTextItemDelegate::UBGraphicsTextItemDelegate(UBGraphicsTextItem* pDelegated, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated,0, GF_COMMON | GF_REVOLVABLE | GF_TITLE_BAR_USED)
    , mLastFontPixelSize(-1)
    , delta(5)
    , mTablePalette(new UBCreateTablePalette())
    , mLinkPalette(new UBCreateHyperLinkPalette())
    , mCellPropertiesPalette(new UBCellPropertiesPalette())
{

    UBGraphicsProxyWidget* w = UBApplication::boardController->activeScene()->addWidget(mTablePalette);
    w->setParentItem(delegated());
    w->hide();
    w->setZValue(1); // ALTI/AOU - 20140610 : this widget appears on top of DelegateFrame and TextEditor Menu.
	
    UBGraphicsProxyWidget* w2 = UBApplication::boardController->activeScene()->addWidget(mLinkPalette);
    w2->setParentItem(delegated());
    w2->hide();
    w2->setZValue(1);   // ALTI/AOU - 20140610 : this widget appears on top of DelegateFrame and TextEditor Menu.

    UBGraphicsProxyWidget* w3 = UBApplication::boardController->activeScene()->addWidget(mCellPropertiesPalette);
    w3->setParentItem(delegated());
    w3->hide();
    w3->setZValue(1);   // ALTI/AOU - 20140610 : this widget appears on top of DelegateFrame and TextEditor Menu.

    delegated()->setData(UBGraphicsItemData::ItemEditable, QVariant(true));
    delegated()->setPlainText("");

    // Issue 16/03/2018 - OpenBoard - Two text-editor toolboards. (by default NOT EXTENDED)
    delegated()->setToolbarExtended(false);

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

    mTablePalette->move(delegated()->boundingRect().width()/2.0, 0 );
    mLinkPalette->move(delegated()->boundingRect().width()/2.0, 0 );
    mCellPropertiesPalette->move(delegated()->boundingRect().width()/2.0, 0 );

    connect(mTablePalette, SIGNAL(validationRequired()), this, SLOT(insertTable()));
    connect(mLinkPalette, SIGNAL(validationRequired()), this, SLOT(insertLink()));
    connect(mCellPropertiesPalette, SIGNAL(validationRequired()), this, SLOT(applyCellProperties()));
}

UBGraphicsTextItemDelegate::~UBGraphicsTextItemDelegate()
{

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
    int pointSize = UBSettings::settings()->fontPointSize();
    if (pointSize > 0) {
        font.setPointSize(pointSize);
    }

    return font;
}

void UBGraphicsTextItemDelegate::buildButtons()
{
    UBGraphicsItemDelegate::buildButtons();

    if(delegated()->isToolbarExtended()){
        buildButtonsExtended();
    }
    else{
        buildButtonsReduced();
    }
}

void UBGraphicsTextItemDelegate::buildButtonsReduced()
{
    mFontButton = new DelegateButton(":/images/textEditor/font.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mFontBoldButton = new DelegateButton(":/images/textEditor/bold.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mFontItalicButton = new DelegateButton(":/images/textEditor/italic.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mFontUnderlineButton = new DelegateButton(":/images/textEditor/underline.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mColorButton = new DelegateButton(":/images/color.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mSwtichTextEditorToolBar = new DelegateButton(":/images/textEditor/expand.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);

    connect(mFontButton, SIGNAL(clicked(bool)), this, SLOT(pickFont()));
    connect(mFontBoldButton, SIGNAL(clicked()), this, SLOT(setFontBold()));
    connect(mFontItalicButton, SIGNAL(clicked()), this, SLOT(setFontItalic()));
    connect(mFontUnderlineButton, SIGNAL(clicked()), this, SLOT(setFontUnderline()));
    connect(mColorButton, SIGNAL(clicked(bool)), this, SLOT(pickColor()));
    connect(mSwtichTextEditorToolBar, SIGNAL(clicked(bool)), this, SLOT(swtichTextEditorToolBar()));

    QList<QGraphicsItem*> itemsOnToolBar;
    itemsOnToolBar << mFontButton << mColorButton
                   << mFontBoldButton << mFontItalicButton << mFontUnderlineButton
                   << DelegateButton::Spacer
                   << mSwtichTextEditorToolBar;

    mToolBarItem->setItemsOnToolBar(itemsOnToolBar);
    //qWarning()<<"6666666666666666666666666666666666666666666666666666666666666666";
    mToolBarItem->setShifting(true);
    mToolBarItem->setVisibleOnBoard(true);
}

void UBGraphicsTextItemDelegate::buildButtonsExtended()
{    
    mFontButton = new DelegateButton(":/images/textEditor/font.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mFontBoldButton = new DelegateButton(":/images/textEditor/bold.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mFontItalicButton = new DelegateButton(":/images/textEditor/italic.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mFontUnderlineButton = new DelegateButton(":/images/textEditor/underline.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mColorButton = new DelegateButton(":/images/color.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mDecreaseSizeButton = new DelegateButton(":/images/textEditor/decrease-font-size.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mIncreaseSizeButton = new DelegateButton(":/images/textEditor/increase-font-size.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mBackgroundColorButton = new DelegateButton(":/images/textEditor/bucket.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mLeftAlignmentButton = new DelegateButton(":/images/textEditor/align-left.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mCenterAlignmentButton = new DelegateButton(":/images/textEditor/align-center.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mJustifyAlignmentButton = new DelegateButton(":/images/textEditor/align-justify.svg", mDelegated, mToolBarItem, Qt::TitleBarArea); // Issue 12/03/2018 - OpenBoard - TEXT EDITOR - NEW JUSTIFY BUTTON
    mRightAlignmentButton = new DelegateButton(":/images/textEditor/align-right.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mCodeButton = new DelegateButton(":/images/textEditor/code.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mUnorderedListButton= new DelegateButton(":/images/textEditor/unordered-list.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mOrderedListButton= new DelegateButton(":/images/textEditor/ordered-list.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mAddIndentButton = new DelegateButton(":/images/textEditor/indent.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mRemoveIndentButton = new DelegateButton(":/images/textEditor/unindent.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mHyperLinkButton = new DelegateButton(":/images/textEditor/link.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mTableButton = new DelegateButton(":/images/textEditor/table.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);
    mSwtichTextEditorToolBar = new DelegateButton(":/images/textEditor/collapse.svg", mDelegated, mToolBarItem, Qt::TitleBarArea);

    connect(mFontButton, SIGNAL(clicked(bool)), this, SLOT(pickFont()));
    connect(mFontBoldButton, SIGNAL(clicked()), this, SLOT(setFontBold()));
    connect(mFontItalicButton, SIGNAL(clicked()), this, SLOT(setFontItalic()));
    connect(mFontUnderlineButton, SIGNAL(clicked()), this, SLOT(setFontUnderline()));
    connect(mColorButton, SIGNAL(clicked(bool)), this, SLOT(pickColor()));    
    connect(mDecreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(decreaseSize()));
    connect(mIncreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(increaseSize()));
    connect(mBackgroundColorButton, SIGNAL(clicked(bool)), this, SLOT(pickBackgroundColor()));    
    connect(mLeftAlignmentButton, SIGNAL(clicked(bool)), this, SLOT(setAlignmentToLeft()));
    connect(mCenterAlignmentButton, SIGNAL(clicked(bool)), this, SLOT(setAlignmentToCenter()));
    connect(mJustifyAlignmentButton, SIGNAL(clicked(bool)), this, SLOT(setAlignmentToJustify())); // Issue 12/03/2018 - OpenBoard - TEXT EDITOR - NEW JUSTIFY BUTTON
    connect(mRightAlignmentButton, SIGNAL(clicked(bool)), this, SLOT(setAlignmentToRight()));
    connect(mCodeButton, SIGNAL(clicked(bool)), this, SLOT(alternHtmlMode()));
    connect(mUnorderedListButton, SIGNAL(clicked(bool)), this, SLOT(insertUnorderedList()));
    connect(mOrderedListButton, SIGNAL(clicked(bool)), this, SLOT(insertOrderedList()));
    connect(mAddIndentButton, SIGNAL(clicked(bool)), this, SLOT(addIndent()));
    connect(mRemoveIndentButton, SIGNAL(clicked(bool)), this, SLOT(removeIndent()));
    connect(mHyperLinkButton, SIGNAL(clicked(bool)), this, SLOT(addLink()));
    connect(mTableButton, SIGNAL(clicked(bool)), this, SLOT(showMenuTable()));
    connect(mSwtichTextEditorToolBar, SIGNAL(clicked(bool)), this, SLOT(swtichTextEditorToolBar()));

    // Create actions and subMenus of the "Table" menu :
    mTableMenu = new QMenu();

    mTableMenu->addAction(QIcon(":/images/textEditor/add-table.png"), tr("Insert table"), this, SLOT(setTableSize()))->setIconVisibleInMenu(true);

    QMenu *columnMenu = mTableMenu->addMenu(tr("Column"));
    columnMenu->addAction(QIcon(":/images/textEditor/insert-column-left.png"), tr("Insert column after"), this, SLOT(insertColumnOnRight()))->setIconVisibleInMenu(true);
    columnMenu->addAction(QIcon(":/images/textEditor/insert-column-right.png"), tr("Insert column before"), this, SLOT(insertColumnOnLeft()))->setIconVisibleInMenu(true);
    columnMenu->addAction(QIcon(":/images/textEditor/delete-column.png"), tr("Delete column"), this, SLOT(deleteColumn()))->setIconVisibleInMenu(true);

    QMenu *rowMenu = mTableMenu->addMenu(tr("Row"));
    rowMenu->addAction(QIcon(":/images/textEditor/insert-row-top.png"), tr("Insert row after"), this, SLOT(insertRowOnBottom()))->setIconVisibleInMenu(true);
    rowMenu->addAction(QIcon(":/images/textEditor/insert-row-bottom.png"), tr("Insert row before"), this, SLOT(insertRowOnTop()))->setIconVisibleInMenu(true);
    rowMenu->addAction(QIcon(":/images/textEditor/delete-row.png"), tr("Delete row"), this, SLOT(deleteRow()))->setIconVisibleInMenu(true);

    mTableMenu->addAction(QIcon(":/images/textEditor/cell-properties.png"), tr("Cell properties"), this, SLOT(setCellProperties()))->setIconVisibleInMenu(true);
    mTableMenu->addAction(QIcon(), tr("Evenly distribute the columns"), this, SLOT(distributeColumn()))->setIconVisibleInMenu(true);

    //update the position of the menu and the sub menu
    mTableMenu->show();
    mTableMenu->hide();

    columnMenu->show();
    columnMenu->hide();

    rowMenu->show();
    rowMenu->hide();

    QList<QGraphicsItem*> itemsOnToolBar;
    itemsOnToolBar << mFontButton << mColorButton
                   << mFontBoldButton << mFontItalicButton << mFontUnderlineButton
                   << DelegateButton::Spacer
                   << mDecreaseSizeButton << mIncreaseSizeButton
                   << DelegateButton::Spacer
                   << mLeftAlignmentButton << mCenterAlignmentButton << mRightAlignmentButton << mJustifyAlignmentButton // Issue 12/03/2018 - OpenBoard - TEXT EDITOR - NEW JUSTIFY BUTTON
                   << DelegateButton::Spacer
                   << mUnorderedListButton << mOrderedListButton
                   << DelegateButton::Spacer << mAddIndentButton << mRemoveIndentButton
                   << DelegateButton::Spacer
                   << mHyperLinkButton << mTableButton << mBackgroundColorButton << mCodeButton
                   << DelegateButton::Spacer
                   << mSwtichTextEditorToolBar;

    mToolBarItem->setItemsOnToolBar(itemsOnToolBar);
    //qWarning()<<"6666666666666666666666666666666666666666666666666666666666666666";
    mToolBarItem->setShifting(true);
    mToolBarItem->setVisibleOnBoard(true);
}

void UBGraphicsTextItemDelegate::contentsChanged()
{
    delegated()->contentsChanged();
    positionHandles();    
}

//Issue N/C - NNE - 20140528
void UBGraphicsTextItemDelegate::duplicate()
{
    UBGraphicsItemDelegate::duplicate();

    delegated()->activateTextEditor(false);
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

// Issue 16/03/2018 - OpenBoard - Two text-editor toolboards.
void UBGraphicsTextItemDelegate::swtichTextEditorToolBar(){
    UBApplication::boardController->activeScene()->removeItem(mToolBarItem);
    mToolBarItem=new UBGraphicsToolBarItem(mDelegated);

    /*mToolBarItem->setVisible(false);*/
    if(delegated()->isToolbarExtended() == true){
        delegated()->setToolbarExtended(false);
    }
    else{
        delegated()->setToolbarExtended(true);
    }

    buildButtons();
}

void UBGraphicsTextItemDelegate::pickFont()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        QFontDialog fontDialog(delegated()->textCursor().charFormat().font(), static_cast<QGraphicsView*>(UBApplication::boardController->controlView()));
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
//          disabled and replaced by the next line because of not optimum result (text splits to two lines when that is not necessary)
    //          delegated()->adjustSize();
                delegated()->resize(delegated()->document()->idealWidth(), delegated()->size().height());
                delegated()->contentsChanged();
            }
        }
    }

void UBGraphicsTextItemDelegate::setFontBold()
{
    QTextCursor cursor = delegated()->textCursor();
    QTextCharFormat format = cursor.charFormat();
    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();

    if (anchorPos > cursorPos)
        std::swap(cursorPos, anchorPos);

    format.setFontWeight(cursor.charFormat().fontWeight() != QFont::Bold ? QFont::Bold : QFont::Normal);

    //UBSettings::settings()->setBoldFont(format.fontWeight() > QFont::Normal);

    if (cursor.selectedText().length() == 0)
    {
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.setCharFormat(format);
        cursor.clearSelection();
    }
    else
        cursor.setCharFormat(format);

    if (anchorPos >= cursorPos)
    {
        cursor.setPosition(cursorPos, QTextCursor::MoveAnchor);
        cursor.setPosition(anchorPos, QTextCursor::KeepAnchor);
    }
    else
    {
        cursor.setPosition(anchorPos, QTextCursor::MoveAnchor);
        cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
    }

    delegated()->setFocus();
    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::setFontItalic()
{
    QTextCharFormat format;
    QTextCursor cursor = delegated()->textCursor();

    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();
    if (anchorPos >= cursorPos)
        std::swap(cursorPos, anchorPos);

    format.setFontItalic(!cursor.charFormat().fontItalic());
    if (cursor.selectedText().length() == 0)
    {
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(format);
        cursor.clearSelection();
    }
    else
        cursor.mergeCharFormat(format);

    cursor.setPosition(anchorPos, QTextCursor::MoveAnchor);
    cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
    delegated()->setFocus();
    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::setFontUnderline()
{
    QTextCharFormat format;
    QTextCursor cursor = delegated()->textCursor();

    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();
    if (anchorPos >= cursorPos)
        std::swap(cursorPos, anchorPos);

    format.setFontUnderline(!cursor.charFormat().fontUnderline());
    if (cursor.selectedText().length() == 0)
    {
        cursor.select(QTextCursor::WordUnderCursor);
        cursor.mergeCharFormat(format);
        cursor.clearSelection();
    }
    else
        cursor.mergeCharFormat(format);

    cursor.setPosition(anchorPos, QTextCursor::MoveAnchor);
    cursor.setPosition(cursorPos, QTextCursor::KeepAnchor);
    delegated()->setFocus();
    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::pickColor()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        QColorDialog colorDialog(delegated()->defaultTextColor(), static_cast<QGraphicsView*>(UBApplication::boardController->controlView()));
        colorDialog.setWindowTitle(tr("Text Color"));
        colorDialog.setOption(QColorDialog::DontUseNativeDialog);
        if (UBSettings::settings()->isDarkBackground())
        {
            colorDialog.setStyleSheet("background-color: white;");
        }

        if (colorDialog.exec())
        {
            QColor selectedColor = colorDialog.selectedColor();
            delegated()->setDefaultTextColor(selectedColor);
            QTextCursor curCursor = delegated()->textCursor();
            QTextCharFormat format;
            format.setForeground(QBrush(selectedColor));
            curCursor.mergeCharFormat(format);
            delegated()->setTextCursor(curCursor);

            UBGraphicsTextItem::lastUsedTextColor = selectedColor;

            delegated()->setSelected(true);            
            delegated()->contentsChanged();
            delegated()->setFocus();
        }
    }
}

void UBGraphicsTextItemDelegate::pickBackgroundColor()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        QColorDialog colorDialog(delegated()->defaultTextColor(), mDelegated->scene()->views().at(0));
        colorDialog.setWindowTitle(tr("Background Color"));
        if (UBSettings::settings()->isDarkBackground())
        {
            colorDialog.setStyleSheet("background-color: white;");
        }

        if (colorDialog.exec())
        {
            QColor selectedColor = colorDialog.selectedColor();
            delegated()->setBackgroundColor(selectedColor);
            delegated()->setSelected(true);
            delegated()->contentsChanged();
            delegated()->setFocus();
        }
    }
}

void UBGraphicsTextItemDelegate::insertTable()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {                
        delegated()->insertTable(mTablePalette->lines(), mTablePalette->columns());
        mTablePalette->hide();
    }
}

void UBGraphicsTextItemDelegate::addIndent()
{
    QTextCursor cursor = delegated()->textCursor();
    QTextBlockFormat blockFmt = cursor.blockFormat();
    QTextListFormat listFmt;
    QTextList *list = cursor.currentList();

    if (list)
    {
        //create new format
        listFmt = list->format();
        listFmt.setStyle(nextStyle(listFmt.style()));
        listFmt.setIndent(listFmt.indent()+1);

        //create new sub-list with new format
        QTextList* theList = cursor.createList(listFmt);
        theList->add(cursor.block());
    }
    else
    {
        blockFmt.setIndent(blockFmt.indent()+1);
        cursor.setBlockFormat(blockFmt);
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::removeIndent()
{
    QTextCursor cursor = delegated()->textCursor();
    QTextBlockFormat blockFmt = cursor.blockFormat();
    QTextListFormat listFmt;

    QTextList *list = cursor.currentList();

    if (list)
    {
        //create new format
        listFmt = list->format();
        listFmt.setStyle(previousStyle(listFmt.style()));
        listFmt.setIndent(listFmt.indent()-1);

        //create new sub-list with new format
        QTextList* theList = cursor.createList(listFmt);
        theList->add(cursor.block());
    }
    else
    {
        blockFmt.setIndent(blockFmt.indent()-1);
        cursor.setBlockFormat(blockFmt);
    }

    delegated()->setFocus();
}

QTextListFormat::Style UBGraphicsTextItemDelegate::nextStyle(QTextListFormat::Style format)
{
    QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;

    if(format >= QTextListFormat::ListSquare){
        style = static_cast<QTextListFormat::Style>((format - 1) % 4);
        if (style == 0)
            style = QTextListFormat::ListDisc;
    }else if(format >= QTextListFormat::ListUpperRoman){
        style = static_cast<QTextListFormat::Style>((format - 1) % 9);
        if(style == 0)
            style = QTextListFormat::ListDecimal;
    }

    return style;
}

QTextListFormat::Style UBGraphicsTextItemDelegate::previousStyle(QTextListFormat::Style format)
{
    QTextListFormat::Style style = QTextListFormat::ListStyleUndefined;

    if(format >= QTextListFormat::ListSquare){
        style = static_cast<QTextListFormat::Style>((format + 1) % 4);
        if (style == 0)
            style = QTextListFormat::ListSquare;
    }else if(format >= QTextListFormat::ListUpperRoman){
        style = static_cast<QTextListFormat::Style>((format + 1) % 9);
        if(style == QTextListFormat::ListSquare)
            style = QTextListFormat::ListUpperRoman;
    }

    return style;
}

void UBGraphicsTextItemDelegate::insertOrderedList()
{
    insertList(QTextListFormat::ListDecimal);
}

void UBGraphicsTextItemDelegate::insertUnorderedList()
{
    insertList(QTextListFormat::ListDisc);
}

void UBGraphicsTextItemDelegate::insertList(QTextListFormat::Style format)
{
    QTextCursor cursor = delegated()->textCursor();

    cursor.beginEditBlock();

    QTextBlockFormat blockFmt = cursor.blockFormat();
    QTextListFormat listFmt;

    QTextList *list = cursor.currentList();

    if (list)
    {
        int oldFormat = static_cast<int>(list->format().style());

        /*
         * If the current format and old format are same, remove the list
         * else switch the style
         */
        if(format >= QTextListFormat::ListSquare && oldFormat >= QTextListFormat::ListSquare
                || (format <= QTextListFormat::ListDecimal && format >= QTextListFormat::ListUpperRoman
                    && oldFormat <= QTextListFormat::ListDecimal && oldFormat >= QTextListFormat::ListUpperRoman)){
            QTextListFormat listFormat;
            listFormat.setIndent(0);
            listFormat.setStyle(QTextListFormat::ListStyleUndefined);
            list->setFormat(listFormat);

            for( int i = list->count() - 1; i>=0 ; --i)
                list->removeItem(i);
        }else{
            QTextListFormat listFormat(list->format());
            listFormat.setStyle(format);

            QTextList* theList = cursor.createList(listFormat);
            theList->add(cursor.block());
        }
    }
    else
    {
        listFmt.setIndent(blockFmt.indent()+1);
        blockFmt.setIndent(0);
        cursor.setBlockFormat(blockFmt);
        listFmt.setStyle(format);
        cursor.createList(listFmt);
    }

    cursor.endEditBlock();
    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::setTableSize()
{
    mLinkPalette->hide();
    mCellPropertiesPalette->hide();
    mTablePalette->show();
    mTablePalette->setFocus();
}

void UBGraphicsTextItemDelegate::insertColumnOnRight()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->insertColumn(true);
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::insertRowOnBottom()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->insertRow(true);
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::insertColumnOnLeft()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->insertColumn();
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::insertRowOnTop()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->insertRow();
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::deleteColumn()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->deleteColumn();
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::deleteRow()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->deleteRow();
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::setCellProperties()
{
    // Initialize CellPropoertiesPalette "Width" field with current cell width :
    QTextTable* t = delegated()->textCursor().currentTable();
    if (t)
    {
        QVector<QTextLength> widths = t->format().toTableFormat().columnWidthConstraints();
        int columnIndex = t->cellAt(delegated()->textCursor()).column();
        qreal val = widths.at(columnIndex).rawValue();
        mCellPropertiesPalette->setWidth(qRound(val));
    }

    mTablePalette->hide();
    mLinkPalette->hide();
    mCellPropertiesPalette->show();
    mCellPropertiesPalette->setFocus();
}

void UBGraphicsTextItemDelegate::distributeColumn()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->distributeColumn();
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::showMenuTable()
{
    // Position of the delegated Text Editor :
    QPointF p = delegated()->pos();

    // Take in account translations applied to the Text Editor :
    p = delegated()->transform().map(p);

    // Map Scene position to View position :
    p = UBApplication::boardController->controlView()->mapFromScene(p);

    // Adjust position : show the menu near the button :
    p.setX(p.x() + mTableButton->pos().x() + mTableButton->boundingRect().size().width());

    // Take in account the toolbar :
    if(UBSettings::settings()->appToolBarPositionedAtTop->get().toBool())
    {
        p.setY(p.y() + UBApplication::app()->toolBarHeight());
    }

    // Show the menu :
    mTableMenu->exec(p.toPoint(), mTableMenu->actions().first());
}

void UBGraphicsTextItemDelegate::applyCellProperties()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->setCellWidth(mCellPropertiesPalette->width());
        mCellPropertiesPalette->hide();
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::setAlignmentToLeft()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->setAlignmentToLeft();        
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::setAlignmentToCenter()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->setAlignmentToCenter();
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::setAlignmentToJustify() // Issue 12/03/2018 - OpenBoard - TEXT EDITOR - NEW JUSTIFY BUTTON
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->setAlignmentToJustify();
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::setAlignmentToRight()
{
    if (mDelegated && mDelegated->scene() && mDelegated->scene()->views().size() > 0)
    {
        delegated()->setAlignmentToRight();
    }

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::addLink()
{
    QString selectedText = delegated()->textCursor().selectedText();
    mLinkPalette->setText(selectedText);
    mLinkPalette->setLink("");
    mLinkPalette->show();
    mLinkPalette->setFocus();
}

void UBGraphicsTextItemDelegate::insertLink()
{
    if (!(mLinkPalette->text().isEmpty() || mLinkPalette->link().isEmpty()))
    {
        QString link = mLinkPalette->link();
        if (!link.startsWith("http://"))
            link = "http://" + link;

        int oldPosCursor = delegated()->textCursor().position();

        // Create a Format for the link :
        QTextCharFormat linkFormat;
        linkFormat.setAnchor(true);
        linkFormat.setAnchorHref(link);
        delegated()->textCursor().mergeCharFormat(linkFormat);

        // Insert the Link text :
        delegated()->textCursor().insertText(mLinkPalette->text());
        int newPosCursor = delegated()->textCursor().position();

        // Apply the "Link Format" to the "Link Text" :
        QTextCursor cursor = delegated()->textCursor();
        cursor.setPosition(oldPosCursor);
        cursor.setPosition(newPosCursor, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(linkFormat);

        // Trick to refresh text. Without this, the link is not displayed.
        alternHtmlMode();
        alternHtmlMode();

        // Restore cursor position to the end of the link :
        cursor.setPosition(newPosCursor);

        // Apply the new QTextCursor to the text :
        delegated()->setTextCursor(cursor);

        // Add a non-breakable space, so user will be able to enter text without "link format"
        delegated()->textCursor().insertHtml("&nbsp");
    }

    mLinkPalette->hide();

    delegated()->setFocus();
}

void UBGraphicsTextItemDelegate::alternHtmlMode()
{    
    if (!delegated()->htmlMode())
    {
        delegated()->setPlainText(delegated()->toHtml());

        QTextCursor cursor = delegated()->textCursor();
        QTextCharFormat format;
        QFont font;

        font.setFamily("Arial");
        font.setPointSize(12);

        format.setFont(font);

        cursor.select(QTextCursor::Document);
        cursor.setCharFormat(format);
        cursor.clearSelection();

        delegated()->setTextCursor(cursor);
        delegated()->setFont(font);
    }
    else
    {
        delegated()->setHtml(delegated()->toPlainText());
        delegated()->setFont(createDefaultFont());
    }

    delegated()->setHtmlMode(!delegated()->htmlMode());
    changeDelegateButtonsMode(delegated()->htmlMode());
}

void UBGraphicsTextItemDelegate::decreaseSize()
{
    ChangeTextSize(-delta, changeSize);
}

void UBGraphicsTextItemDelegate::increaseSize()
{
   ChangeTextSize(delta, changeSize);
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

    mToolBarItem->setEnabled(editable); // ALTI/AOU - 20140602 : make toolbar inactive when TextEditor is not Editable.
}

void UBGraphicsTextItemDelegate::remove(bool canUndo)
{
    mTablePalette->hide();
    mLinkPalette->hide();
    mCellPropertiesPalette->hide();
    UBGraphicsItemDelegate::remove(canUndo);
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
    //qWarning()<<"UBGraphicsTextItemDelegate::positionHandles()";

    if (mDelegated->isSelected() || (mDelegated->parentItem() && UBGraphicsGroupContainerItem::Type == mDelegated->parentItem()->type()))
    {
        //qWarning()<<"UBGraphicsTextItemDelegate 1";
        if (mToolBarItem->isVisibleOnBoard())
        {
            //qWarning()<<"UBGraphicsTextItemDelegate 2";
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
            if (mToolBarItem->parentItem())
            {
                //qWarning()<<"UBGraphicsTextItemDelegate 3";
                if (group && group->getCurrentItem() == mDelegated && group->isSelected()){
                    mToolBarItem->show();
                }

                if (!group){
                     mToolBarItem->show();
                }
            }

        }
    }
    else
    {
        mToolBarItem->hide();
    }
    UBGraphicsItemDelegate::positionHandles();
}

void UBGraphicsTextItemDelegate::changeDelegateButtonsMode(bool htmlMode)
{
    if (htmlMode)
    {
        mFontButton->setEnabled(false);
        mFontBoldButton->setEnabled(false);
        mFontItalicButton->setEnabled(false);
        mFontUnderlineButton->setEnabled(false);
        mColorButton->setEnabled(false);
        //mDecreaseSizeButton->setEnabled(false);
        //mIncreaseSizeButton->setEnabled(false);
        mBackgroundColorButton->setEnabled(false);
        mLeftAlignmentButton->setEnabled(false);
        mCenterAlignmentButton->setEnabled(false);
        mJustifyAlignmentButton->setEnabled(false);
        mRightAlignmentButton->setEnabled(false);
        mUnorderedListButton->setEnabled(false);
        mOrderedListButton->setEnabled(false);
        mTableButton->setEnabled(false);
        mHyperLinkButton->setEnabled(false);
        mAddIndentButton->setEnabled(false);
        mRemoveIndentButton->setEnabled(false);
    }
    else
    {
        mFontButton->setEnabled(true);
        mFontBoldButton->setEnabled(true);
        mFontItalicButton->setEnabled(true);
        mFontUnderlineButton->setEnabled(true);
        mColorButton->setEnabled(true);
        //mDecreaseSizeButton->setEnabled(true);
        //mIncreaseSizeButton->setEnabled(true);
        mBackgroundColorButton->setEnabled(true);
        mLeftAlignmentButton->setEnabled(true);
        mCenterAlignmentButton->setEnabled(true);
        mJustifyAlignmentButton->setEnabled(true);
        mRightAlignmentButton->setEnabled(true);
        mUnorderedListButton->setEnabled(true);
        mOrderedListButton->setEnabled(true);
        mTableButton->setEnabled(true);
        mHyperLinkButton->setEnabled(true);
        mAddIndentButton->setEnabled(true);
        mRemoveIndentButton->setEnabled(true);
    }
}

void UBGraphicsTextItemDelegate::ChangeTextSize(qreal factor, textChangeMode changeMode)
{    
    if (scaleSize == changeMode)
    {
        if (1 == factor)
            return;
    }
    else
    if (0 == factor)
        return;

    UBGraphicsTextItem *item = dynamic_cast<UBGraphicsTextItem*>(delegated());

    if (item && (QString() == item->toPlainText()))
    {
        delegated()->setFocus();
        return;
    }

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
    bool isUnderline = false;
    bool isBold = false;
    bool isItalic = false;
    QFont nextFont;
    bool nextIsUnderline = false;
    bool nextIsItalic = false;
    bool nextIsBold = false;

    // we search continuous blocks of the text with the same PointSize and allpy new settings for them.
    cursor.setPosition (startPos, QTextCursor::MoveAnchor);
    while(iCursorPos < endPos)
    {
        bEndofTheSameBlock = false;
        iBlockLen = 0;

        cursor.setPosition (iCursorPos+1, QTextCursor::KeepAnchor);
        iPointSize = cursor.charFormat().font().pointSize();
        curFont = cursor.charFormat().font();        
        isUnderline = cursor.charFormat().fontUnderline();
        isItalic = cursor.charFormat().fontItalic();
        isBold = cursor.charFormat().fontWeight() == QFont::Bold;
        cursor.setPosition (iCursorPos, QTextCursor::KeepAnchor);

        do
        {
            iBlockLen++;

            cursor.setPosition (iCursorPos+iBlockLen+1, QTextCursor::KeepAnchor);
            iNextPointSize = cursor.charFormat().font().pointSize();
            nextFont = cursor.charFormat().font();
            nextIsUnderline = cursor.charFormat().fontUnderline();
            nextIsItalic = cursor.charFormat().fontItalic();
            nextIsBold = cursor.charFormat().fontWeight() == QFont::Bold;

            cursor.setPosition (iCursorPos+iBlockLen, QTextCursor::KeepAnchor);

            if(cursor.charFormat().isTableCellFormat()){
                //if the current char is in a cell end the block
                cursor.setPosition (iCursorPos+iBlockLen-1, QTextCursor::KeepAnchor);
                bEndofTheSameBlock = true;
            }

            if ((isBold != nextIsBold)
                || (isItalic != nextIsItalic)
                || (isUnderline != nextIsUnderline)
                || (curFont != nextFont)
                || (iCursorPos+iBlockLen >= endPos))
                bEndofTheSameBlock = true;

        }while(!bEndofTheSameBlock);

        //setting new parameters
        int iNewPointSize = (changeSize == changeMode) ? (iPointSize + factor) : (iPointSize * factor);

        curFont.setPointSize( (iNewPointSize > 0)?iNewPointSize:1);
        textFormat.setFont(curFont);
        cursor.mergeCharFormat(textFormat);

        iCursorPos += iBlockLen;
        cursor.setPosition (iCursorPos, QTextCursor::MoveAnchor);
    }

    UBSettings::settings()->setFontPointSize(iPointSize);
    //returning initial selection
    cursor.setPosition (anchorPos, QTextCursor::MoveAnchor);
    cursor.setPosition (cursorPos, QTextCursor::KeepAnchor);

    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::scaleTextSize(qreal multiplyer)
{
    ChangeTextSize(multiplyer, scaleSize);
}

QVariant UBGraphicsTextItemDelegate::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemSelectedChange)
    {
        if (delegated()->isSelected())
        {
            QTextCursor c = delegated()->textCursor();
            if (c.hasSelection())
            {
                c.clearSelection();
                delegated()->setTextCursor(c);
            }
        }
    }
    return UBGraphicsItemDelegate::itemChange(change, value);
}

UBCreateHyperLinkPalette *UBGraphicsTextItemDelegate::linkPalette()
{
    return mLinkPalette;
}

UBCreateTablePalette* UBGraphicsTextItemDelegate::tablePalette()
{
    return mTablePalette;
}


UBCellPropertiesPalette* UBGraphicsTextItemDelegate::cellPropertiesPalette()
{
    return mCellPropertiesPalette;
}
