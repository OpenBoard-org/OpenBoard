/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBITEM_H
#define UBITEM_H

#include <QtGui>
#include "domain/UBGraphicsItemDelegate.h"
#include "core/UB.h"

class UBGraphicsScene;
class UBGraphicsItem;

class UBItem
{
    protected:

        UBItem();

    public:

        virtual ~UBItem();

        enum RenderingQuality
        {
            RenderingQualityNormal = 0, RenderingQualityHigh
        };

        virtual QUuid uuid() const
        {
                return mUuid;
        }

        virtual void setUuid(const QUuid& pUuid)
        {
                mUuid = pUuid;
        }

        virtual RenderingQuality renderingQuality() const
        {
            return mRenderingQuality;
        }

        virtual void setRenderingQuality(RenderingQuality pRenderingQuality)
        {
            mRenderingQuality = pRenderingQuality;
        }

        virtual UBItem* deepCopy() const = 0;

        virtual void copyItemParameters(UBItem *copy) const = 0;

        virtual UBGraphicsScene* scene() // TODO UB 4.x should be pure virtual ...
        {
            return 0;
        }

        virtual QUrl sourceUrl() const
        {
            return mSourceUrl;
        }

        virtual void setSourceUrl(const QUrl& pSourceUrl)
        {
            mSourceUrl = pSourceUrl;
        }

    protected:

        QUuid mUuid;

        RenderingQuality mRenderingQuality;

        QUrl mSourceUrl;

};


class UBGraphicsItem
{
protected:

    UBGraphicsItem() : mDelegate(0)
    {
        // NOOP
    }
    UBGraphicsItemDelegate* mDelegate;

    virtual ~UBGraphicsItem()
    {
        // NOOP
    }

public:

    static void assignZValue(QGraphicsItem*, qreal value);
    static bool isRotatable(QGraphicsItem *item);
    static bool isFlippable(QGraphicsItem *item);

    virtual UBGraphicsItemDelegate *Delegate() const  = 0;

    virtual void remove() = 0;

    virtual void clearSource(){;}
};

#endif // UBITEM_H
