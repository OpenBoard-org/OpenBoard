/*
 * UB.h
 *
 *  Created on: Sep 16, 2008
 *      Author: luc
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
        ItemLayerType, ItemLocked
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
		TriangleItemType
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
