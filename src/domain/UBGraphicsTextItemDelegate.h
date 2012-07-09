/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

    public:
        UBGraphicsTextItemDelegate(UBGraphicsTextItem* pDelegated, QObject * parent = 0);
        virtual ~UBGraphicsTextItemDelegate();
        bool isEditable();

    public slots:
        void contentsChanged();
        virtual void setEditable(bool);

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
        void ChangeTextSize(int delta);

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
