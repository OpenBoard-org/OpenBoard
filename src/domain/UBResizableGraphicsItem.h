/*
 * UBResisableGraphicsItem.h
 *
 *  Created on: 30 juin 2009
 *      Author: Luc
 */

#ifndef UBRESIZABLEGRAPHICSITEM_H_
#define UBRESIZABLEGRAPHICSITEM_H_

#include <QtGui>

class UBResizableGraphicsItem
{
    public:
        UBResizableGraphicsItem();
        virtual ~UBResizableGraphicsItem();

        virtual void resize(const QSizeF& pSize);
        virtual void resize(qreal w, qreal h) = 0;

        virtual QSizeF size() const = 0;

};

#endif /* UBRESIZABLEGRAPHICSITEM_H_ */
