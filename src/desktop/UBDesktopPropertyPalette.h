/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UBDESKTOPPENPALETTE_H
#define UBDESKTOPPENPALETTE_H

#include <QtGui>
#include <QResizeEvent>

#include "gui/UBPropertyPalette.h"

class UBRightPalette;

class UBDesktopPropertyPalette : public UBPropertyPalette
{
    Q_OBJECT

    public:
        UBDesktopPropertyPalette(QWidget *parent, UBRightPalette* _rightPalette);
    private:
        UBRightPalette* rightPalette;
    protected:
        virtual int getParentRightOffset();
};

class UBDesktopPenPalette : public UBDesktopPropertyPalette
{
    Q_OBJECT
    public:
        UBDesktopPenPalette(QWidget *parent, UBRightPalette* rightPalette);
        virtual ~UBDesktopPenPalette(){}
    public slots:
        void onParentMinimized();
        void onParentMaximized();

    private slots:
        void onButtonReleased();

};

class UBDesktopEraserPalette : public UBDesktopPropertyPalette
{
    public:
        UBDesktopEraserPalette(QWidget *parent, UBRightPalette* rightPalette);
        virtual ~UBDesktopEraserPalette(){}
};

class UBDesktopMarkerPalette : public UBDesktopPropertyPalette
{
    public:
        UBDesktopMarkerPalette(QWidget *parent, UBRightPalette* rightPalette);
        virtual ~UBDesktopMarkerPalette(){}
};


#endif // UBDESKTOPPENPALETTE_H
