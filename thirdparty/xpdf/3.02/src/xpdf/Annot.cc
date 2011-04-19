//========================================================================
//
// Annot.cc
//
// Copyright 2000-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <math.h>
#include "gmem.h"
#include "GList.h"
#include "Error.h"
#include "Object.h"
#include "Catalog.h"
#include "Gfx.h"
#include "GfxFont.h"
#include "Lexer.h"
#include "PDFDoc.h"
#include "OptionalContent.h"
#include "Annot.h"

//------------------------------------------------------------------------

#define annotFlagHidden    0x0002
#define annotFlagPrint     0x0004
#define annotFlagNoView    0x0020

#define fieldFlagReadOnly           0x00000001
#define fieldFlagRequired           0x00000002
#define fieldFlagNoExport           0x00000004
#define fieldFlagMultiline          0x00001000
#define fieldFlagPassword           0x00002000
#define fieldFlagNoToggleToOff      0x00004000
#define fieldFlagRadio              0x00008000
#define fieldFlagPushbutton         0x00010000
#define fieldFlagCombo              0x00020000
#define fieldFlagEdit               0x00040000
#define fieldFlagSort               0x00080000
#define fieldFlagFileSelect         0x00100000
#define fieldFlagMultiSelect        0x00200000
#define fieldFlagDoNotSpellCheck    0x00400000
#define fieldFlagDoNotScroll        0x00800000
#define fieldFlagComb               0x01000000
#define fieldFlagRichText           0x02000000
#define fieldFlagRadiosInUnison     0x02000000
#define fieldFlagCommitOnSelChange  0x04000000

#define fieldQuadLeft   0
#define fieldQuadCenter 1
#define fieldQuadRight  2

// distance of Bezier control point from center for circle approximation
// = (4 * (sqrt(2) - 1) / 3) * r
#define bezierCircle 0.55228475

//------------------------------------------------------------------------
// AnnotBorderStyle
//------------------------------------------------------------------------

AnnotBorderStyle::AnnotBorderStyle(AnnotBorderType typeA, double widthA,
				   double *dashA, int dashLengthA,
				   double rA, double gA, double bA) {
  type = typeA;
  width = widthA;
  dash = dashA;
  dashLength = dashLengthA;
  r = rA;
  g = gA;
  b = bA;
}

AnnotBorderStyle::~AnnotBorderStyle() {
  if (dash) {
    gfree(dash);
  }
}

//------------------------------------------------------------------------
// Annot
//------------------------------------------------------------------------

Annot::Annot(PDFDoc *docA, Dict *dict, Ref *refA) {
  Object apObj, asObj, obj1, obj2, obj3;
  AnnotBorderType borderType;
  double borderWidth;
  double *borderDash;
  int borderDashLength;
  double borderR, borderG, borderB;
  double t;
  int i;

  ok = gTrue;
  doc = docA;
  xref = doc->getXRef();
  ref = *refA;
  type = NULL;
  appearanceState = NULL;
  appearBuf = NULL;
  borderStyle = NULL;

  //----- parse the type

  if (dict->lookup("Subtype", &obj1)->isName()) {
    type = new GString(obj1.getName());
  }
  obj1.free();

  //----- parse the rectangle

  if (dict->lookup("Rect", &obj1)->isArray() &&
      obj1.arrayGetLength() == 4) {
    xMin = yMin = xMax = yMax = 0;
    if (obj1.arrayGet(0, &obj2)->isNum()) {
      xMin = obj2.getNum();
    }
    obj2.free();
    if (obj1.arrayGet(1, &obj2)->isNum()) {
      yMin = obj2.getNum();
    }
    obj2.free();
    if (obj1.arrayGet(2, &obj2)->isNum()) {
      xMax = obj2.getNum();
    }
    obj2.free();
    if (obj1.arrayGet(3, &obj2)->isNum()) {
      yMax = obj2.getNum();
    }
    obj2.free();
    if (xMin > xMax) {
      t = xMin; xMin = xMax; xMax = t;
    }
    if (yMin > yMax) {
      t = yMin; yMin = yMax; yMax = t;
    }
  } else {
    error(errSyntaxError, -1, "Bad bounding box for annotation");
    ok = gFalse;
  }
  obj1.free();

  //----- parse the flags

  if (dict->lookup("F", &obj1)->isInt()) {
    flags = obj1.getInt();
  } else {
    flags = 0;
  }
  obj1.free();

  //----- parse the border style

  borderType = annotBorderSolid;
  borderWidth = 1;
  borderDash = NULL;
  borderDashLength = 0;
  borderR = 0;
  borderG = 0;
  borderB = 1;
  if (dict->lookup("BS", &obj1)->isDict()) {
    if (obj1.dictLookup("S", &obj2)->isName()) {
      if (obj2.isName("S")) {
	borderType = annotBorderSolid;
      } else if (obj2.isName("D")) {
	borderType = annotBorderDashed;
      } else if (obj2.isName("B")) {
	borderType = annotBorderBeveled;
      } else if (obj2.isName("I")) {
	borderType = annotBorderInset;
      } else if (obj2.isName("U")) {
	borderType = annotBorderUnderlined;
      }
    }
    obj2.free();
    if (obj1.dictLookup("W", &obj2)->isNum()) {
      borderWidth = obj2.getNum();
    }
    obj2.free();
    if (obj1.dictLookup("D", &obj2)->isArray()) {
      borderDashLength = obj2.arrayGetLength();
      borderDash = (double *)gmallocn(borderDashLength, sizeof(double));
      for (i = 0; i < borderDashLength; ++i) {
	if (obj2.arrayGet(i, &obj3)->isNum()) {
	  borderDash[i] = obj3.getNum();
	} else {
	  borderDash[i] = 1;
	}
	obj3.free();
      }
    }
    obj2.free();
  } else {
    obj1.free();
    if (dict->lookup("Border", &obj1)->isArray()) {
      if (obj1.arrayGetLength() >= 3) {
	if (obj1.arrayGet(2, &obj2)->isNum()) {
	  borderWidth = obj2.getNum();
	}
	obj2.free();
	if (obj1.arrayGetLength() >= 4) {
	  if (obj1.arrayGet(3, &obj2)->isArray()) {
	    borderType = annotBorderDashed;
	    borderDashLength = obj2.arrayGetLength();
	    borderDash = (double *)gmallocn(borderDashLength, sizeof(double));
	    for (i = 0; i < borderDashLength; ++i) {
	      if (obj2.arrayGet(i, &obj3)->isNum()) {
		borderDash[i] = obj3.getNum();
	      } else {
		borderDash[i] = 1;
	      }
	      obj3.free();
	    }
	  } else {
	    // Adobe draws no border at all if the last element is of
	    // the wrong type.
	    borderWidth = 0;
	  }
	  obj2.free();
	}
      }
    }
  }
  obj1.free();
  if (dict->lookup("C", &obj1)->isArray() && obj1.arrayGetLength() == 3) {
    if (obj1.arrayGet(0, &obj2)->isNum()) {
      borderR = obj2.getNum();
    }
    obj1.free();
    if (obj1.arrayGet(1, &obj2)->isNum()) {
      borderG = obj2.getNum();
    }
    obj1.free();
    if (obj1.arrayGet(2, &obj2)->isNum()) {
      borderB = obj2.getNum();
    }
    obj1.free();
  }
  obj1.free();
  borderStyle = new AnnotBorderStyle(borderType, borderWidth,
				     borderDash, borderDashLength,
				     borderR, borderG, borderB);

  //----- get the appearance state

  dict->lookup("AP", &apObj);
  dict->lookup("AS", &asObj);
  if (asObj.isName()) {
    appearanceState = new GString(asObj.getName());
  } else if (apObj.isDict()) {
    apObj.dictLookup("N", &obj1);
    if (obj1.isDict() && obj1.dictGetLength() == 1) {
      appearanceState = new GString(obj1.dictGetKey(0));
    }
    obj1.free();
  }
  if (!appearanceState) {
    appearanceState = new GString("Off");
  }
  asObj.free();

  //----- get the annotation appearance

  if (apObj.isDict()) {
    apObj.dictLookup("N", &obj1);
    apObj.dictLookupNF("N", &obj2);
    if (obj1.isDict()) {
      if (obj1.dictLookupNF(appearanceState->getCString(), &obj3)->isRef()) {
	obj3.copy(&appearance);
      }
      obj3.free();
    } else if (obj2.isRef()) {
      obj2.copy(&appearance);
    }
    obj1.free();
    obj2.free();
  }
  apObj.free();

  //----- get the optional content entry

  dict->lookupNF("OC", &ocObj);
}

