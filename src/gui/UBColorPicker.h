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


#ifndef UBCOLORPICKER_H_
#define UBCOLORPICKER_H_

#include <QtGui>

class UBColorPicker : public QFrame
{

    Q_OBJECT;

    public:
        UBColorPicker(QWidget* parent);
        UBColorPicker(QWidget* parent, const QList<QColor>& colors, int pSelectedColorIndex = 0);
        virtual ~UBColorPicker();
        QList<QColor> getColors() const
        {
            return mColors;
        }

        void setColors(const QList<QColor>& pColors)
        {
            mColors = pColors;
            repaint();
        }

        int selectedColorIndex() const
        {
            return mSelectedColorIndex;
        }

        void setSelectedColorIndex(int pSelectedColorIndex)
        {
            mSelectedColorIndex = pSelectedColorIndex;
            repaint();
        }

    signals:
        void colorSelected(const QColor& color);

    protected:

        virtual void paintEvent ( QPaintEvent * event );
        virtual void mousePressEvent ( QMouseEvent * event );


    private:
        QList<QColor> mColors;
        int mSelectedColorIndex;
};

#endif /* UBCOLORPICKER_H_ */
