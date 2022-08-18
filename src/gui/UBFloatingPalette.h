/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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




#ifndef UBFLOATINGPALLETTE_H_
#define UBFLOATINGPALLETTE_H_

#include <QWidget>
#include <QPoint>

typedef enum
{
    eMinimizedLocation_None,
    eMinimizedLocation_Left,
    eMinimizedLocation_Top,
    eMinimizedLocation_Right,
    eMinimizedLocation_Bottom
}eMinimizedLocation;

class UBFloatingPalette : public QWidget
{
    Q_OBJECT

    public:

        UBFloatingPalette(Qt::Corner = Qt::TopLeftCorner, QWidget *parent = 0);

        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);

        /**
         * @brief adjustSizeAndPosition adjusts the size of the palette. In case it is within a parent
         * widget, the size must be within the parent widget.
         * @param alignUp This is a special function that controls what happens if the height changes.
         * If in this case alignUp is true, the palette is moved up. If alignUp is false, the palette
         * is moved down.
         */
        virtual void adjustSizeAndPosition(bool alignUp = true);

        void setCustomPosition(bool pFlag);

        QSize preferredSize();

        void setBackgroundBrush(const QBrush& brush);
        void setGrip(bool newGrip);

    protected:

        virtual void enterEvent(QEvent *event);
        virtual void showEvent(QShowEvent *event);
        virtual void paintEvent(QPaintEvent *event);

        virtual int radius();
        virtual int border();
        virtual int gripSize();

        QBrush mBackgroundBrush;
        bool mbGrip;
        static const int sLayoutContentMargin = 12;
        static const int sLayoutSpacing = 15;
        void moveInsideParent(const QPoint &position);
        bool mCustomPosition;
        bool mIsMoving;

        std::function<int()> getParentRightOffset;

    private:
        virtual void minimizePalette(const QPoint& pos){Q_UNUSED(pos)};
        virtual void minimizeMe(){};

        QPoint mDragPosition;
        Qt::Corner mDefaultPosition;

    signals:
        void mouseEntered();
        void maximized();
        void moving();
};


#endif /* UBFLOATINGPALLETTE_H_ */
