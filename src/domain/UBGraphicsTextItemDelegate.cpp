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
#include "board/UBBoardView.h"

#include "core/memcheck.h"

const int UBGraphicsTextItemDelegate::sMinPixelSize = 8;
const int UBGraphicsTextItemDelegate::sMinPointSize = 8;


AlignTextButton::AlignTextButton(const QString &fileName, QGraphicsItem *pDelegated, QGraphicsItem *parent, Qt::WindowFrameSection section)
    : DelegateButton(fileName, pDelegated, parent, section)
    , lft(new QSvgRenderer(QString(":/images/leftAligned.svg")))
    , cntr(new QSvgRenderer(QString(":/images/centerAligned.svg")))
    , rght(new QSvgRenderer(QString(":/images/rightAligned.svg")))
    , mxd(new QSvgRenderer(QString(":/images/notAligned.svg")))
    , mHideMixed(true)
{
    setKind(k_left);
}

AlignTextButton::~AlignTextButton()
{
    if (lft) delete lft;
    if (cntr) delete cntr;
    if (rght) delete rght;
    if (mxd) delete mxd;
}

void AlignTextButton::setKind(int pKind)
{
    if (mHideMixed && pKind == k_mixed) {
        qDebug() << "Mixed button is hidden, can't process it";
        return;
    }
    mKind = pKind;
    QSvgRenderer *rndrer = rndFromKind(pKind);
    Q_ASSERT(rndrer);
    setSharedRenderer(rndrer);
}

void AlignTextButton::setNextKind()
{
    int mxKind = MAX_KIND;
    if (mHideMixed) {
        mxKind--;
    }
    setKind(mKind == mxKind ? 0 : ++mKind);
}

int AlignTextButton::nextKind() const
{
    int mxKind = MAX_KIND;
    if (mHideMixed) {
        mxKind--;
    }
    int result = mKind;
    return mKind == mxKind ? 0 : ++result;
}

UBGraphicsTextItemDelegate::UBGraphicsTextItemDelegate(UBGraphicsTextItem* pDelegated, QObject *)
    : UBGraphicsItemDelegate(pDelegated,0, GF_COMMON | GF_REVOLVABLE | GF_TITLE_BAR_USED)
    , mFontButton(0)
    , mColorButton(0)
    , mDecreaseSizeButton(0)
    , mIncreaseSizeButton(0)
    , mAlignButton(0)
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
    if (UBSettings::settings()->isDarkBackground())
    {
        if (UBGraphicsTextItem::lastUsedTextColor == Qt::black)
            UBGraphicsTextItem::lastUsedTextColor = Qt::white;
    }
    else
    {
        if (UBGraphicsTextItem::lastUsedTextColor == Qt::white)
            UBGraphicsTextItem::lastUsedTextColor = Qt::black;
    }
    delegated()->setDefaultTextColor(UBGraphicsTextItem::lastUsedTextColor);
    format.setForeground(QBrush(UBGraphicsTextItem::lastUsedTextColor));
    curCursor.mergeCharFormat(format);
    delegated()->setTextCursor(curCursor);
    delegated()->setFont(font);

    delegated()->adjustSize();
    delegated()->contentsChanged();

    connect(delegated()->document(), SIGNAL(cursorPositionChanged(QTextCursor)), this, SLOT(onCursorPositionChanged(QTextCursor)));
    connect(delegated()->document(), SIGNAL(modificationChanged(bool)), this, SLOT(onModificationChanged(bool)));
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
    int pointSize = UBSettings::settings()->fontPointSize();
    if (pointSize > 0) {
        font.setPointSize(pointSize);
    }

    return font;
}

