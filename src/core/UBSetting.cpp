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




#include "UBSetting.h"

#include <QtGui>

#include "UBSettings.h"

#include "core/memcheck.h"

UBSetting::UBSetting(UBSettings* parent) :
    QObject(parent)
{
    //NOOP
}

UBSetting::UBSetting(UBSettings* owner, const QString& pDomain, const QString& pKey,
        const QVariant& pDefaultValue) :
    QObject(owner), 
        mOwner(owner), 
        mDomain(pDomain), 
        mKey(pKey), 
        mPath(pDomain + "/" + pKey), 
        mDefaultValue(pDefaultValue)
{
    get(); // force caching of the setting
}

UBSetting::~UBSetting()
{
    // NOOP
}

void UBSetting::set(const QVariant& pValue)
{
    if (pValue != get())
    {
        mOwner->setValue(mPath, pValue);
        emit changed(pValue);
    }
}

QVariant UBSetting::get()
{
    return mOwner->value(mPath, mDefaultValue);
}

QVariant UBSetting::reset()
{
    set(mDefaultValue);
    return mDefaultValue;
}


void UBSetting::setBool(bool pValue)
{
    set(pValue);
}

void UBSetting::setString(const QString& pValue)
{
    set(pValue);
}
void UBSetting::setInt(int pValue)
{
    set(pValue);
}


UBColorListSetting::UBColorListSetting(UBSettings* parent)
    : UBSetting(parent)
{
    //NOOP
}

UBColorListSetting::UBColorListSetting(UBSettings* owner, const QString& pDomain,
        const QString& pKey, const QVariant& pDefaultValue, qreal pAlpha)
    : UBSetting(owner, pDomain, pKey, pDefaultValue)
    , mAlpha(pAlpha)
{

    foreach(QString s, get().toStringList())
    {
        QColor color;
        color.setNamedColor(s);
        if (mAlpha>=0)
            color.setAlphaF(mAlpha);
        mColors.append(color);
    }

}

UBColorListSetting::~UBColorListSetting()
{
    // NOOP
}

QVariant UBColorListSetting::reset()
{
    QVariant result = UBSetting::reset();

    mColors.clear();

    foreach(QString s, get().toStringList())
    {
        QColor color;
        color.setNamedColor(s);
        if (mAlpha>=0)
            color.setAlphaF(mAlpha);

        mColors.append(color);
    }

    mOwner->colorChanged();

    return result;
}

QList<QColor> UBColorListSetting::colors() const
{
    return mColors;
}

void UBColorListSetting::setColor(int pIndex, const QColor& color)
{
    QStringList list = get().toStringList();
    list.replace(pIndex, color.name(QColor::HexArgb));
    if (mAlpha>=0)
    {
        QColor c = color;
        c.setAlphaF(mAlpha);
        mColors.replace(pIndex, c);
    }
    else
        mColors.replace(pIndex, color);
    set(list);
}

void UBColorListSetting::setAlpha(qreal pAlpha)
{
    mAlpha = pAlpha;

    for(int i = 0 ; i < mColors.size() ; i ++)
    {
        QColor c = mColors.at(i);
        c.setAlphaF(mAlpha);
        mColors.replace(i, c);
    }

    mOwner->colorChanged();
}
