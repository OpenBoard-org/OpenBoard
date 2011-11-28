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

#include "UBGraphicsTextItemDelegate.h"
#include "UBGraphicsScene.h"
#include "domain/UBGraphicsTextItem.h"
#include "domain/UBGraphicsDelegateFrame.h"
#include "core/UBSettings.h"

#include "core/UBApplication.h" // TODO UB 4.x clean that dependency
#include "core/UBApplicationController.h" // TODO UB 4.x clean that dependency
#include "core/UBDisplayManager.h" // TODO UB 4.x clean that dependency

#include "core/memcheck.h"
#include "board/UBBoardController.h"

const int UBGraphicsTextItemDelegate::sMinPixelSize = 8;

UBGraphicsTextItemDelegate::UBGraphicsTextItemDelegate(UBGraphicsTextItem* pDelegated, QObject * parent)
    : UBGraphicsItemDelegate(pDelegated,0, parent, true)
    , mLastFontPixelSize(-1)
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

    delegated()->adjustSize();
    delegated()->contentsChanged();

//    QTextCursor defcursor(createDefaultCursor());
//    defcursor.movePosition(QTextCursor::Start);
//    delegated()->setTextCursor(defcursor);


    //    QFont selectedFont = fontDialog.selectedFont();
//    UBSettings::settings()->setFontFamily(selectedFont.family());
//    QFontInfo fi(selectedFont);
//    mLastFontPixelSize = fi.pixelSize();
//    UBSettings::settings()->setFontPixelSize(mLastFontPixelSize);
//    UBSettings::settings()->setBoldFont(selectedFont.bold());
//    UBSettings::settings()->setItalicFont(selectedFont.italic());
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
    int pixSize = UBSettings::settings()->fontPixelSize();
    if (pixSize > 0) {
        mLastFontPixelSize = pixSize;
        font.setPixelSize(pixSize);
    }

    return font;
}

void UBGraphicsTextItemDelegate::buildButtons()
{
    UBGraphicsItemDelegate::buildButtons();

    mFontButton = new DelegateButton(":/images/font.svg", mDelegated, mFrame);
    mColorButton = new DelegateButton(":/images/color.svg", mDelegated, mFrame);
    mDecreaseSizeButton = new DelegateButton(":/images/minus.svg", mDelegated, mFrame);
    mIncreaseSizeButton = new DelegateButton(":/images/plus.svg", mDelegated, mFrame);

    connect(mFontButton, SIGNAL(clicked(bool)), this, SLOT(pickFont()));
    connect(mColorButton, SIGNAL(clicked(bool)), this, SLOT(pickColor()));
    connect(mDecreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(decreaseSize()));
    connect(mIncreaseSizeButton, SIGNAL(clicked(bool)), this, SLOT(increaseSize()));

    mButtons << mFontButton << mColorButton << mDecreaseSizeButton << mIncreaseSizeButton;
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

    if (UBSettings::settings()->isDarkBackground())
    {
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
            //safeWebFontNames.append("Century Gothic"); Not available on OSX
            safeWebFontNames.append("Courier New");
            safeWebFontNames.append("Georgia");
            safeWebFontNames.append("Impact");
            safeWebFontNames.append("Times New Roman");
            safeWebFontNames.append("Trebuchet MS");
            safeWebFontNames.append("Verdana");
            int index = 0;
            foreach (QString dialogFontName, dialogFontNames)
            {
                if (safeWebFontNames.contains(dialogFontName, Qt::CaseInsensitive))
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
            QFontInfo fi(selectedFont);
            mLastFontPixelSize = fi.pixelSize();
            UBSettings::settings()->setFontPixelSize(mLastFontPixelSize);
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
//            format.setBackground(Qt::yellow);
            curCursor.mergeCharFormat(format);
            QTextBlockFormat blFormat;
            blFormat.setAlignment(Qt::AlignCenter);
            curCursor.setBlockFormat(blFormat);
            delegated()->setTextCursor(curCursor);

            UBGraphicsTextItem::lastUsedTextColor = selectedColor;

            delegated()->setSelected(true);
            delegated()->contentsChanged();
        }
    }
}

void UBGraphicsTextItemDelegate::decreaseSize()
{
//    QFontInfo fi(delegated()->font());
//    int pixelSize = fi.pixelSize();
//    if (-1 == mLastFontPixelSize)
//        mLastFontPixelSize = pixelSize;

//    int newPixelSize = sMinPixelSize;
//    while (newPixelSize * 1.5 < pixelSize)
//        newPixelSize *= 1.5;

//    if (newPixelSize < mLastFontPixelSize && mLastFontPixelSize < pixelSize)
//        newPixelSize = mLastFontPixelSize;

//    if (pixelSize > newPixelSize)
//    {
//        QFont font = delegated()->font();
//        font.setPixelSize(newPixelSize);
//        delegated()->setFont(font);
//        UBSettings::settings()->setFontPixelSize(newPixelSize);

//        delegated()->document()->adjustSize();
//        delegated()->contentsChanged();dddd
//    }
}

void UBGraphicsTextItemDelegate::increaseSize()
{
//    QFontInfo fi(delegated()->font());
//    int pixelSize = fi.pixelSize();
//    if (-1 == mLastFontPixelSize)
//        mLastFontPixelSize = pixelSize;

//    int newPixelSize = sMinPixelSize;
//    while (newPixelSize <= pixelSize)
//        newPixelSize *= 1.5;

//    if (pixelSize < mLastFontPixelSize && mLastFontPixelSize < newPixelSize)
//        newPixelSize = mLastFontPixelSize;

//    QFont font = delegated()->font();
//    font.setPixelSize(newPixelSize);
//    delegated()->setFont(font);
//    UBSettings::settings()->setFontPixelSize(newPixelSize);

//    delegated()->document()->adjustSize();
//    delegated()->contentsChanged();

//    qDebug() << newPixelSize;
    delegated()->document()->adjustSize();
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
    setEditable(isEditable());
}
