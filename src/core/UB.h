/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#ifndef UB_H_
#define UB_H_

#include <QtGui>

#define UB_MAX_ZOOM 15

struct UBStylusTool
{
    enum Enum
    {
        Pen = 0, 
		Eraser, 
 		Marker, 
		Selector, 
		Hand, 
		ZoomIn, 
		ZoomOut, 
		Pointer, 
		Line, 
		Text, 
		Capture
    };
};

struct UBWidth
{
    enum Enum
    {
        Fine = 0, Medium, Strong
    };
};

struct UBZoom
{
    enum Enum
    {
        Small = 0, Medium, Large
    };
};

struct UBSize
{
    enum Enum
    {
        Small = 0, Medium, Large
    };
};


struct UBItemLayerType
{
    enum Enum
    {
        FixedBackground = -2000, Object = -1000, Graphic = 0, Tool = 1000, Control = 2000
    };
};

struct UBGraphicsItemData
{
    enum Enum
    {
        ItemLayerType
        , ItemLocked
        , ItemEditable//for text only
    };
};

struct UBGraphicsItemType
{
    enum Enum
    {
        PolygonItemType = QGraphicsItem::UserType + 1, 
		PixmapItemType,
        SvgItemType, 
		VideoItemType, 
		AudioItemType, 
		AppleWidgetItemType, 
		PDFItemType, 
		TextItemType, 
		W3CWidgetItemType, 
		CurtainItemType,
        RulerItemType, 
		CompassItemType, 
		ProtractorItemType, 
		StrokeItemType,
		TriangleItemType,
        MagnifierItemType,
        cacheItemType
    };
};

struct DocumentSizeRatio
{
    enum Enum
    {
        Ratio4_3 = 0, Ratio16_9, Custom
    };
};

#endif /* UB_H_ */
