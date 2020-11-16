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




#ifndef UBITEM_H
#define UBITEM_H

#include <QtGui>
#include "core/UB.h"
#include "domain/UBGraphicsItemDelegate.h"

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

        enum CacheBehavior
        {
            CacheNotAllowed,
            CacheAllowed,
            nbrCacheBehavior
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

        virtual void setCacheBehavior(CacheBehavior cacheBehavior)
        {
            mCacheBehavior = cacheBehavior;
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

        CacheBehavior mCacheBehavior;
};

class UBGraphicsItem
{
protected:
    UBGraphicsItem() : mDelegate(NULL)
    {
        // NOOP
    }
    virtual ~UBGraphicsItem();
    void setDelegate(UBGraphicsItemDelegate* mDelegate);

public:
    virtual int type() const = 0;

    UBGraphicsItemDelegate *Delegate() const;

    static void assignZValue(QGraphicsItem*, qreal value);
    static bool isRotatable(QGraphicsItem *item);
    static bool isFlippable(QGraphicsItem *item);
    static bool isLocked(QGraphicsItem *item);
    static QUuid getOwnUuid(QGraphicsItem *item);

    static UBGraphicsItemDelegate *Delegate(QGraphicsItem *pItem);

    void remove(bool canUndo = true);

    virtual void clearSource(){}

private:
    UBGraphicsItemDelegate* mDelegate;
};

#endif // UBITEM_H