Annot::~Annot() {
  if (type) {
    delete type;
  }
  if (appearanceState) {
    delete appearanceState;
  }
  appearance.free();
  if (appearBuf) {
    delete appearBuf;
  }
  if (borderStyle) {
    delete borderStyle;
  }
  ocObj.free();
}

void Annot::generateFieldAppearance(Dict *field, Dict *annot, Dict *acroForm) {
  Object mkObj, ftObj, appearDict, drObj, obj1, obj2, obj3;
  Dict *mkDict;
  MemStream *appearStream;
  GfxFontDict *fontDict;
  GBool hasCaption;
  double w, dx, dy, r;
  double *dash;
  GString *caption, *da;
  GString **text;
  GBool *selection;
  int rot, dashLength, ff, quadding, comb, nOptions, topIdx, i, j;

  // must be a Widget annotation
  if (type->cmp("Widget")) {
    return;
  }

  appearBuf = new GString();

  // get the appearance characteristics (MK) dictionary
  if (annot->lookup("MK", &mkObj)->isDict()) {
    mkDict = mkObj.getDict();
  } else {
    mkDict = NULL;
  }

  // draw the background
  if (mkDict) {
    if (mkDict->lookup("BG", &obj1)->isArray() &&
	obj1.arrayGetLength() > 0) {
      setColor(obj1.getArray(), gTrue, 0);
      appearBuf->appendf("0 0 {0:.2f} {1:.2f} re f\n",
			 xMax - xMin, yMax - yMin);
    }
    obj1.free();
  }

  // get the field type
  fieldLookup(field, acroForm, "FT", &ftObj);

  // get the field flags (Ff) value
  if (fieldLookup(field, acroForm, "Ff", &obj1)->isInt()) {
    ff = obj1.getInt();
  } else {
    ff = 0;
  }
  obj1.free();

  // draw the border
  if (mkDict) {
    w = borderStyle->getWidth();
    if (w > 0) {
      mkDict->lookup("BC", &obj1);
      if (!(obj1.isArray() && obj1.arrayGetLength() > 0)) {
	mkDict->lookup("BG", &obj1);
      }
      if (obj1.isArray() && obj1.arrayGetLength() > 0) {
	dx = xMax - xMin;
	dy = yMax - yMin;

	// radio buttons with no caption have a round border
	hasCaption = mkDict->lookup("CA", &obj2)->isString();
	obj2.free();
	if (ftObj.isName("Btn") && (ff & fieldFlagRadio) && !hasCaption) {
	  r = 0.5 * (dx < dy ? dx : dy);
	  switch (borderStyle->getType()) {
	  case annotBorderDashed:
	    appearBuf->append("[");
	    borderStyle->getDash(&dash, &dashLength);
	    for (i = 0; i < dashLength; ++i) {
	      appearBuf->appendf(" {0:.2f}", dash[i]);
	    }
	    appearBuf->append("] 0 d\n");
	    // fall through to the solid case
	  case annotBorderSolid:
	  case annotBorderUnderlined:
	    appearBuf->appendf("{0:.2f} w\n", w);
	    setColor(obj1.getArray(), gFalse, 0);
	    drawCircle(0.5 * dx, 0.5 * dy, r - 0.5 * w, gFalse);
	    break;
	  case annotBorderBeveled:
	  case annotBorderInset:
	    appearBuf->appendf("{0:.2f} w\n", 0.5 * w);
	    setColor(obj1.getArray(), gFalse, 0);
	    drawCircle(0.5 * dx, 0.5 * dy, r - 0.25 * w, gFalse);
	    setColor(obj1.getArray(), gFalse,
		     borderStyle->getType() == annotBorderBeveled ? 1 : -1);
	    drawCircleTopLeft(0.5 * dx, 0.5 * dy, r - 0.75 * w);
	    setColor(obj1.getArray(), gFalse,
		     borderStyle->getType() == annotBorderBeveled ? -1 : 1);
	    drawCircleBottomRight(0.5 * dx, 0.5 * dy, r - 0.75 * w);
	    break;
	  }

	} else {
	  switch (borderStyle->getType()) {
	  case annotBorderDashed:
	    appearBuf->append("[");
	    borderStyle->getDash(&dash, &dashLength);
	    for (i = 0; i < dashLength; ++i) {
	      appearBuf->appendf(" {0:.2f}", dash[i]);
	    }
	    appearBuf->append("] 0 d\n");
	    // fall through to the solid case
	  case annotBorderSolid:
	    appearBuf->appendf("{0:.2f} w\n", w);
	    setColor(obj1.getArray(), gFalse, 0);
	    appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re s\n",
			       0.5 * w, dx - w, dy - w);
	    break;
	  case annotBorderBeveled:
	  case annotBorderInset:
	    setColor(obj1.getArray(), gTrue,
		     borderStyle->getType() == annotBorderBeveled ? 1 : -1);
	    appearBuf->append("0 0 m\n");
	    appearBuf->appendf("0 {0:.2f} l\n", dy);
	    appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
	    appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
	    appearBuf->appendf("{0:.2f} {1:.2f} l\n", w, dy - w);
	    appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
	    appearBuf->append("f\n");
	    setColor(obj1.getArray(), gTrue,
		     borderStyle->getType() == annotBorderBeveled ? -1 : 1);
	    appearBuf->append("0 0 m\n");
	    appearBuf->appendf("{0:.2f} 0 l\n", dx);
	    appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx, dy);
	    appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, dy - w);
	    appearBuf->appendf("{0:.2f} {1:.2f} l\n", dx - w, w);
	    appearBuf->appendf("{0:.2f} {0:.2f} l\n", w);
	    appearBuf->append("f\n");
	    break;
	  case annotBorderUnderlined:
	    appearBuf->appendf("{0:.2f} w\n", w);
	    setColor(obj1.getArray(), gFalse, 0);
	    appearBuf->appendf("0 0 m {0:.2f} 0 l s\n", dx);
	    break;
	  }

	  // clip to the inside of the border
	  appearBuf->appendf("{0:.2f} {0:.2f} {1:.2f} {2:.2f} re W n\n",
			     w, dx - 2 * w, dy - 2 * w);
	}
      }
      obj1.free();
    }
  }

  // get the resource dictionary
  fieldLookup(field, acroForm, "DR", &drObj);

  // build the font dictionary
  if (drObj.isDict() && drObj.dictLookup("Font", &obj1)->isDict()) {
    fontDict = new GfxFontDict(doc->getXRef(), NULL, obj1.getDict());
  } else {
    fontDict = NULL;
  }
  obj1.free();

  // get the default appearance string
  if (fieldLookup(field, acroForm, "DA", &obj1)->isNull()) {
    obj1.free();
    acroForm->lookup("DA", &obj1);
  }
  if (obj1.isString()) {
    da = obj1.getString()->copy();
  } else {
    da = NULL;
  }
  obj1.free();

  // get the rotation value
  rot = 0;
  if (mkDict) {
    if (mkDict->lookup("R", &obj1)->isInt()) {
      rot = obj1.getInt();
    }
    obj1.free();
  }

  // draw the field contents
  if (ftObj.isName("Btn")) {
    caption = NULL;
    if (mkDict) {
      if (mkDict->lookup("CA", &obj1)->isString()) {
	caption = obj1.getString()->copy();
      }
      obj1.free();
    }
    // radio button
    if (ff & fieldFlagRadio) {
      //~ Acrobat doesn't draw a caption if there is no AP dict (?)
      if (fieldLookup(field, acroForm, "V", &obj1)
	    ->isName(appearanceState->getCString())) {
	if (caption) {
	  drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
		   gFalse, gTrue, rot);
	} else {
	  if (mkDict) {
	    if (mkDict->lookup("BC", &obj2)->isArray() &&
		obj2.arrayGetLength() > 0) {
	      dx = xMax - xMin;
	      dy = yMax - yMin;
	      setColor(obj2.getArray(), gTrue, 0);
	      drawCircle(0.5 * dx, 0.5 * dy, 0.2 * (dx < dy ? dx : dy),
			 gTrue);
	    }
	    obj2.free();
	  }
	}
      }
      obj1.free();
    // pushbutton
    } else if (ff & fieldFlagPushbutton) {
      if (caption) {
	drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
		 gFalse, gFalse, rot);
      }
    // checkbox
    } else {
      fieldLookup(field, acroForm, "V", &obj1);
      if (obj1.isName() && !obj1.isName("Off")) {
	if (!caption) {
	  caption = new GString("3"); // ZapfDingbats checkmark
	}
	drawText(caption, da, fontDict, gFalse, 0, fieldQuadCenter,
		 gFalse, gTrue, rot);
      }
      obj1.free();
    }
    if (caption) {
      delete caption;
    }
  } else if (ftObj.isName("Tx")) {
    //~ value strings can be Unicode
    if (!fieldLookup(field, acroForm, "V", &obj1)->isString()) {
      obj1.free();
      fieldLookup(field, acroForm, "DV", &obj1);
    }
    if (obj1.isString()) {
      if (fieldLookup(field, acroForm, "Q", &obj2)->isInt()) {
	quadding = obj2.getInt();
      } else {
	quadding = fieldQuadLeft;
      }
      obj2.free();
      comb = 0;
      if (ff & fieldFlagComb) {
	if (fieldLookup(field, acroForm, "MaxLen", &obj2)->isInt()) {
	  comb = obj2.getInt();
	}
	obj2.free();
      }
      drawText(obj1.getString(), da, fontDict,
	       ff & fieldFlagMultiline, comb, quadding, gTrue, gFalse, rot);
    }
    obj1.free();
  } else if (ftObj.isName("Ch")) {
    //~ value/option strings can be Unicode
    if (fieldLookup(field, acroForm, "Q", &obj1)->isInt()) {
      quadding = obj1.getInt();
    } else {
      quadding = fieldQuadLeft;
    }
    obj1.free();
    // combo box
    if (ff & fieldFlagCombo) {
      if (fieldLookup(field, acroForm, "V", &obj1)->isString()) {
	drawText(obj1.getString(), da, fontDict,
		 gFalse, 0, quadding, gTrue, gFalse, rot);
	//~ Acrobat draws a popup icon on the right side
      }
      obj1.free();
    // list box
    } else {
      if (field->lookup("Opt", &obj1)->isArray()) {
	nOptions = obj1.arrayGetLength();
	// get the option text
	text = (GString **)gmallocn(nOptions, sizeof(GString *));
	for (i = 0; i < nOptions; ++i) {
	  text[i] = NULL;
	  obj1.arrayGet(i, &obj2);
	  if (obj2.isString()) {
	    text[i] = obj2.getString()->copy();
	  } else if (obj2.isArray() && obj2.arrayGetLength() == 2) {
	    if (obj2.arrayGet(1, &obj3)->isString()) {
	      text[i] = obj3.getString()->copy();
	    }
	    obj3.free();
	  }
	  obj2.free();
	  if (!text[i]) {
	    text[i] = new GString();
	  }
	}
	// get the selected option(s)
	selection = (GBool *)gmallocn(nOptions, sizeof(GBool));
	//~ need to use the I field in addition to the V field
	fieldLookup(field, acroForm, "V", &obj2);
	for (i = 0; i < nOptions; ++i) {
	  selection[i] = gFalse;
	  if (obj2.isString()) {
	    if (!obj2.getString()->cmp(text[i])) {
	      selection[i] = gTrue;
	    }
	  } else if (obj2.isArray()) {
	    for (j = 0; j < obj2.arrayGetLength(); ++j) {
	      if (obj2.arrayGet(j, &obj3)->isString() &&
		  !obj3.getString()->cmp(text[i])) {
		selection[i] = gTrue;
	      }
	      obj3.free();
	    }
	  }
	}
	obj2.free();
	// get the top index
	if (field->lookup("TI", &obj2)->isInt()) {
	  topIdx = obj2.getInt();
	} else {
	  topIdx = 0;
	}
	obj2.free();
	// draw the text
	drawListBox(text, selection, nOptions, topIdx, da, fontDict, quadding);
	for (i = 0; i < nOptions; ++i) {
	  delete text[i];
	}
	gfree(text);
	gfree(selection);
      }
      obj1.free();
    }
  } else if (ftObj.isName("Sig")) {
    //~unimp
  } else {
    error(errSyntaxError, -1, "Unknown field type");
  }

  if (da) {
    delete da;
  }

  // build the appearance stream dictionary
  appearDict.initDict(doc->getXRef());
  appearDict.dictAdd(copyString("Length"),
		     obj1.initInt(appearBuf->getLength()));
  appearDict.dictAdd(copyString("Subtype"), obj1.initName("Form"));
  obj1.initArray(doc->getXRef());
  obj1.arrayAdd(obj2.initReal(0));
  obj1.arrayAdd(obj2.initReal(0));
  obj1.arrayAdd(obj2.initReal(xMax - xMin));
  obj1.arrayAdd(obj2.initReal(yMax - yMin));
  appearDict.dictAdd(copyString("BBox"), &obj1);

  // set the resource dictionary
  if (drObj.isDict()) {
    appearDict.dictAdd(copyString("Resources"), drObj.copy(&obj1));
  }
  drObj.free();

  // build the appearance stream
  appearStream = new MemStream(appearBuf->getCString(), 0,
			       appearBuf->getLength(), &appearDict);
  appearance.free();
  appearance.initStream(appearStream);

  if (fontDict) {
    delete fontDict;
  }
  ftObj.free();
  mkObj.free();
}

