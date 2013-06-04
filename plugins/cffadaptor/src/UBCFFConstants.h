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


#ifndef UBCFFCONSTANTS_H
#define UBCFFCONSTANTS_H

#define PI 3.1415926535

const int DEFAULT_BACKGROUND_LAYER = -20000002;
const int DEFAULT_BACKGROUND_CROSS_LAYER = -20000001;

// Constant names. Use only them instead const char* in each function

// Constant fileNames;
const QString fMetadata = "metadata.rdf";
const QString fIWBContent = "content.xml";
const QString fIWBBackground = "background.png";
const QString sAudioElementImage = ":images/soundOn.svg";

// Constant messages;
const QString noErrorMsg = "NoError";

// Tag names
const QString tDescription = "Description";
const QString tIWBRoot = "iwb";
const QString tIWBMeta = "meta";
const QString tUBZSize = "size";
const QString tSvg = "svg";
const QString tIWBPage = "page";
const QString tIWBPageSet = "pageset";
const QString tId = "id";
const QString tElement = "element";
const QString tUBZGroup = "group";
const QString tUBZGroups = "groups";
const QString tUBZG = "g";
const QString tUBZPolygon = "polygon";
const QString tUBZPolyline = "polyline";
const QString tUBZLine = "line";
const QString tUBZAudio = "audio";
const QString tUBZVideo = "video";
const QString tUBZImage = "image";
const QString tUBZForeignObject = "foreignObject";
const QString tUBZTextContent = "itemTextContent";

const QString tIWBA = "a";
const QString tIWBG = "g";
const QString tIWBSwitch = "switch";
const QString tIWBImage = "image";
const QString tIWBVideo = "video";
const QString tIWBAudio = "audio";
const QString tIWBText = "text";
const QString tIWBTextArea = "textarea";
const QString tIWBPolyLine = "polyline";
const QString tIWBPolygon = "polygon";
const QString tIWBFlash = "video";
const QString tIWBRect = "rect";
const QString tIWBLine = "line";
const QString tIWBTbreak = "tbreak";
const QString tIWBTspan = "tspan";

// Attributes names
const QString aIWBVersion = "version";
const QString aOwner  = "owner";
const QString aDescription  = "description";
const QString aCreator  = "creator";
const QString aAbout  = "about";
const QString aIWBViewBox = "viewbox";
const QString aUBZViewBox = "viewBox";
const QString aDarkBackground = "dark-background";
const QString aBackground = "background";
const QString aCrossedBackground = "crossed-background";
const QString aUBZType = "type";
const QString aUBZUuid = "uuid";
const QString aUBZParent = "parent";
const QString aFill = "fill"; // IWB attribute contans color to fill

const QString aID = "id";   // ID of any svg element can be placed in to iwb section
const QString aRef = "ref"; // as reference for applying additional attributes
const QString aSVGHref = "xlink:href"; // reference to file
const QString aIWBHref = "ref"; // reference to element ID
const QString aUBZHref = "href";
const QString aUBZSource = "source";
const QString aSrc = "src";
const QString aSVGRequiredExtension = "requiredExtensions";

const QString aX = "x";
const QString aY = "y";
const QString aWidth = "width";
const QString aHeight = "height";
const QString aStroke = "stroke";
const QString aStrokeWidth = "stroke-width";
const QString aPoints = "points";
const QString aZLayer = "z-value";
const QString aLayer = "layer";
const QString aTransform = "transform";
const QString aLocked = "locked";
const QString aIWBName = "name";
const QString aIWBContent = "content";


// Attribute values
const QString avIWBVersionNo = "1.0"; 
const QString avUBZText = "text";
const QString avFalse = "false";
const QString avTrue = "true";

// Namespaces and prefixes
const QString svgRequiredExtensionPrefix = "http://www.imsglobal.org/iwb/";
const QString dcNS = "http://purl.org/dc/elements/1.1/";
const QString ubNS = "http://uniboard.mnemis.com/document";
const QString svgUBZNS = "http://www.imsglobal.org/xsd/iwb_v1p0";
const QString svgIWBNS = "http://www.w3.org/2000/svg";
const QString xlinkNS = "http://www.w3.org/1999/xlink";
const QString iwbNS = "http://www.imsglobal.org/xsd/iwb_v1p0";
const QString xsiNS = "http://www.w3.org/2001/XMLSchema-instance";
const QString xsiShemaLocation = "\
http://www.imsglobal.org/xsd/iwb_v1p0 \
http://www.imsglobal.org/profile/iwb/iwbv1p0_v1p0.xsd \
http://www.w3.org/2000/svg http://www.imsglobal.org/profile/iwb/svgsubsetv1p0_v1p0.xsd \
http://www.w3.org/1999/xlink http://www.imsglobal.org/xsd/w3/1999/xlink.xsd";
const QString dcNSPrefix = "dc";
const QString ubNSPrefix = "ub";
const QString svgIWBNSPrefix = "svg";
const QString xlinkNSPrefix = "xlink";
const QString iwbNsPrefix = "iwb";
const QString xsiPrefix = "xsi";
const QString xsiSchemaLocationPrefix = "schemaLocation";

