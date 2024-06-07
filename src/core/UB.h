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




#ifndef UB_H_
#define UB_H_

#include <QtWidgets>


#define UB_MAX_ZOOM 9

#if defined(Q_OS_LINUX) || (defined(Q_OS_OSX) && (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0)))
#define UB_REQUIRES_MASK_UPDATE 1
#endif

struct UBMimeType
{
    enum Enum
    {
        RasterImage = 0,
        VectorImage,
        AppleWidget,
        W3CWidget,
        Video,
        Audio,
        Flash,
        PDF,
        OpenboardTool,
        Group,
        Html,
        Document,
        UNKNOWN
    };
};

struct UBStylusTool
{
    enum Enum
    {
        Pen = 0,
        Eraser,
        Marker,
        Selector,
        Play,
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

// Deprecated. Keep it for backward campability with old versions
struct UBItemLayerType
{
    enum Enum
    {
        FixedBackground = -2000, Object = -1000, Graphic = 0, Tool = 1000, Control = 2000
    };
};

struct itemLayerType
{
    enum Enum {
        NoLayer = 0
        , BackgroundItem
        , ObjectItem
        , DrawingItem
        , ToolItem
        , CppTool
        , Eraiser
        , Curtain
        , Pointer
        , Cache
        , SelectedItem
        , SelectionFrame
    };
};


struct UBGraphicsItemData
{
    enum Enum
    {
        ItemLayerType //Deprecated. Keep it for backward campability with old versions. Use itemLayerType instead
        , ItemLocked
        , ItemEditable//for text only
        , ItemOwnZValue
        , itemLayerType //use instead of deprecated ItemLayerType
        , ItemUuid //storing uuid in QGraphicsItem for fast finding operations
        //Duplicating delegate's functions to make possible working with pure QGraphicsItem
        , ItemFlippable // (bool)
        , ItemRotatable // (bool)
        , ItemCanBeSetAsBackground
        , ItemIsHiddenOnDisplay
    };
};



struct UBGraphicsItemType
{
    enum Enum
    {
        PolygonItemType = QGraphicsItem::UserType + 1,  //65537
        PixmapItemType,                                 //65538
        SvgItemType,                                    //65539
        DelegateButtonType,                             //65540
        MediaItemType,                                  //65541
        VideoItemType,                                  //65542
        AudioItemType,                                  //65543
        PDFItemType,                                    //65544
        TextItemType,                                   //65545
        CurtainItemType,                                //65546
        RulerItemType,                                  //65547
        CompassItemType,                                //65548
        ProtractorItemType,                             //65549
        StrokeItemType,                                 //65550
        TriangleItemType,                               //65551
        MagnifierItemType,                              //65552
        cacheItemType,                                  //65553
        groupContainerType,                             //65554
        ToolWidgetItemType,                             //65555
        GraphicsWidgetItemType,                         //65556
        UserTypesCount,                                 //65557
        AxesItemType,                                   //65558
        SelectionFrameType                              // this line must be the last line in this enum because it is types counter.
    };
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
    ,GF_RESPECT_RATIO                = 0x0400 //0000 0100 0000 0000
    ,GF_TITLE_BAR_USED               = 0x0800 //0000 1000 0000 0000
    ,GF_COMMON                       = 0x00F8 /*0000 0000 1111 1000   GF_SCALABLE_ALL_AXIS
                                                                     |GF_DUPLICATION_ENABLED
                                                                     |GF_MENU_SPECIFIED
                                                                     |GF_ZORDER_MANIPULATIONS_ALLOWED */
    ,GF_ALL                          = 0xFFFF //1111 1111 1111 1111
};
Q_DECLARE_FLAGS(UBGraphicsFlags, UBGraphicsFlag)
Q_DECLARE_OPERATORS_FOR_FLAGS(UBGraphicsFlags )

struct DocumentSizeRatio
{
    enum Enum
    {
        Ratio4_3 = 0, Ratio16_9, Custom
    };
};


struct UBUndoType
{
    enum Enum
    {
        undotype_UNKNOWN  = 0, undotype_DOCUMENT, undotype_GRAPHICITEMTRANSFORM, undotype_GRAPHICITEM, undotype_GRAPHICTEXTITEM, undotype_PAGESIZE, undotype_GRAPHICSGROUPITEM, undotype_GRAPHICITEMZVALUE
    };
};

/*
 * Qt Version Compatibility
 */

namespace UB {
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
typedef Qt::SplitBehaviorFlags SplitBehavior;
#else
typedef QString::SplitBehavior SplitBehavior;
#endif

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
typedef QEvent EnterEvent;
#else
typedef QEnterEvent EnterEvent;
#endif

}

#endif /* UB_H_ */
