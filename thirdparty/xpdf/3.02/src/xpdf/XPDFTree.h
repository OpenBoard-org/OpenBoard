//========================================================================
//
// XPDFTree.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef XPDFTREE_H
#define XPDFTREE_H

#include <aconf.h>
#include <Xm/Xm.h>

extern "C" {

externalref WidgetClass xpdfTreeWidgetClass;

typedef struct _XPDFTreeClassRec *XPDFTreeWidgetClass;
typedef struct _XPDFTreeRec      *XPDFTreeWidget;

#ifndef XPDFIsTree
#define XPDFIsTree(w) XtIsSubclass(w, xpdfTreeWidgetClass)
#endif

#define XPDFNentryParent       "entryParent"
#define XPDFNentryExpanded     "entryExpanded"
#define XPDFNentryPosition     "entryPosition"
#define XPDFNselectionCallback "selectionCallback"

#define XPDFCentryParent       "EntryParent"
#define XPDFCentryExpanded     "EntryExpanded"
#define XPDFCentryPosition     "EntryPosition"

typedef struct {
  int reason;
  XEvent *event;
  Widget selectedItem;
} XPDFTreeSelectCallbackStruct;

extern Widget XPDFCreateTree(Widget parent, char *name,
			     ArgList argList, Cardinal argCount);

} // extern "C"

#endif