const QString avOwner = "";
const QString avCreator = "";
const QString avDescription = "";

//constant symbols and words etc
const QString dimensionsDelimiter1 = "x";
const QString dimensionsDelimiter2 = " ";
const QString pageAlias = "page";
const QString pageFileExtentionUBZ = "svg";

//content folder names
const QString cfImages = "images";
const QString cfVideos = "video";
const QString cfAudios = "audio";
const QString cfFlash = "flash";

//known file extentions
const QString feSvg = "svg";
const QString feWgt = "wgt";
const QString fePng = "png";

const int iCrossSize = 32;
const int iCrossWidth = 1;

// Image formats supported by CFF exclude wgt. Wgt is Sankore widget, which is considered as a .png preview.
const QString iwbElementImage(" \
wgt, \
jpeg, \
jpg, \
bmp, \
gif, \
wmf, \
emf, \
png, \
tif, \
tiff \
");

// Video formats supported by CFF
const QString iwbElementVideo(" \
mpg, \
mpeg, \
swf, \
");

// Audio formats supported by CFF
const QString iwbElementAudio(" \
mp3, \
wav \
");

const QString cffSupportedFileFormats(iwbElementImage + iwbElementVideo + iwbElementAudio);
const QString ubzFormatsToConvert("svg");


const QString iwbSVGImageAttributes(" \
id, \
xlink:href, \
x, \
y, \
height, \
width, \
fill-opacity, \
requiredExtentions, \
transform \
");


const QString iwbSVGAudioAttributes(" \
id, \
xlink:href, \
x, \
y, \
height, \
width, \
fill-opacity, \
requiredExtentions, \
transform \
");

const QString iwbSVGVideoAttributes(" \
id, \
xlink:href, \
x, \
y, \
height, \
width, \
fill-opacity, \
requiredExtentions, \
transform \
");

const QString iwbSVGRectAttributes(" \
id, \
x, \
y, \
height, \
width, \
fill, \
fill-opacity, \
stroke, \
stroke-dasharray, \
stroke-linecap, \
stroke-linejoin, \
stroke-opacity, \
stroke-width, \
transform \
");



const QString iwbSVGTextAttributes(" \
id, \
x, \
y, \
fill, \
font-family, \
font-size, \
font-style, \
font-weight, \
font-stretch, \
transform \
");

const QString iwbSVGTextAreaAttributes(" \
id, \
x, \
y, \
height, \
width, \
fill, \
font-family, \
font-size, \
font-style, \
font-weight, \
font-stretch, \
text-align, \
transform \
");

const QString iwbSVGTspanAttributes(" \
id, \
fill, \
font-family, \
font-size, \
font-style, \
font-weight, \
font-stretch, \
text-align, \
");

const QString iwbSVGLineAttributes(" \
id, \
x1, \
y1, \
x2, \
y2, \
stroke, \
stroke-dasharray, \
stroke-width, \
stroke-opacity, \
stroke-linecap, \
transform \
");

const QString iwbSVGPolyLineAttributes(" \
id, \
points, \
stroke, \
stroke-width, \
stroke-dasharray, \
stroke-opacity, \
stroke-linecap, \
transform \
");

const QString iwbSVGPolygonAttributes(" \
id, \
points, \
fill, \
fill-opacity, \
stroke, \
stroke-dasharray, \
stroke-width, \
stroke-linecap, \
stroke-linejoin, \
stroke-opacity, \
stroke-width, \
transform \
");

// 1 to 1 copy to SVG section
const QString iwbElementAttributes(" \
background, \
background-fill, \
background-posture, \
flip, \
freehand, \
highlight, \
highlight-fill, \
list-style-type, \
list-style-type-fill, \
locked, \
replicate, \
revealer, \
stroke-lineshape-start, \
stroke-lineshape-end \
");

// cannot be copied 1 to 1 to SVG section
const QString ubzElementAttributesToConvert(" \
xlink:href, \
src, \
transform, \
uuid \
"
);

// additional attributes. Have references in SVG section.
const QString svgElementAttributes(" \
points, \
fill, \
fill-opacity, \
stroke, \
stroke-dasharray, \
stroke-linecap, \
stroke-opacity, \
stroke-width, \
stroke_linejoin, \
requiredExtensions, \
viewbox, \
x, \
y, \
x1, \
y1, \
x2, \
y2, \
height, \
width, \
font-family, \
font-size, \
font-style, \
font-weight, \
font-stretch, \
text-align \
");

const QString ubzContentFolders("audios,videos,images,widgets");

struct UBItemLayerType
{
    enum Enum
    {
        FixedBackground = -2000, Object = -1000, Graphic = 0, Tool = 1000, Control = 2000
    };
};

#endif // UBCFFCONSTANTS_H