void UBGraphicsTextItemDelegate::createControls()
{
    UBGraphicsItemDelegate::createControls();

    if (!mFontButton) {
        mFontButton = new DelegateButton(":/images/font.svg", mDelegated, mFrame, Qt::TitleBarArea);
        connect(mFontButton, SIGNAL(clicked(bool)), this, SLOT(pickFont()));
        mButtons << mFontButton;
    }
    if (!mColorButton) {
        mColorButton = new DelegateButton(":/images/color.svg", mDelegated, mFrame, Qt::TitleBarArea);
        connect(mColorButton, SIGNAL(clicked(bool)), this, SLOT(pickColor()));
        mButtons << mColorButton;
    }

    if (!mDecreaseSizeButton) {
        mDecreaseSizeButton = new DelegateButton(":/images/minus.svg", mDelegated, mFrame, Qt::TitleBarArea);
        connect(mDecreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(decreaseSize()));
        mButtons << mDecreaseSizeButton;
    }

    if (!mIncreaseSizeButton) {
        mIncreaseSizeButton = new DelegateButton(":/images/plus.svg", mDelegated, mFrame, Qt::TitleBarArea);
        connect(mIncreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(increaseSize()));
        mButtons << mIncreaseSizeButton;
    }

    if (!mAlignButton) {
        mAlignButton = new AlignTextButton(":/images/plus.svg", mDelegated, mFrame, Qt::TitleBarArea);
        connect(mAlignButton, SIGNAL(clicked()), this, SLOT(alignButtonProcess()));
        mButtons << mAlignButton;
    }

    foreach(DelegateButton* button, mButtons)
    {
        button->hide();
        button->setFlag(QGraphicsItem::ItemIsSelectable, true);
    }

    // Claudio: on changing the zvlaue the frame is updated updateSelectionFrame and
    // the default value for the operation mode is set (scaling). This isn't the right
    // mode for text so we call that here.
    frame()->setOperationMode(UBGraphicsDelegateFrame::Resizing);

}

/**
 * @brief Calculate the width of the toolbar containing the text item-related buttons
 * @return The space between the left-most and right-most buttons in pixels
 */
qreal UBGraphicsTextItemDelegate::titleBarWidth()
{
    if (!mFontButton)
        return 0;

    // refresh the frame and buttons' positions
    positionHandles();

    qreal titleBarWidth(0);
    qreal frameLeftCoordinate = mFontButton->pos().x();
    qreal frameRightCoordinate = frameLeftCoordinate;

    foreach(DelegateButton* button, mButtons) {
        if (button->getSection() == Qt::TitleBarArea) {
            frameLeftCoordinate = qMin(button->pos().x(), frameLeftCoordinate);
            frameRightCoordinate = qMax(button->pos().x() + button->boundingRect().width(), frameRightCoordinate);
        }
    }

    return frameRightCoordinate - frameLeftCoordinate;
}

void UBGraphicsTextItemDelegate::freeButtons()
{
    mButtons.removeOne(mFontButton);
    delete mFontButton;
    mFontButton = 0;

    mButtons.removeOne(mColorButton);
    delete mColorButton;
    mColorButton = 0;

    mButtons.removeOne(mDecreaseSizeButton);
    delete mDecreaseSizeButton;
    mDecreaseSizeButton = 0;

    mButtons.removeOne(mIncreaseSizeButton);
    delete mIncreaseSizeButton;
    mIncreaseSizeButton = 0;

    mButtons.removeOne(mAlignButton);
    delete mAlignButton;
    mAlignButton = 0;

    UBGraphicsItemDelegate::freeButtons();
}




void UBGraphicsTextItemDelegate::contentsChanged()
{
    delegated()->contentsChanged();
}

// This method is used to filter the available fonts. Only the web-compliant fonts
// will remain in the font list.
void UBGraphicsTextItemDelegate::customize(QFontDialog &fontDialog)
{
    if (UBSettings::settings()->isDarkBackground()) {
        fontDialog.setStyleSheet("background-color: white;");
    }

    QListView *fontNameListView = NULL;
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
        // https://bugreports.qt.io/browse/QTBUG-79637
        QFontDialog fontDialog(static_cast<QGraphicsView*>(UBApplication::boardController->controlView()));

        fontDialog.setOption(QFontDialog::DontUseNativeDialog);
        fontDialog.setCurrentFont(delegated()->textCursor().charFormat().font());
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

            delegated()->setSelected(true);
            delegated()->setFocus();
            delegated()->contentsChanged();
        }
    }
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

            QTextCursor curCursor = delegated()->textCursor();

            QTextCharFormat format;
            format.setForeground(QBrush(selectedColor));
            curCursor.mergeCharFormat(format);
            delegated()->setTextCursor(curCursor);

            if (!curCursor.hasSelection() || (curCursor.selectedText().length() == delegated()->toPlainText().length()))
            {
                delegated()->setDefaultTextColor(selectedColor);
                UBGraphicsTextItem::lastUsedTextColor = selectedColor;
            }

            delegated()->setSelected(true);
            delegated()->setFocus();
            delegated()->contentsChanged();
        }
    }
}

