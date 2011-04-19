/*
 * UBSetting.cpp
 *
 *  Created on: Nov 24, 2008
 *      Author: luc
 */

#include "UBSetting.h"

#include <QtGui>

#include "UBSettings.h"


UBSetting::UBSetting(UBSettings* parent) :
    QObject(parent)
{
    //NOOP
}

UBSetting::UBSetting(UBSettings* owner, const QString& pDomain, const QString& pKey,
        const QVariant& pDefaultValue) :
    QObject(owner), mOwner(owner), mDomain(pDomain), mKey(pKey), mPath(
            pDomain + "/" + pKey), mDefaultValue(pDefaultValue)
{
    //NOOP
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
    list.replace(pIndex, color.name());
    QColor c = color;
    c.setAlphaF(mAlpha);
    mColors.replace(pIndex, c);
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
