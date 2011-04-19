/*
 * UBResizableGraphicsItem.cpp
 *
 *  Created on: 30 juin 2009
 *      Author: Luc
 */

#include "UBResizableGraphicsItem.h"

UBResizableGraphicsItem::UBResizableGraphicsItem()
{
    // NOOP

}

UBResizableGraphicsItem::~UBResizableGraphicsItem()
{
    // NOOP
}

void UBResizableGraphicsItem::resize(const QSizeF& pSize)
{
    resize(pSize.width(), pSize.height());
}
