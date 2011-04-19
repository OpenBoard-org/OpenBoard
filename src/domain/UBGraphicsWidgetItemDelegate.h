/*
 * UBGraphicsWidgetItemDelegate.h
 *
 *  Created on: October 09, 2009
 *      Author: Luc
 */

#ifndef UBGRAPHICSWIDGETITEMDELEGATE_H_
#define UBGRAPHICSWIDGETITEMDELEGATE_H_

#include <QtGui>

#include "UBGraphicsItemDelegate.h"
#include "UBGraphicsWidgetItem.h"


class UBGraphicsWidgetItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT;

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