void UBGraphicsTextItemDelegate::decreaseSize()
{
    ChangeTextSize(-delta, changeSize);
}

void UBGraphicsTextItemDelegate::increaseSize()
{
   ChangeTextSize(delta, changeSize);
}

void UBGraphicsTextItemDelegate::alignButtonProcess()
{
    qDebug() << "alignButtonProcess() clicked";
    QObject *sndr = sender();

    if (sndr == mAlignButton) {
        qDebug() << "Align button";
        AlignTextButton *asAlText = static_cast<AlignTextButton*>(mAlignButton);
        if (asAlText->nextKind() == AlignTextButton::k_mixed) {
            restoreTextCursorFormats();
            asAlText->setNextKind();
            return;
        }
        asAlText->setNextKind();

        QTextCursor cur = delegated()->textCursor();
        QTextBlockFormat fmt = cur.blockFormat();
        switch (asAlText->kind()) {
        case AlignTextButton::k_left:
            fmt.setAlignment(Qt::AlignLeft);
            break;
        case AlignTextButton::k_center:
            fmt.setAlignment(Qt::AlignCenter);
            break;
        case AlignTextButton::k_right:
            fmt.setAlignment(Qt::AlignRight);
            break;
        case AlignTextButton::k_mixed:
            break;
        }

        delegated()->setTextCursor(cur);
        cur.setBlockFormat(fmt);
        delegated()->setFocus();
    }

    qDebug() << "sender process" << sndr;
}

void UBGraphicsTextItemDelegate::onCursorPositionChanged(const QTextCursor &cursor)
{
    qDebug() << "cursor position changed";
    qDebug() << "-----------------------";
    qDebug() << "we have a selection!" << cursor.selectionStart();
    qDebug() << "-----------------------";
}

void UBGraphicsTextItemDelegate::onModificationChanged(bool ch)
{
    Q_UNUSED(ch);
    qDebug() << "modification changed";
}

void UBGraphicsTextItemDelegate::onContentChanged()
{
    qDebug() << "onContentChanged";
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

void UBGraphicsTextItemDelegate::remove(bool canUndo)
{
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

bool UBGraphicsTextItemDelegate::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    mSelectionData.mButtonIsPressed = true;
    qDebug() << "Reporting selection  of the cursor (mouse press)" << delegated()->textCursor().selection().isEmpty();
    qDebug() << QString("Anchor: %1\nposition: %2 (mouse press)").arg(delegated()->textCursor().anchor()).arg(delegated()->textCursor().position());

    if (!UBGraphicsItemDelegate::mousePressEvent(event)) {
        return false;
    }

    return true;
}

bool UBGraphicsTextItemDelegate::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (mSelectionData.mButtonIsPressed) {
        qDebug() << "Reporting selection of the cursor (mouse move)" << delegated()->textCursor().selection().isEmpty();
        qDebug() << QString("Anchor: %1\nposition: %2 (mouse mouse move)").arg(delegated()->textCursor().anchor()).arg(delegated()->textCursor().position());
    }

    if (!UBGraphicsItemDelegate::mouseMoveEvent(event)) {
        return false;
    }

    return true;
}

bool UBGraphicsTextItemDelegate::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    mSelectionData.mButtonIsPressed = false;
    qDebug() << "Reporting selection of the cursor (mouse release)" << delegated()->textCursor().selection().isEmpty();
    qDebug() << QString("Anchor: %1\nposition: %2 (mouse mouse release)").arg(delegated()->textCursor().anchor()).arg(delegated()->textCursor().position());
    updateAlighButtonState();

    if (!UBGraphicsItemDelegate::mouseReleaseEvent(event)) {
        return false;
    }

    return true;
}

bool UBGraphicsTextItemDelegate::keyPressEvent(QKeyEvent *event)
{
    Q_UNUSED(event);
    return true;
}

bool UBGraphicsTextItemDelegate::keyReleaseEvent(QKeyEvent *event)
{
    if (!delegated()->hasFocus()) {
        return true;
    }

    switch (event->key()) {
    case Qt::Key_Left:
    case Qt::Key_Right:
    case Qt::Key_Up:
    case Qt::Key_Down:
        updateAlighButtonState();
        break;
    }

    qDebug() << "Key has been released" << QString::number(event->key(), 16);
    return true;
}

