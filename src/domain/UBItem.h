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

// Might be fit in int value under most OS
enum UBGraphicsFlag {
    GF_NONE                          = 0x0000 //0000 0000 0000 0000
    ,GF_FLIPPABLE_X_AXIS             = 0x0001 //0000 0000 0000 0001
    ,GF_FLIPPABLE_Y_AXIS             = 0x0002 //0000 0000 0000 0010
    ,GF_FLIPPABLE_ALL_AXIS           = 0x0003 //0000 0000 0000 0011 GF_FLIPPABLE_X_AXIS | GF_FLIPPABLE_Y_AXIS
    ,GF_REVOLVABLE                   = 0x0004 //0000 0000 0000 0100
    ,GF_SCALABLE_X_AXIS              = 0x0008 //0000 0000 0000 1000
    ,GF_SCALABLE_Y_AXIS              = 0x0010 //0000 0000 0001 0000
    ,GF_SCALABLE_ALL_AXIS            = 0x0018 //0000 0000 0001 1000 GF_SCALABLE_X_AXIS | GF_SCALABLE_Y_AXIS
    ,GF_DUPLICATION_ENABLED          = 0x0020 //0000 0000 0010 0000
    ,GF_MENU_SPECIFIED               = 0x0040 //0000 0000 0100 0000
    ,GF_ZORDER_MANIPULATIONS_ALLOWED = 0x0080 //0000 0000 1000 0000
    ,GF_TOOLBAR_USED                 = 0x0100 //0000 0001 0000 0000
    ,GF_SHOW_CONTENT_SOURCE          = 0x0200 //0000 0010 0000 0000
    ,GF_COMMON                       = 0x00F8 /*0000 0000 1111 1000   GF_FLIPPABLE_ALL_AXIS
                                                                     |GF_DUPLICATION_ENABLED
                                                                     |GF_MENU_SPECIFIED
                                                                     |GF_ZORDER_MANIPULATIONS_ALLOWED */
    ,GF_ALL                          = 0xFFFF //1111 1111 1111 1111
};
Q_DECLARE_FLAGS(UBGraphicsFlags, UBGraphicsFlag)

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

    inline UBGraphicsItemDelegate *Delegate() const { return mDelegate; }

    static void assignZValue(QGraphicsItem*, qreal value);
    static bool isRotatable(QGraphicsItem *item);
    static bool isFlippable(QGraphicsItem *item);
    static QUuid getOwnUuid(QGraphicsItem *item);

    static UBGraphicsItemDelegate *Delegate(QGraphicsItem *pItem);

    void remove(bool canUndo = true);

    virtual void clearSource(){}

private:
    UBGraphicsItemDelegate* mDelegate;
};

#endif // UBITEM_H
