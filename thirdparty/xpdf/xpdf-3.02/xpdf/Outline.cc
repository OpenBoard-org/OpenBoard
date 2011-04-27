//========================================================================
//
// Outline.cc
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "gmem.h"
#include "GString.h"
#include "GList.h"
#include "Link.h"
#include "PDFDocEncoding.h"
#include "Outline.h"

//------------------------------------------------------------------------

Outline::Outline(Object *outlineObj, XRef *xref) {
  Object first, last;

  items = NULL;
  if (!outlineObj->isDict()) {
    return;
  }
  items = OutlineItem::readItemList(outlineObj->dictLookupNF("First", &first),
				    outlineObj->dictLookupNF("Last", &last),
				    xref);
  first.free();
  last.free();
}

Outline::~Outline() {
  if (items) {
    deleteGList(items, OutlineItem);
  }
}

//------------------------------------------------------------------------

OutlineItem::OutlineItem(Dict *dict, XRef *xrefA) {
  Object obj1;
  GString *s;
  int i;

  xref = xrefA;
  title = NULL;
  action = NULL;
  kids = NULL;

  if (dict->lookup("Title", &obj1)->isString()) {
    s = obj1.getString();
    if ((s->getChar(0) & 0xff) == 0xfe &&
	(s->getChar(1) & 0xff) == 0xff) {
      titleLen = (s->getLength() - 2) / 2;
      title = (Unicode *)gmallocn(titleLen, sizeof(Unicode));
      for (i = 0; i < titleLen; ++i) {
	title[i] = ((s->getChar(2 + 2*i) & 0xff) << 8) |
	           (s->getChar(3 + 2*i) & 0xff);
      }
    } else {
      titleLen = s->getLength();
      title = (Unicode *)gmallocn(titleLen, sizeof(Unicode));
      for (i = 0; i < titleLen; ++i) {
	title[i] = pdfDocEncoding[s->getChar(i) & 0xff];
      }
    }
  } else {
    titleLen = 0;
  }
  obj1.free();

  if (!dict->lookup("Dest", &obj1)->isNull()) {
    action = LinkAction::parseDest(&obj1);
  } else {
    obj1.free();
    if (!dict->lookup("A", &obj1)->isNull()) {
      action = LinkAction::parseAction(&obj1);
    }
  }
  obj1.free();

  dict->lookupNF("First", &firstRef);
  dict->lookupNF("Last", &lastRef);
  dict->lookupNF("Next", &nextRef);

  startsOpen = gFalse;
  if (dict->lookup("Count", &obj1)->isInt()) {
    if (obj1.getInt() > 0) {
      startsOpen = gTrue;
    }
  }
  obj1.free();
}

OutlineItem::~OutlineItem() {
  close();
  if (title) {
    gfree(title);
  }
  if (action) {
    delete action;
  }
  firstRef.free();
  lastRef.free();
  nextRef.free();
}

GList *OutlineItem::readItemList(Object *firstItemRef, Object *lastItemRef,
				 XRef *xrefA) {
  GList *items;
  OutlineItem *item;
  Object obj;
  Object *p;

  items = new GList();
  p = firstItemRef;
  while (p->isRef()) {
    if (!p->fetch(xrefA, &obj)->isDict()) {
      obj.free();
      break;
    }
    item = new OutlineItem(obj.getDict(), xrefA);
    obj.free();
    items->append(item);
    if (p->getRef().num == lastItemRef->getRef().num &&
	p->getRef().gen == lastItemRef->getRef().gen) {
      break;
    }
    p = &item->nextRef;
  }
  return items;
}

void OutlineItem::open() {
  if (!kids) {
    kids = readItemList(&firstRef, &lastRef, xref);
  }
}

void OutlineItem::close() {
  if (kids) {
    deleteGList(kids, OutlineItem);
    kids = NULL;
  }
}