void UBGraphicsTextItemDelegate::ChangeTextSize(qreal factor, textChangeMode changeMode)
{
    // round it to the nearest hundredth
    factor = floor(factor*100+0.5)/100.;

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
        return;

    QTextCursor cursor = delegated()->textCursor();
    QTextCharFormat textFormat;

    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();

    if (!cursor.hasSelection())
    {
        cursor.select(QTextCursor::Document);
    }

    // Now we got the real start and stop positions
    int startPos = qMin(cursor.anchor(), cursor.position());
    int endPos = qMax(cursor.anchor(), cursor.position());

    QFont curFont;
    QFont nextCharFont;
    bool bEndofTheSameBlock;
    int iBlockLen;
    int iPointSize;
    int iNextPointSize;
    int iCursorPos = startPos;
    QBrush curBrush;
    QBrush nextCharBrush;

   // we search continuous blocks of the text with the same PointSize and allpy new settings for them.
    cursor.setPosition (startPos, QTextCursor::MoveAnchor);
    while(iCursorPos < endPos)
    {
        bEndofTheSameBlock = false;
        iBlockLen = 0;

        // Here we get the point size of the first character
        cursor.setPosition (iCursorPos+1, QTextCursor::KeepAnchor);
        curFont = cursor.charFormat().font();
        curBrush = cursor.charFormat().foreground();
        iPointSize = curFont.pointSize();

        // Then we position the end cursor to the start cursor position
        cursor.setPosition (iCursorPos, QTextCursor::KeepAnchor);

        do
        {
            // Get the next character font size
            cursor.setPosition (iCursorPos+iBlockLen+1, QTextCursor::KeepAnchor);
            nextCharFont = cursor.charFormat().font();
            nextCharBrush = cursor.charFormat().foreground();
            iNextPointSize = nextCharFont.pointSize();

            if (
                    (iPointSize != iNextPointSize)
                 || (iCursorPos+iBlockLen >= endPos)
                 || (curFont.family().compare(nextCharFont.family()) != 0)
                 || (curFont.italic() != nextCharFont.italic())
                 || (curFont.bold() != nextCharFont.bold())
                 || (curFont.underline() != nextCharFont.underline())
                 || (curBrush != nextCharBrush))
            {
                bEndofTheSameBlock = true;
                break;
            }

            iBlockLen++;

        }while(!bEndofTheSameBlock);


        //setting new parameters
        QFont tmpFont = curFont;
        int iNewPointSize = (changeSize == changeMode) ? (iPointSize + factor) : (iPointSize * factor);
        tmpFont.setPointSize( (iNewPointSize > 0)?iNewPointSize:1);
        textFormat.setFont(tmpFont);
        textFormat.setForeground(curBrush);
        cursor.setPosition (iCursorPos+iBlockLen, QTextCursor::KeepAnchor);
        cursor.mergeCharFormat(textFormat);

        iCursorPos += iBlockLen;
        cursor.setPosition (iCursorPos, QTextCursor::MoveAnchor);

        curFont = nextCharFont;
    }

    delegated()->setFont(curFont);
    UBSettings::settings()->setFontPointSize(iPointSize);
    //returning initial selection
    cursor.setPosition (anchorPos, QTextCursor::MoveAnchor);
    cursor.setPosition (cursorPos, QTextCursor::KeepAnchor);

    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::recolor()
{
    QTextCursor cursor = delegated()->textCursor();
    QTextCharFormat textFormat;

    int anchorPos = cursor.anchor();
    int cursorPos = cursor.position();

    if (0 == anchorPos-cursorPos)
    {
        // If nothing is selected, then we select all the text
        cursor.setPosition (0, QTextCursor::MoveAnchor);
        cursor.setPosition (cursor.document()->characterCount()-1, QTextCursor::KeepAnchor);
    }

    int startPos = qMin(cursor.anchor(), cursor.position());
    int endPos = qMax(cursor.anchor(), cursor.position());

    QFont curFont;
    QFont nextCharFont;
    bool bEndofTheSameBlock;
    int iBlockLen;
    int iCursorPos = startPos;
    QBrush curBrush;
    QBrush nextCharBrush;

    cursor.setPosition (startPos, QTextCursor::MoveAnchor);
    while(iCursorPos < endPos)
    {
        bEndofTheSameBlock = false;
        iBlockLen = 0;

        // Here we get the point size of the first character
        cursor.setPosition (iCursorPos+1, QTextCursor::KeepAnchor);
        curBrush = cursor.charFormat().foreground();

        // Then we position the end cursor to the start cursor position
        cursor.setPosition (iCursorPos, QTextCursor::KeepAnchor);

        do
        {
            cursor.setPosition (iCursorPos+iBlockLen+1, QTextCursor::KeepAnchor);
            nextCharBrush = cursor.charFormat().foreground();

            if (curBrush != nextCharBrush || (iCursorPos+iBlockLen >= endPos))
            {
                bEndofTheSameBlock = true;
                break;
            }

            iBlockLen++;

        }while(!bEndofTheSameBlock);


        //setting new parameters
        if (delegated()->scene()->isDarkBackground())
        {
            if (curBrush.color() == Qt::black)
            {
                curBrush = QBrush(Qt::white);
            }
        }
        else
        {
            if (curBrush.color() == Qt::white)
            {
                curBrush = QBrush(Qt::black);
            }
        }

        cursor.setPosition (iCursorPos+iBlockLen, QTextCursor::KeepAnchor);
        textFormat.setForeground(curBrush);
        cursor.mergeCharFormat(textFormat);

        iCursorPos += iBlockLen;
        cursor.setPosition (iCursorPos, QTextCursor::MoveAnchor);

        curFont = nextCharFont;
    }

    delegated()->setFont(curFont);
    //returning initial selection
    cursor.setPosition (anchorPos, QTextCursor::MoveAnchor);
    cursor.setPosition (cursorPos, QTextCursor::KeepAnchor);

    delegated()->setTextCursor(cursor);
}

void UBGraphicsTextItemDelegate::updateAlighButtonState()
{
    if (!mAlignButton) {
        return;
    }

    qDebug() << "new cursor position" << delegated()->textCursor().position();
    AlignTextButton *asAlBtn = static_cast<AlignTextButton*>(mAlignButton);

    if (!oneBlockSelection()) {
        asAlBtn->setMixedButtonVisible(true);
        asAlBtn->setKind(AlignTextButton::k_mixed);
        saveTextCursorFormats();
        return;
    }

    asAlBtn->setMixedButtonVisible(false);

    Qt::Alignment cf = delegated()->textCursor().blockFormat().alignment();
    qDebug() << "getting alignment" << cf;

    if (cf & Qt::AlignCenter) {
        asAlBtn->setKind(AlignTextButton::k_center);
    } else if (cf & Qt::AlignRight) {
        asAlBtn->setKind(AlignTextButton::k_right);
    } else {
        asAlBtn->setKind(AlignTextButton::k_left);
    }
}

bool UBGraphicsTextItemDelegate::oneBlockSelection()
{
    const QTextCursor cursor = delegated()->textCursor();
    int pos = cursor.position();
    int anchor = cursor.anchor();

    // no selection
    if (pos == anchor) {
        return true;
    }

    //selecton within one text block
    QTextBlock blck = cursor.block();
    if (blck.contains(pos) && blck.contains(anchor)) {
        return true;
    }

    //otherwise
    return false;
}

void UBGraphicsTextItemDelegate::saveTextCursorFormats()
{
    mSelectionData.anchor = delegated()->textCursor().anchor();
    mSelectionData.position = delegated()->textCursor().position();
    mSelectionData.html = delegated()->document()->toHtml();
}

void UBGraphicsTextItemDelegate::restoreTextCursorFormats()
{
    delegated()->document()->setHtml(mSelectionData.html);

    int min = qMin(mSelectionData.position, mSelectionData.anchor);
    int max = qMax(mSelectionData.position, mSelectionData.anchor);
    int steps = max - min;

    QTextCursor tcrsr = delegated()->textCursor();
    tcrsr.setPosition(mSelectionData.position);
    tcrsr.movePosition(QTextCursor::Left, QTextCursor::KeepAnchor, steps);
    delegated()->setTextCursor(tcrsr);
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
