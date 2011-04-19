/*
 * UBColorPicker.cpp
 *
 *  Created on: Nov 19, 2008
 *      Author: luc
 */

#include "UBColorPicker.h"

#include <QtGui>

UBColorPicker::UBColorPicker(QWidget* parent)
    : QFrame(parent)
    , mSelectedColorIndex(0)
{
    // NOOP
}

UBColorPicker::UBColorPicker(QWidget* parent, const QList<QColor>& colors, int pSelectedColorIndex)
    : QFrame(parent)
    , mColors(colors)
    , mSelectedColorIndex(pSelectedColorIndex)
{
    // NOOP
}


UBColorPicker::~UBColorPicker()
{
    // NOOP
}


void UBColorPicker::paintEvent ( QPaintEvent * event )
{
    Q_UNUSED(event);

    QPainter painter(this);

    if (mSelectedColorIndex < mColors.size())
    {
        painter.setRenderHint(QPainter::Antialiasing);

        painter.setBrush(mColors.at(mSelectedColorIndex));

        painter.drawRect(0, 0, width(), height());
    }

}

void UBColorPicker::mousePressEvent ( QMouseEvent * event )
{
    if (event->buttons() & Qt::LeftButton)
    {
        mSelectedColorIndex++;

        if (mSelectedColorIndex >= mColors.size())
            mSelectedColorIndex = 0;

        repaint();

        emit colorSelected(mColors.at(mSelectedColorIndex));

    }
}