// Set the current fill or stroke color, based on <a> (which should
// have 1, 3, or 4 elements).  If <adjust> is +1, color is brightened;
// if <adjust> is -1, color is darkened; otherwise color is not
// modified.
void Annot::setColor(Array *a, GBool fill, int adjust) {
  Object obj1;
  double color[4];
  int nComps, i;

  nComps = a->getLength();
  if (nComps > 4) {
    nComps = 4;
  }
  for (i = 0; i < nComps && i < 4; ++i) {
    if (a->get(i, &obj1)->isNum()) {
      color[i] = obj1.getNum();
    } else {
      color[i] = 0;
    }
    obj1.free();
  }
  if (nComps == 4) {
    adjust = -adjust;
  }
  if (adjust > 0) {
    for (i = 0; i < nComps; ++i) {
      color[i] = 0.5 * color[i] + 0.5;
    }
  } else if (adjust < 0) {
    for (i = 0; i < nComps; ++i) {
      color[i] = 0.5 * color[i];
    }
  }
  if (nComps == 4) {
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:c}\n",
		       color[0], color[1], color[2], color[3],
		       fill ? 'k' : 'K');
  } else if (nComps == 3) {
    appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:s}\n",
		       color[0], color[1], color[2],
		       fill ? "rg" : "RG");
  } else {
    appearBuf->appendf("{0:.2f} {1:c}\n",
		       color[0],
		       fill ? 'g' : 'G');
  }
}

