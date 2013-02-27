/*
 * Copyright (C) 2012 Webdoc SA
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

class UBGraphicsTextItem;

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


    public slots:
        void contentsChanged();
        virtual void setEditable(bool);
        virtual void remove(bool canUndo);

    protected:
        virtual void buildButtons();
        virtual void decorateMenu(QMenu *menu);
        virtual void updateMenuActionState();

        virtual void positionHandles();

    private:

        UBGraphicsTextItem* delegated();

        DelegateButton* mFontButton;
        DelegateButton* mColorButton;
        DelegateButton* mDecreaseSizeButton;
        DelegateButton* mIncreaseSizeButton;

        int mLastFontPixelSize;

        static const int sMinPixelSize;
        static const int sMinPointSize;

    private:
        void customize(QFontDialog &fontDialog);
        void ChangeTextSize(qreal factor, textChangeMode changeMode);

        QFont createDefaultFont();
        QAction *mEditableAction;

    private slots:

        void pickFont();
        void pickColor();

        void decreaseSize();
        void increaseSize();

private:
      const int delta;

};

#endif /* UBGRAPHICSTEXTITEMDELEGATE_H_ */
