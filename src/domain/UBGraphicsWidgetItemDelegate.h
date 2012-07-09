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

#ifndef UBGRAPHICSWIDGETITEMDELEGATE_H_
#define UBGRAPHICSWIDGETITEMDELEGATE_H_

#include <QtGui>

#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsWidgetItem.h"


class UBGraphicsWidgetItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT

    public:
        UBGraphicsWidgetItemDelegate(UBGraphicsWidgetItem* pDelegated, int widgetType = 0);
        virtual ~UBGraphicsWidgetItemDelegate();

    protected:

        virtual void decorateMenu(QMenu* menu);
        virtual void updateMenuActionState();
        virtual void remove(bool canundo);

    private slots:

        void freeze(bool frozeon);
        void pin();

    private:

        int mWidgetType;

        UBGraphicsWidgetItem* delegated();

        QAction* freezeAction;
        QAction* setAsToolAction;

};


#endif /* UBGRAPHICSWIDGETITEMDELEGATE_H_ */