// Draw the variable text or caption for a field.
void Annot::drawText(GString *text, GString *da, GfxFontDict *fontDict,
		     GBool multiline, int comb, int quadding,
		     GBool txField, GBool forceZapfDingbats, int rot) {
  GString *text2;
  GList *daToks;
  GString *tok;
  GfxFont *font;
  double dx, dy;
  double fontSize, fontSize2, border, x, xPrev, y, w, w2, wMax;
  int tfPos, tmPos, i, j, k, c;

  //~ if there is no MK entry, this should use the existing content stream,
  //~ and only replace the marked content portion of it
  //~ (this is only relevant for Tx fields)

  // check for a Unicode string
  //~ this currently drops all non-Latin1 characters
  if (text->getLength() >= 2 &&
      text->getChar(0) == '\xfe' && text->getChar(1) == '\xff') {
    text2 = new GString();
    for (i = 2; i+1 < text->getLength(); i += 2) {
      c = ((text->getChar(i) & 0xff) << 8) + (text->getChar(i+1) & 0xff);
      if (c <= 0xff) {
	text2->append((char)c);
      } else {
	text2->append('?');
      }
    }
  } else {
    text2 = text;
  }

  // parse the default appearance string
  tfPos = tmPos = -1;
  if (da) {
    daToks = new GList();
    i = 0;
    while (i < da->getLength()) {
      while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
	++i;
      }
      if (i < da->getLength()) {
	for (j = i + 1;
	     j < da->getLength() && !Lexer::isSpace(da->getChar(j));
	     ++j) ;
	daToks->append(new GString(da, i, j - i));
	i = j;
      }
    }
    for (i = 2; i < daToks->getLength(); ++i) {
      if (i >= 2 && !((GString *)daToks->get(i))->cmp("Tf")) {
	tfPos = i - 2;
      } else if (i >= 6 && !((GString *)daToks->get(i))->cmp("Tm")) {
	tmPos = i - 6;
      }
    }
  } else {
    daToks = NULL;
  }

  // force ZapfDingbats
  //~ this should create the font if needed (?)
  if (forceZapfDingbats) {
    if (tfPos >= 0) {
      tok = (GString *)daToks->get(tfPos);
      if (tok->cmp("/ZaDb")) {
	tok->clear();
	tok->append("/ZaDb");
      }
    }
  }

  // get the font and font size
  font = NULL;
  fontSize = 0;
  if (tfPos >= 0) {
    tok = (GString *)daToks->get(tfPos);
    if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
      if (!fontDict || !(font = fontDict->lookup(tok->getCString() + 1))) {
	error(errSyntaxError, -1, "Unknown font in field's DA string");
      }
    } else {
      error(errSyntaxError, -1,
	    "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GString *)daToks->get(tfPos + 1);
    fontSize = atof(tok->getCString());
  } else {
    error(errSyntaxError, -1, "Missing 'Tf' operator in field's DA string");
  }

  // get the border width
  border = borderStyle->getWidth();

  // setup
  if (txField) {
    appearBuf->append("/Tx BMC\n");
  }
  appearBuf->append("q\n");
  if (rot == 90) {
    appearBuf->appendf("0 1 -1 0 {0:.2f} 0 cm\n", xMax - xMin);
    dx = yMax - yMin;
    dy = xMax - xMin;
  } else if (rot == 180) {
    appearBuf->appendf("-1 0 0 -1 {0:.2f} {1:.2f} cm\n",
		       xMax - xMin, yMax - yMin);
    dx = xMax - yMax;
    dy = yMax - yMin;
  } else if (rot == 270) {
    appearBuf->appendf("0 -1 1 0 0 {0:.2f} cm\n", yMax - yMin);
    dx = yMax - yMin;
    dy = xMax - xMin;
  } else { // assume rot == 0
    dx = xMax - xMin;
    dy = yMax - yMin;
  }
  appearBuf->append("BT\n");

  // multi-line text
  if (multiline) {
    // note: the comb flag is ignored in multiline mode

    wMax = dx - 2 * border - 4;

    // compute font autosize
    if (fontSize == 0) {
      for (fontSize = 20; fontSize > 1; --fontSize) {
	y = dy - 3;
	w2 = 0;
	i = 0;
	while (i < text2->getLength()) {
	  getNextLine(text2, i, font, fontSize, wMax, &j, &w, &k);
	  if (w > w2) {
	    w2 = w;
	  }
	  i = k;
	  y -= fontSize;
	}
	// approximate the descender for the last line
	if (y >= 0.33 * fontSize) {
	  break;
	}
      }
      if (tfPos >= 0) {
	tok = (GString *)daToks->get(tfPos + 1);
	tok->clear();
	tok->appendf("{0:.2f}", fontSize);
      }
    }

    // starting y coordinate
    // (note: each line of text starts with a Td operator that moves
    // down a line)
    y = dy - 3;

    // set the font matrix
    if (tmPos >= 0) {
      tok = (GString *)daToks->get(tmPos + 4);
      tok->clear();
      tok->append('0');
      tok = (GString *)daToks->get(tmPos + 5);
      tok->clear();
      tok->appendf("{0:.2f}", y);
    }

    // write the DA string
    if (daToks) {
      for (i = 0; i < daToks->getLength(); ++i) {
	appearBuf->append((GString *)daToks->get(i))->append(' ');
      }
    }

    // write the font matrix (if not part of the DA string)
    if (tmPos < 0) {
      appearBuf->appendf("1 0 0 1 0 {0:.2f} Tm\n", y);
    }

    // write a series of lines of text
    i = 0;
    xPrev = 0;
    while (i < text2->getLength()) {

      getNextLine(text2, i, font, fontSize, wMax, &j, &w, &k);

      // compute text start position
      switch (quadding) {
      case fieldQuadLeft:
      default:
	x = border + 2;
	break;
      case fieldQuadCenter:
	x = (dx - w) / 2;
	break;
      case fieldQuadRight:
	x = dx - border - 2 - w;
	break;
      }

      // draw the line
      appearBuf->appendf("{0:.2f} {1:.2f} Td\n", x - xPrev, -fontSize);
      appearBuf->append('(');
      for (; i < j; ++i) {
	c = text2->getChar(i) & 0xff;
	if (c == '(' || c == ')' || c == '\\') {
	  appearBuf->append('\\');
	  appearBuf->append(c);
	} else if (c < 0x20 || c >= 0x80) {
	  appearBuf->appendf("\\{0:03o}", c);
	} else {
	  appearBuf->append(c);
	}
      }
      appearBuf->append(") Tj\n");

      // next line
      i = k;
      xPrev = x;
    }

  // single-line text
  } else {
    //~ replace newlines with spaces? - what does Acrobat do?

    // comb formatting
    if (comb > 0) {

      // compute comb spacing
      w = (dx - 2 * border) / comb;

      // compute font autosize
      if (fontSize == 0) {
	fontSize = dy - 2 * border;
	if (w < fontSize) {
	  fontSize = w;
	}
	fontSize = floor(fontSize);
	if (tfPos >= 0) {
	  tok = (GString *)daToks->get(tfPos + 1);
	  tok->clear();
	  tok->appendf("{0:.2f}", fontSize);
	}
      }

      // compute text start position
      switch (quadding) {
      case fieldQuadLeft:
      default:
	x = border + 2;
	break;
      case fieldQuadCenter:
	x = border + 2 + 0.5 * (comb - text2->getLength()) * w;
	break;
      case fieldQuadRight:
	x = border + 2 + (comb - text2->getLength()) * w;
	break;
      }
      y = 0.5 * dy - 0.4 * fontSize;

      // set the font matrix
      if (tmPos >= 0) {
	tok = (GString *)daToks->get(tmPos + 4);
	tok->clear();
	tok->appendf("{0:.2f}", x);
	tok = (GString *)daToks->get(tmPos + 5);
	tok->clear();
	tok->appendf("{0:.2f}", y);
      }

      // write the DA string
      if (daToks) {
	for (i = 0; i < daToks->getLength(); ++i) {
	  appearBuf->append((GString *)daToks->get(i))->append(' ');
	}
      }

      // write the font matrix (if not part of the DA string)
      if (tmPos < 0) {
	appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
      }

      // write the text string
      //~ this should center (instead of left-justify) each character within
      //~     its comb cell
      for (i = 0; i < text2->getLength(); ++i) {
	if (i > 0) {
	  appearBuf->appendf("{0:.2f} 0 Td\n", w);
	}
	appearBuf->append('(');
	c = text2->getChar(i) & 0xff;
	if (c == '(' || c == ')' || c == '\\') {
	  appearBuf->append('\\');
	  appearBuf->append(c);
	} else if (c < 0x20 || c >= 0x80) {
	  appearBuf->appendf("{0:.2f} 0 Td\n", w);
	} else {
	  appearBuf->append(c);
	}
	appearBuf->append(") Tj\n");
      }

    // regular (non-comb) formatting
    } else {

      // compute string width
      if (font && !font->isCIDFont()) {
	w = 0;
	for (i = 0; i < text2->getLength(); ++i) {
	  w += ((Gfx8BitFont *)font)->getWidth(text2->getChar(i));
	}
      } else {
	// otherwise, make a crude estimate
	w = text2->getLength() * 0.5;
      }

      // compute font autosize
      if (fontSize == 0) {
	fontSize = dy - 2 * border;
	fontSize2 = (dx - 4 - 2 * border) / w;
	if (fontSize2 < fontSize) {
	  fontSize = fontSize2;
	}
	fontSize = floor(fontSize);
	if (tfPos >= 0) {
	  tok = (GString *)daToks->get(tfPos + 1);
	  tok->clear();
	  tok->appendf("{0:.2f}", fontSize);
	}
      }

      // compute text start position
      w *= fontSize;
      switch (quadding) {
      case fieldQuadLeft:
      default:
	x = border + 2;
	break;
      case fieldQuadCenter:
	x = (dx - w) / 2;
	break;
      case fieldQuadRight:
	x = dx - border - 2 - w;
	break;
      }
      y = 0.5 * dy - 0.4 * fontSize;

      // set the font matrix
      if (tmPos >= 0) {
	tok = (GString *)daToks->get(tmPos + 4);
	tok->clear();
	tok->appendf("{0:.2f}", x);
	tok = (GString *)daToks->get(tmPos + 5);
	tok->clear();
	tok->appendf("{0:.2f}", y);
      }

      // write the DA string
      if (daToks) {
	for (i = 0; i < daToks->getLength(); ++i) {
	  appearBuf->append((GString *)daToks->get(i))->append(' ');
	}
      }

      // write the font matrix (if not part of the DA string)
      if (tmPos < 0) {
	appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
      }

      // write the text string
      appearBuf->append('(');
      for (i = 0; i < text2->getLength(); ++i) {
	c = text2->getChar(i) & 0xff;
	if (c == '(' || c == ')' || c == '\\') {
	  appearBuf->append('\\');
	  appearBuf->append(c);
	} else if (c < 0x20 || c >= 0x80) {
	  appearBuf->appendf("\\{0:03o}", c);
	} else {
	  appearBuf->append(c);
	}
      }
      appearBuf->append(") Tj\n");
    }
  }

  // cleanup
  appearBuf->append("ET\n");
  appearBuf->append("Q\n");
  if (txField) {
    appearBuf->append("EMC\n");
  }

  if (daToks) {
    deleteGList(daToks, GString);
  }
  if (text2 != text) {
    delete text2;
  }
}

