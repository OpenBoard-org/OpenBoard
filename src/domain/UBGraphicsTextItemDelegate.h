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



#ifndef UBGRAPHICSTEXTITEMDELEGATE_H_
#define UBGRAPHICSTEXTITEMDELEGATE_H_

#include <QtGui>

#include <QtSvg>

#include "core/UB.h"
#include "UBGraphicsItemDelegate.h"
#include "gui/UBMainWindow.h"
#include "gui/UBCreateTablePalette.h"
#include "gui/UBCreateHyperLinkPalette.h"
#include "gui/UBCellPropertiesPalette.h"

class UBGraphicsTextItem;
class UBGraphicsProxyWidget;

class UBGraphicsTextItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT

    enum textChangeMode
    {
        changeSize = 0,
        scaleSize
    };

    public:
        UBGraphicsTextItemDelegate(UBGraphicsTextItem* pDelegated, QObject * parent = 0);
        virtual ~UBGraphicsTextItemDelegate();
        bool isEditable();
        void scaleTextSize(qreal multiplyer);
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);

        UBCreateTablePalette* tablePalette();
        UBCreateHyperLinkPalette* linkPalette();
        UBCellPropertiesPalette* cellPropertiesPalette();

        // Issue 16/03/2018 - OpenBoard - Two text-editor toolboards.
        void buildButtonsExtended();
        void buildButtonsReduced();
        // END Issue



        void changeDelegateButtonsMode(bool htmlMode);

    public slots:
        void contentsChanged();
        virtual void setEditable(bool);
        virtual void remove(bool canUndo);
        void alternHtmlMode();
        void duplicate();

    protected:
        virtual void buildButtons();
        virtual void decorateMenu(QMenu *menu);
        virtual void updateMenuActionState();

        virtual void positionHandles();

    private:

        UBGraphicsTextItem* delegated();

        DelegateButton* mSwtichTextEditorToolBar;
        DelegateButton* mFontButton;
        DelegateButton* mFontBoldButton;        
        DelegateButton* mFontItalicButton;
        DelegateButton* mFontUnderlineButton;
        DelegateButton* mColorButton;
        DelegateButton* mDecreaseSizeButton;
        DelegateButton* mIncreaseSizeButton;
        DelegateButton* mBackgroundColorButton;
        DelegateButton* mLeftAlignmentButton;
        DelegateButton* mCenterAlignmentButton;
        DelegateButton* mRightAlignmentButton;
        DelegateButton* mJustifyAlignmentButton;
        DelegateButton* mCodeButton;
        DelegateButton* mUnorderedListButton;
        DelegateButton* mOrderedListButton;
        DelegateButton* mAddIndentButton;
        DelegateButton* mRemoveIndentButton;
        DelegateButton* mHyperLinkButton;
        DelegateButton* mTableButton;

        UBCreateTablePalette* mTablePalette;
        UBCreateHyperLinkPalette* mLinkPalette;
        UBCellPropertiesPalette* mCellPropertiesPalette;

        // Issue 16/03/2018 - OpenBoard - Two text-editor toolboards.
        bool toolbarExtended;

        int mLastFontPixelSize;

        static const int sMinPixelSize;
        static const int sMinPointSize;

    private:
        void customize(QFontDialog &fontDialog);
        void ChangeTextSize(qreal factor, textChangeMode changeMode);

        QFont createDefaultFont();
        QAction *mEditableAction;

        QMenu * mTableMenu;

    private slots:

        // Issue 16/03/2018 - OpenBoard - Two text-editor toolboards.
        void swtichTextEditorToolBar();

        void pickFont();
        void setFontBold();
        void setFontItalic();
        void setFontUnderline();
        void pickColor();

        void decreaseSize();
        void increaseSize();

        void pickBackgroundColor();
        void setTableSize();
        void setCellProperties();
        void insertTable();
        void addIndent();
        void removeIndent();
        void insertOrderedList();
        void insertUnorderedList();

        void setAlignmentToLeft();
        void setAlignmentToCenter();
        void setAlignmentToJustify();
        void setAlignmentToRight();
        void addLink();
        void insertLink();
        void insertColumnOnRight();
        void insertColumnOnLeft();
        void insertRowOnBottom();
        void insertRowOnTop();
        void deleteColumn();
        void deleteRow();
        void applyCellProperties();
        void distributeColumn();
        void showMenuTable();

private:
      const int delta;
      void insertList(QTextListFormat::Style format);
      QTextListFormat::Style nextStyle(QTextListFormat::Style format);
      QTextListFormat::Style previousStyle(QTextListFormat::Style format);


};

#endif /* UBGRAPHICSTEXTITEMDELEGATE_H_ */
