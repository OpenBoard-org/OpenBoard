//========================================================================
//
// Outline.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef OUTLINE_H
#define OUTLINE_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Object.h"
#include "CharTypes.h"

class GString;
class GList;
class XRef;
class LinkAction;

//------------------------------------------------------------------------

class Outline {
public:

  Outline(Object *outlineObj, XRef *xref);
  ~Outline();

  GList *getItems() { return items; }

private:

  GList *items;			// NULL if document has no outline
				//   [OutlineItem]
};

//------------------------------------------------------------------------

class OutlineItem {
public:

  OutlineItem(Dict *dict, XRef *xrefA);
  ~OutlineItem();

  static GList *readItemList(Object *firstItemRef, Object *lastItemRef,
			     XRef *xrefA);

  void open();
  void close();

  Unicode *getTitle() { return title; }
  int getTitleLength() { return titleLen; }
  LinkAction *getAction() { return action; }
  GBool isOpen() { return startsOpen; }
  GBool hasKids() { return firstRef.isRef(); }
  GList *getKids() { return kids; }

private:

  XRef *xref;
  Unicode *title;
  int titleLen;
  LinkAction *action;
  Object firstRef;
  Object lastRef;
  Object nextRef;
  GBool startsOpen;
  GList *kids;			// NULL unless this item is open [OutlineItem]
};

#endif