// Draw the variable text or caption for a field.
void Annot::drawListBox(GString **text, GBool *selection,
			int nOptions, int topIdx,
			GString *da, GfxFontDict *fontDict, GBool quadding) {
  GList *daToks;
  GString *tok;
  GfxFont *font;
  double fontSize, fontSize2, border, x, y, w, wMax;
  int tfPos, tmPos, i, j, c;

  //~ if there is no MK entry, this should use the existing content stream,
  //~ and only replace the marked content portion of it
  //~ (this is only relevant for Tx fields)

  // parse the default appearance string
  tfPos = tmPos = -1;
  if (da) {
    daToks = new GList();
    i = 0;
    while (i < da->getLength()) {
      while (i < da->getLength() && Lexer::isSpace(da->getChar(i))) {
	++i;
      }
      if (i < da->getLength()) {
	for (j = i + 1;
	     j < da->getLength() && !Lexer::isSpace(da->getChar(j));
	     ++j) ;
	daToks->append(new GString(da, i, j - i));
	i = j;
      }
    }
    for (i = 2; i < daToks->getLength(); ++i) {
      if (i >= 2 && !((GString *)daToks->get(i))->cmp("Tf")) {
	tfPos = i - 2;
      } else if (i >= 6 && !((GString *)daToks->get(i))->cmp("Tm")) {
	tmPos = i - 6;
      }
    }
  } else {
    daToks = NULL;
  }

  // get the font and font size
  font = NULL;
  fontSize = 0;
  if (tfPos >= 0) {
    tok = (GString *)daToks->get(tfPos);
    if (tok->getLength() >= 1 && tok->getChar(0) == '/') {
      if (!fontDict || !(font = fontDict->lookup(tok->getCString() + 1))) {
	error(errSyntaxError, -1, "Unknown font in field's DA string");
      }
    } else {
      error(errSyntaxError, -1,
	    "Invalid font name in 'Tf' operator in field's DA string");
    }
    tok = (GString *)daToks->get(tfPos + 1);
    fontSize = atof(tok->getCString());
  } else {
    error(errSyntaxError, -1, "Missing 'Tf' operator in field's DA string");
  }

  // get the border width
  border = borderStyle->getWidth();

  // compute font autosize
  if (fontSize == 0) {
    wMax = 0;
    for (i = 0; i < nOptions; ++i) {
      if (font && !font->isCIDFont()) {
	w = 0;
	for (j = 0; j < text[i]->getLength(); ++j) {
	  w += ((Gfx8BitFont *)font)->getWidth(text[i]->getChar(j));
	}
      } else {
	// otherwise, make a crude estimate
	w = text[i]->getLength() * 0.5;
      }
      if (w > wMax) {
	wMax = w;
      }
    }
    fontSize = yMax - yMin - 2 * border;
    fontSize2 = (xMax - xMin - 4 - 2 * border) / wMax;
    if (fontSize2 < fontSize) {
      fontSize = fontSize2;
    }
    fontSize = floor(fontSize);
    if (tfPos >= 0) {
      tok = (GString *)daToks->get(tfPos + 1);
      tok->clear();
      tok->appendf("{0:.2f}", fontSize);
    }
  }

  // draw the text
  y = yMax - yMin - 1.1 * fontSize;
  for (i = topIdx; i < nOptions; ++i) {

    // setup
    appearBuf->append("q\n");

    // draw the background if selected
    if (selection[i]) {
      appearBuf->append("0 g f\n");
      appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} re f\n",
	      border,
	      y - 0.2 * fontSize,
	      xMax - xMin - 2 * border,
	      1.1 * fontSize);
    }

    // setup
    appearBuf->append("BT\n");

    // compute string width
    if (font && !font->isCIDFont()) {
      w = 0;
      for (j = 0; j < text[i]->getLength(); ++j) {
	w += ((Gfx8BitFont *)font)->getWidth(text[i]->getChar(j));
      }
    } else {
      // otherwise, make a crude estimate
      w = text[i]->getLength() * 0.5;
    }

    // compute text start position
    w *= fontSize;
    switch (quadding) {
    case fieldQuadLeft:
    default:
      x = border + 2;
      break;
    case fieldQuadCenter:
      x = (xMax - xMin - w) / 2;
      break;
    case fieldQuadRight:
      x = xMax - xMin - border - 2 - w;
      break;
    }

    // set the font matrix
    if (tmPos >= 0) {
      tok = (GString *)daToks->get(tmPos + 4);
      tok->clear();
      tok->appendf("{0:.2f}", x);
      tok = (GString *)daToks->get(tmPos + 5);
      tok->clear();
      tok->appendf("{0:.2f}", y);
    }

    // write the DA string
    if (daToks) {
      for (j = 0; j < daToks->getLength(); ++j) {
	appearBuf->append((GString *)daToks->get(j))->append(' ');
      }
    }

    // write the font matrix (if not part of the DA string)
    if (tmPos < 0) {
      appearBuf->appendf("1 0 0 1 {0:.2f} {1:.2f} Tm\n", x, y);
    }

    // change the text color if selected
    if (selection[i]) {
      appearBuf->append("1 g\n");
    }

    // write the text string
    appearBuf->append('(');
    for (j = 0; j < text[i]->getLength(); ++j) {
      c = text[i]->getChar(j) & 0xff;
      if (c == '(' || c == ')' || c == '\\') {
	appearBuf->append('\\');
	appearBuf->append(c);
      } else if (c < 0x20 || c >= 0x80) {
	appearBuf->appendf("\\{0:03o}", c);
      } else {
	appearBuf->append(c);
      }
    }
    appearBuf->append(") Tj\n");

    // cleanup
    appearBuf->append("ET\n");
    appearBuf->append("Q\n");

    // next line
    y -= 1.1 * fontSize;
  }

  if (daToks) {
    deleteGList(daToks, GString);
  }
}

