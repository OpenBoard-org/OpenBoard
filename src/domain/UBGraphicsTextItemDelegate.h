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




#ifndef UBGRAPHICSTEXTITEMDELEGATE_H_
#define UBGRAPHICSTEXTITEMDELEGATE_H_

#include <QtGui>

#include <QtSvg>

#include "core/UB.h"
#include "UBGraphicsItemDelegate.h"

class UBGraphicsTextItem;

class AlignTextButton : public DelegateButton
{
    Q_OBJECT

public:
    static const int MAX_KIND = 3;
    enum kind_t{
        k_left = 0
        , k_center
        , k_right
        , k_mixed
    };

    AlignTextButton(const QString & fileName, QGraphicsItem* pDelegated, QGraphicsItem * parent = 0, Qt::WindowFrameSection section = Qt::TopLeftSection);
    virtual ~AlignTextButton();

    void setKind(int pKind);
    int kind() {return mKind;}

    void setNextKind();
    int nextKind() const;

    void setMixedButtonVisible(bool v = true) {mHideMixed = !v;}
    bool isMixedButtonVisible() {return !mHideMixed;}

private:

    QSvgRenderer *rndFromKind(int pknd)
    {
        switch (pknd) {
        case k_left:
            return lft;
            break;
        case k_center:
            return cntr;
            break;
        case k_right:
            return rght;
            break;
        case k_mixed:
            return mxd;
            break;
        }

        return 0;
    }

    QSvgRenderer *curRnd() {return rndFromKind(mKind);}

    QPointer<QSvgRenderer> lft;
    QPointer<QSvgRenderer> cntr;
    QPointer<QSvgRenderer> rght;
    QPointer<QSvgRenderer> mxd;

    int mKind;
    bool mHideMixed;
};

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
        void recolor();
        virtual QVariant itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant &value);
        virtual void createControls();
        qreal titleBarWidth();

    public slots:
        void contentsChanged();
        virtual void setEditable(bool);
        virtual void remove(bool canUndo);

    protected:
        virtual void decorateMenu(QMenu *menu);
        virtual void updateMenuActionState();

        virtual void freeButtons();

        virtual bool mousePressEvent(QGraphicsSceneMouseEvent *event);
        virtual bool mouseMoveEvent(QGraphicsSceneMouseEvent *event);
        virtual bool mouseReleaseEvent(QGraphicsSceneMouseEvent *event);

        virtual bool keyPressEvent(QKeyEvent *event);
        virtual bool keyReleaseEvent(QKeyEvent *event);

    private:
        UBGraphicsTextItem* delegated();

        DelegateButton* mFontButton;
        DelegateButton* mColorButton;
        DelegateButton* mDecreaseSizeButton;
        DelegateButton* mIncreaseSizeButton;
        DelegateButton* mAlignButton;

        int mLastFontPixelSize;

        static const int sMinPixelSize;
        static const int sMinPointSize;

    private:
        void customize(QFontDialog &fontDialog);
        void ChangeTextSize(qreal factor, textChangeMode changeMode);
        void updateAlighButtonState();
        bool oneBlockSelection();
        void saveTextCursorFormats();
        void restoreTextCursorFormats();


        QFont createDefaultFont();
        QAction *mEditableAction;
        struct selectionData_t {
            selectionData_t()
                : mButtonIsPressed(false)
            {}
            bool mButtonIsPressed;
            int position;
            int anchor;
            QString html;
            QTextDocumentFragment selection;
            QList<QTextBlockFormat> fmts;

        } mSelectionData;

    private slots:

        void pickFont();
        void pickColor();

        void decreaseSize();
        void increaseSize();

        void alignButtonProcess();
        void onCursorPositionChanged(const QTextCursor& cursor);
        void onModificationChanged(bool ch);
        void onContentChanged();

private:
      const int delta;
};

#endif /* UBGRAPHICSTEXTITEMDELEGATE_H_ */