// Figure out how much text will fit on the next line.  Returns:
// *end = one past the last character to be included
// *width = width of the characters start .. end-1
// *next = index of first character on the following line
void Annot::getNextLine(GString *text, int start,
			GfxFont *font, double fontSize, double wMax,
			int *end, double *width, int *next) {
  double w, dw;
  int j, k, c;

  // figure out how much text will fit on the line
  //~ what does Adobe do with tabs?
  w = 0;
  for (j = start; j < text->getLength() && w <= wMax; ++j) {
    c = text->getChar(j) & 0xff;
    if (c == 0x0a || c == 0x0d) {
      break;
    }
    if (font && !font->isCIDFont()) {
      dw = ((Gfx8BitFont *)font)->getWidth(c) * fontSize;
    } else {
      // otherwise, make a crude estimate
      dw = 0.5 * fontSize;
    }
    w += dw;
  }
  if (w > wMax) {
    for (k = j; k > start && text->getChar(k-1) != ' '; --k) ;
    for (; k > start && text->getChar(k-1) == ' '; --k) ;
    if (k > start) {
      j = k;
    }
    if (j == start) {
      // handle the pathological case where the first character is
      // too wide to fit on the line all by itself
      j = start + 1;
    }
  }
  *end = j;

  // compute the width
  w = 0;
  for (k = start; k < j; ++k) {
    if (font && !font->isCIDFont()) {
      dw = ((Gfx8BitFont *)font)->getWidth(text->getChar(k)) * fontSize;
    } else {
      // otherwise, make a crude estimate
      dw = 0.5 * fontSize;
    }
    w += dw;
  }
  *width = w;

  // next line
  while (j < text->getLength() && text->getChar(j) == ' ') {
    ++j;
  }
  if (j < text->getLength() && text->getChar(j) == 0x0d) {
    ++j;
  }
  if (j < text->getLength() && text->getChar(j) == 0x0a) {
    ++j;
  }
  *next = j;
}

// Draw an (approximate) circle of radius <r> centered at (<cx>, <cy>).
// If <fill> is true, the circle is filled; otherwise it is stroked.
void Annot::drawCircle(double cx, double cy, double r, GBool fill) {
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
		     cx + r, cy);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
		     cx + r, cy + bezierCircle * r,
		     cx + bezierCircle * r, cy + r,
		     cx, cy + r);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
		     cx - bezierCircle * r, cy + r,
		     cx - r, cy + bezierCircle * r,
		     cx - r, cy);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
		     cx - r, cy - bezierCircle * r,
		     cx - bezierCircle * r, cy - r,
		     cx, cy - r);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
		     cx + bezierCircle * r, cy - r,
		     cx + r, cy - bezierCircle * r,
		     cx + r, cy);
  appearBuf->append(fill ? "f\n" : "s\n");
}

// Draw the top-left half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void Annot::drawCircleTopLeft(double cx, double cy, double r) {
  double r2;

  r2 = r / sqrt(2.0);
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
		     cx + r2, cy + r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
		     cx + (1 - bezierCircle) * r2,
		     cy + (1 + bezierCircle) * r2,
		     cx - (1 - bezierCircle) * r2,
		     cy + (1 + bezierCircle) * r2,
		     cx - r2,
		     cy + r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
		     cx - (1 + bezierCircle) * r2,
		     cy + (1 - bezierCircle) * r2,
		     cx - (1 + bezierCircle) * r2,
		     cy - (1 - bezierCircle) * r2,
		     cx - r2,
		     cy - r2);
  appearBuf->append("S\n");
}

// Draw the bottom-right half of an (approximate) circle of radius <r>
// centered at (<cx>, <cy>).
void Annot::drawCircleBottomRight(double cx, double cy, double r) {
  double r2;

  r2 = r / sqrt(2.0);
  appearBuf->appendf("{0:.2f} {1:.2f} m\n",
		     cx - r2, cy - r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
		     cx - (1 - bezierCircle) * r2,
		     cy - (1 + bezierCircle) * r2,
		     cx + (1 - bezierCircle) * r2,
		     cy - (1 + bezierCircle) * r2,
		     cx + r2,
		     cy - r2);
  appearBuf->appendf("{0:.2f} {1:.2f} {2:.2f} {3:.2f} {4:.2f} {5:.2f} c\n",
		     cx + (1 + bezierCircle) * r2,
		     cy - (1 - bezierCircle) * r2,
		     cx + (1 + bezierCircle) * r2,
		     cy + (1 - bezierCircle) * r2,
		     cx + r2,
		     cy + r2);
  appearBuf->append("S\n");
}

// Look up an inheritable field dictionary entry.
Object *Annot::fieldLookup(Dict *field, Dict *acroForm,
			   char *key, Object *obj) {
  Dict *dict;
  Object parent;

  dict = field;
  if (!dict->lookup(key, obj)->isNull()) {
    return obj;
  }
  obj->free();
  if (dict->lookup("Parent", &parent)->isDict()) {
    fieldLookup(parent.getDict(), acroForm, key, obj);
  } else if (acroForm) {
    // some fields don't specify a parent, so we check the AcroForm
    // dictionary just in case
    fieldLookup(acroForm, NULL, key, obj);
  } else {
    obj->initNull();
  }
  parent.free();
  return obj;
}

void Annot::draw(Gfx *gfx, GBool printing) {
  Object obj;
  GBool oc, isLink;

  // check the flags
  if ((flags & annotFlagHidden) ||
      (printing && !(flags & annotFlagPrint)) ||
      (!printing && (flags & annotFlagNoView))) {
    return;
  }

  // check the optional content entry
  if (doc->getOptionalContent()->evalOCObject(&ocObj, &oc) && !oc) {
    return;
  }

  // draw the appearance stream
  isLink = type && !type->cmp("Link");
  appearance.fetch(doc->getXRef(), &obj);
  gfx->drawAnnot(&obj, isLink ? borderStyle : (AnnotBorderStyle *)NULL,
		 xMin, yMin, xMax, yMax);
  obj.free();
}

//------------------------------------------------------------------------
// Annots
//------------------------------------------------------------------------

Annots::Annots(PDFDoc *docA, Object *annotsObj) {
  Annot *annot;
  Object obj1;
  Ref ref;
  int size;
  int i;

  doc = docA;
  annots = NULL;
  size = 0;
  nAnnots = 0;

  if (annotsObj->isArray()) {
    for (i = 0; i < annotsObj->arrayGetLength(); ++i) {
      if (annotsObj->arrayGetNF(i, &obj1)->isRef()) {
	ref = obj1.getRef();
	obj1.free();
	annotsObj->arrayGet(i, &obj1);
      } else {
	ref.num = ref.gen = -1;
      }
      if (obj1.isDict()) {
	annot = new Annot(doc, obj1.getDict(), &ref);
	if (annot->isOk()) {
	  if (nAnnots >= size) {
	    size += 16;
	    annots = (Annot **)greallocn(annots, size, sizeof(Annot *));
	  }
	  annots[nAnnots++] = annot;
	} else {
	  delete annot;
	}
      }
      obj1.free();
    }
  }
}

Annots::~Annots() {
  int i;

  for (i = 0; i < nAnnots; ++i) {
    delete annots[i];
  }
  gfree(annots);
}

void Annots::generateAppearances() {
  Dict *acroForm;
  Object obj1, obj2;
  Ref ref;
  int i;

  acroForm = doc->getCatalog()->getAcroForm()->isDict() ?
               doc->getCatalog()->getAcroForm()->getDict() : NULL;
  if (acroForm->lookup("Fields", &obj1)->isArray()) {
    for (i = 0; i < obj1.arrayGetLength(); ++i) {
      if (obj1.arrayGetNF(i, &obj2)->isRef()) {
	ref = obj2.getRef();
	obj2.free();
	obj1.arrayGet(i, &obj2);
      } else {
	ref.num = ref.gen = -1;
      }
      if (obj2.isDict()) {
	scanFieldAppearances(obj2.getDict(), &ref, NULL, acroForm);
      }
      obj2.free();
    }
  }
  obj1.free();
}

void Annots::scanFieldAppearances(Dict *node, Ref *ref, Dict *parent,
				  Dict *acroForm) {
  Annot *annot;
  Object obj1, obj2;
  Ref ref2;
  int i;

  // non-terminal node: scan the children
  if (node->lookup("Kids", &obj1)->isArray()) {
    for (i = 0; i < obj1.arrayGetLength(); ++i) {
      if (obj1.arrayGetNF(i, &obj2)->isRef()) {
	ref2 = obj2.getRef();
	obj2.free();
	obj1.arrayGet(i, &obj2);
      } else {
	ref2.num = ref2.gen = -1;
      }
      if (obj2.isDict()) {
	scanFieldAppearances(obj2.getDict(), &ref2, node, acroForm);
      }
      obj2.free();
    }
    obj1.free();
    return;
  }
  obj1.free();

  // terminal node: this is either a combined annot/field dict, or an
  // annot dict whose parent is a field
  if ((annot = findAnnot(ref))) {
    node->lookupNF("Parent", &obj1);
    if (!parent || !obj1.isNull()) {
      annot->generateFieldAppearance(node, node, acroForm);
    } else {
      annot->generateFieldAppearance(parent, node, acroForm);
    }
    obj1.free();
  }
}

Annot *Annots::findAnnot(Ref *ref) {
  int i;

  for (i = 0; i < nAnnots; ++i) {
    if (annots[i]->match(ref)) {
      return annots[i];
    }
  }
  return NULL;
}
