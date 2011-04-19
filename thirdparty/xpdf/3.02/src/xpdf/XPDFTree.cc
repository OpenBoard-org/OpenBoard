//========================================================================
//
// XPDFTree.cc
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>
#include <stdlib.h>
#include "gmem.h"
#include "XPDFTreeP.h"

//------------------------------------------------------------------------

#define xpdfTreeIndent 16

//------------------------------------------------------------------------

struct _XPDFTreeEntry {
  Widget widget;
  XPDFTreeEntry *children;
  XPDFTreeEntry *next;
};

//------------------------------------------------------------------------

static void classPartInitialize(WidgetClass widgetClass);
static void initialize(Widget requestWidget, Widget newWidget,
		       ArgList args, Cardinal *numArgs);
static void destroy(Widget widget);
static void destroySubtree(XPDFTreeEntry *e);
static void resize(Widget widget);
static void redisplay(Widget widget, XEvent *event, Region region);
static void redisplaySubtree(XPDFTreeWidget w, XPDFTreeEntry *e,
			     XEvent *event, Region region);
static void drawExpandedIcon(XPDFTreeWidget w, Position x, Position y);
static void drawCollapsedIcon(XPDFTreeWidget w, Position x, Position y);
static Boolean setValues(Widget oldWidget, Widget requestWidget,
			 Widget newWidget, ArgList args, Cardinal *numArgs);
static void setValuesAlmost(Widget oldWidget, Widget newWidget,
			    XtWidgetGeometry *request,
			    XtWidgetGeometry *reply);
static XtGeometryResult queryGeometry(Widget widget,
				      XtWidgetGeometry *request,
				      XtWidgetGeometry *reply);
static XtGeometryResult geometryManager(Widget widget,
					XtWidgetGeometry *request,
					XtWidgetGeometry *reply);
static void changeManaged(Widget widget);
static void initConstraint(Widget requestWidget, Widget newWidget,
			   ArgList args, Cardinal *numArgs);
static void destroyConstraint(Widget widget);
static void deleteSubtree(Widget widget);
static Boolean constraintSetValues(Widget oldWidget, Widget requestWidget,
				   Widget newWidget,
				   ArgList args, Cardinal *numArgs);
static void insertChildOnList(XPDFTreeEntry *e, XPDFTreeEntry **listHead);
static void deleteChildFromList(XPDFTreeEntry *e, XPDFTreeEntry **listHead);
static void createGC(Widget widget);
static void destroyGC(Widget widget);
static void layout(Widget widget, Widget instigator);
static int layoutSubtree(XPDFTreeWidget w, Widget instigator,
			 XPDFTreeEntry *e, Position x, Position y,
			 Boolean visible);
static void calcSize(Widget widget, Widget instigator,
		     Dimension *totalWidth,
		     Dimension *totalHeight);
static void calcSubtreeSize(XPDFTreeWidget w, Widget instigator,
			    XPDFTreeEntry *e,
			    Dimension *width, Dimension *height);
static Boolean needRelayout(Widget oldWidget, Widget newWidget);
static void click(Widget widget, XEvent *event,
		  String *params, Cardinal *numParams);
static Boolean findPosition(XPDFTreeWidget w, int x, int y,
			    XPDFTreeEntry **e, Boolean *onExpandIcon);
static Boolean findPositionInSubtree(XPDFTreeWidget w, int x, int y,
				     XPDFTreeEntry **e,
				     Boolean *onExpandIcon);

//------------------------------------------------------------------------

static XtResource resources[] = {
  { XmNmarginWidth, XmCMarginWidth, XmRHorizontalDimension,
    sizeof(Dimension), XtOffsetOf(XPDFTreeRec, tree.marginWidth),
    XmRImmediate, (XtPointer)0 },
  { XmNmarginHeight, XmCMarginHeight, XmRVerticalDimension,
    sizeof(Dimension), XtOffsetOf(XPDFTreeRec, tree.marginHeight),
    XmRImmediate, (XtPointer)0 },
  { XPDFNselectionCallback, XmCCallback, XmRCallback,
    sizeof(XtCallbackList), XtOffsetOf(XPDFTreeRec, tree.selectCallback),
    XmRImmediate, (XtPointer)NULL }
};

static XmSyntheticResource synResources[] = {
  { XmNmarginWidth, sizeof(Dimension),
    XtOffsetOf(XPDFTreeRec, tree.marginWidth),
#if XmVERSION > 1
    XmeFromHorizontalPixels, XmeToHorizontalPixels
#else
    _XmFromHorizontalPixels, _XmToHorizontalPixels
#endif
  },
  { XmNmarginHeight, sizeof(Dimension),
    XtOffsetOf(XPDFTreeRec, tree.marginHeight),
#if XmVERSION > 1
    XmeFromVerticalPixels, XmeToVerticalPixels
#else
    _XmFromVerticalPixels, _XmToVerticalPixels
#endif
  }
};

static XtResource constraints[] = {
  { XPDFNentryParent, XPDFCentryParent, XmRWidget,
    sizeof(Widget), XtOffsetOf(XPDFTreeConstraintRec, tree.entryParent),
    XmRImmediate, (XtPointer)NULL },
  { XPDFNentryExpanded, XPDFCentryExpanded, XmRBoolean,
    sizeof(Boolean), XtOffsetOf(XPDFTreeConstraintRec, tree.entryExpanded),
    XmRImmediate, (XtPointer)False },
  { XPDFNentryPosition, XPDFCentryPosition, XmRInt,
    sizeof(int), XtOffsetOf(XPDFTreeConstraintRec, tree.entryPosition),
    XmRImmediate, (XtPointer)0 }
};

static char defaultTranslations[] =
  "<Btn1Down>: XPDFTreeClick()";

static XtActionsRec actions[] = {
  { "XPDFTreeClick", click }
};

externaldef(xpdftreeclassrec) XPDFTreeClassRec xpdfTreeClassRec = {
  { // Core
    (WidgetClass)&xmManagerClassRec,		// superclass
    "XPDFTree",					// class_name
    sizeof(XPDFTreeRec),			// widget_size
    NULL,					// class_initialize
    &classPartInitialize,			// class_part_initialize
    FALSE,					// class_inited
    &initialize,				// initialize
    NULL,					// initialize_hook
    XtInheritRealize,				// realize
    actions,					// actions
    XtNumber(actions),				// num_actions
    resources,					// resources
    XtNumber(resources),			// num_resources
    NULLQUARK,					// xrm_class
    TRUE,					// compress_motion
    XtExposeCompressMaximal,			// compress_exposure
    TRUE,					// compress_enterleave
    FALSE,					// visible_interest
    &destroy,					// destroy
    &resize,					// resize
    &redisplay,					// expose
    &setValues,					// set_values
    NULL,					// set_values_hook
    &setValuesAlmost,				// set_values_almost
    NULL,					// get_values_hook
    NULL,					// accept_focus
    XtVersion,					// version
    NULL,					// callback_private
    defaultTranslations,			// tm_table
    &queryGeometry,				// query_geometry
    NULL,					// display_accelerator
    NULL					// extension
  },
  { // Composite
    &geometryManager,				// geometry_manager
    &changeManaged,				// change_managed
    XtInheritInsertChild,			// insert_child
    XtInheritDeleteChild,			// delete_child
    NULL					// extension
  },
  { // Constraint
    constraints,				// constraint_resources
    XtNumber(constraints),			// constraint_num_resources
    sizeof(XPDFTreeConstraintRec),		// constraint_size
    &initConstraint,				// constraint_initialize
    &destroyConstraint,				// constraint_destroy
    &constraintSetValues,			// constraint_set_values
    NULL					// extension
  },
  { // XmManager
    XtInheritTranslations,			// translations
#if XmVERSION > 1
    synResources,				// syn_resources
    XtNumber(synResources),			// num_syn_resources
#else
    NULL,					// syn_resources
    0,						// num_syn_resources
#endif
    NULL,					// syn_constraint_resources
    0,						// num_syn_constraint_res's
    XmInheritParentProcess,			// parent_process
    NULL					// extension
  },
  { // XPDFTree
    &createGC,					// createGC
    &destroyGC,					// destroyGC
    &layout,					// layout
    &calcSize,					// calcSize
    &needRelayout,				// needRelayout
    NULL					// extension
  }
};

externaldef(xpdftreewidgetclass) WidgetClass xpdfTreeWidgetClass =
  (WidgetClass)&xpdfTreeClassRec;

//------------------------------------------------------------------------

static void classPartInitialize(WidgetClass widgetCls) {
  XPDFTreeWidgetClass wc = (XPDFTreeWidgetClass)widgetCls;
  XPDFTreeWidgetClass sc = (XPDFTreeWidgetClass)wc->coreClass.superclass;

  // method inheritance
  if (wc->treeClass.createGC == XPDFInheritCreateGC) {
    wc->treeClass.createGC = sc->treeClass.createGC;
  }
  if (wc->treeClass.destroyGC == XPDFInheritDestroyGC) {
    wc->treeClass.destroyGC = sc->treeClass.destroyGC;
  }
  if (wc->treeClass.layout == XPDFInheritLayout) {
    wc->treeClass.layout = sc->treeClass.layout;
  }
  if (wc->treeClass.calcSize == XPDFInheritCalcSize) {
    wc->treeClass.calcSize = sc->treeClass.calcSize;
  }
  if (wc->treeClass.needRelayout == XPDFInheritNeedRelayout) {
    wc->treeClass.needRelayout = sc->treeClass.needRelayout;
  }
}

static void initialize(Widget requestWidget, Widget newWidget,
		       ArgList args, Cardinal *numArgs) {
  XPDFTreeWidget nw = (XPDFTreeWidget)newWidget;
  XPDFTreeWidgetClass cls = (XPDFTreeWidgetClass)XtClass(newWidget);

  nw->tree.root = NULL;
  nw->tree.redrawY = -1;
  if (cls->treeClass.createGC) {
    (*cls->treeClass.createGC)(newWidget);
  } else {
    createGC(newWidget);
  }
}

static void destroy(Widget widget) {
  XPDFTreeWidget w = (XPDFTreeWidget)widget;
  XPDFTreeWidgetClass cls = (XPDFTreeWidgetClass)XtClass(widget);

  if (w->tree.root) {
    destroySubtree(w->tree.root);
    w->tree.root = NULL;
  }
  if (cls->treeClass.destroyGC) {
    (*cls->treeClass.destroyGC)(widget);
  } else {
    destroyGC(widget);
  }
}

static void destroySubtree(XPDFTreeEntry *e) {
  if (e->children) {
    destroySubtree(e->children);
  }
  if (e->next) {
    destroySubtree(e->next);
  }
}

static void resize(Widget widget) {
  XPDFTreeWidgetClass cls = (XPDFTreeWidgetClass)XtClass(widget);

  if (cls->treeClass.layout) {
    (*cls->treeClass.layout)(widget, NULL);
  } else {
    layout(widget, NULL);
  }
}

static void redisplay(Widget widget, XEvent *event, Region region) {
  XPDFTreeWidget w = (XPDFTreeWidget)widget;
  XPDFTreeEntry *e;

  if (w->tree.redrawY >= 0) {
    XClearArea(XtDisplay((Widget)w), XtWindow((Widget)w),
	       0, w->tree.redrawY, w->core.width, w->core.height, False);
    w->tree.redrawY = -1;
  }
  for (e = w->tree.root; e; e = e->next) {
    redisplaySubtree(w, e, event, region);
  }
}

static void redisplaySubtree(XPDFTreeWidget w, XPDFTreeEntry *e,
			     XEvent *event, Region region) {
  XPDFTreeConstraint c;
  Position x, y, y2;
  XPDFTreeEntry *child;

  (*XtClass(e->widget)->core_class.expose)(e->widget, event, region);
  c = XPDFTreeCPart(e->widget);
  x = e->widget->core.x;
  y = e->widget->core.y + e->widget->core.height / 2;
  if (e->children) {
    if (c->entryExpanded) {
      drawExpandedIcon(w, x - 8, y);
      y2 = y; // make gcc happy
      for (child = e->children; child; child = child->next) {
	y2 = child->widget->core.y + child->widget->core.height / 2;
	XDrawLine(XtDisplay((Widget)w), XtWindow((Widget)w), w->tree.dottedGC,
		  x - 8, y2, x + 6, y2);
	redisplaySubtree(w, child, event, region);
      }
      XDrawLine(XtDisplay((Widget)w), XtWindow((Widget)w), w->tree.dottedGC,
		x - 8, y + 2, x - 8, y2);
    } else {
      drawCollapsedIcon(w, x - 8, y);
    }
  }
}

static void drawExpandedIcon(XPDFTreeWidget w, Position x, Position y) {
  XPoint pts[4];

  pts[0].x = x - 4;    pts[0].y = y - 2;
  pts[1].x = x + 4;    pts[1].y = y - 2;
  pts[2].x = x;        pts[2].y = y + 2;
  pts[3].x = x - 4;    pts[3].y = y - 2;
  XDrawLines(XtDisplay((Widget)w), XtWindow((Widget)w), w->tree.plainGC,
	     pts, 4, CoordModeOrigin);
}

static void drawCollapsedIcon(XPDFTreeWidget w, Position x, Position y) {
  XPoint pts[4];

  pts[0].x = x - 2;    pts[0].y = y - 4;
  pts[1].x = x - 2;    pts[1].y = y + 4;
  pts[2].x = x + 2;    pts[2].y = y;
  pts[3].x = x - 2;    pts[3].y = y - 4;
  XDrawLines(XtDisplay((Widget)w), XtWindow((Widget)w), w->tree.plainGC,
	     pts, 4, CoordModeOrigin);
}

static Boolean setValues(Widget oldWidget, Widget requestWidget,
			 Widget newWidget, ArgList args, Cardinal *numArgs) {
  XPDFTreeWidget ow = (XPDFTreeWidget)oldWidget;
  XPDFTreeWidget nw = (XPDFTreeWidget)newWidget;
  XPDFTreeWidgetClass cls = (XPDFTreeWidgetClass)XtClass(nw);
  Boolean relayout, redisp;

  // check to see if layout-affecting resources have changed
  if (cls->treeClass.needRelayout) {
    relayout = (*cls->treeClass.needRelayout)((Widget)ow, (Widget)nw);
  } else {
    relayout = needRelayout((Widget)ow, (Widget)nw);
  }
  redisp = False;
  if (relayout) {

    // calculate a new ideal size (reset the widget size first so
    // calcSize will compute a new one)
    if (nw->core.width == ow->core.width) {
      nw->core.width = 0;
    }
    if (nw->core.height == ow->core.height) {
      nw->core.height = 0;
    }
    if (cls->treeClass.calcSize) {
      (*cls->treeClass.calcSize)((Widget)nw, NULL,
				 &nw->core.width, &nw->core.height);
    } else {
      calcSize((Widget)nw, NULL, &nw->core.width, &nw->core.height);
    }

    // if resources have changed but size hasn't, layout manually
    // (because Xt just looks at the size)
    if (nw->core.width == ow->core.width &&
	nw->core.height == ow->core.height) {
      if (cls->treeClass.layout) {
	(*cls->treeClass.layout)((Widget)nw, NULL);
      } else {
	layout((Widget)nw, NULL);
      }
      redisp = True;
    }
  }

  return redisp;
}

static void setValuesAlmost(Widget oldWidget, Widget newWidget,
			    XtWidgetGeometry *request,
			    XtWidgetGeometry *reply) {
  XPDFTreeWidgetClass cls = (XPDFTreeWidgetClass)XtClass(newWidget);

  // our parent rejected a geometry request, so accept the compromise
  // and relayout
  if (!reply->request_mode) {
    if (cls->treeClass.layout) {
      (*cls->treeClass.layout)(newWidget, NULL);
    } else {
      layout(newWidget, NULL);
    }
  }
  *request = *reply;
}

static XtGeometryResult queryGeometry(Widget widget,
				      XtWidgetGeometry *request,
				      XtWidgetGeometry *reply) {
  XPDFTreeWidgetClass cls = (XPDFTreeWidgetClass)XtClass(widget);

  if (!XtIsRealized(widget)) {
    reply->width = XtWidth(widget);
    reply->height = XtHeight(widget);
  } else {
    reply->width = 0;
    reply->height = 0;
  }
  if (cls->treeClass.calcSize) {
    (*cls->treeClass.calcSize)(widget, NULL, &reply->width, &reply->height);
  } else {
    calcSize(widget, NULL, &reply->width, &reply->height);
  }
#if XmVERSION > 1
  return XmeReplyToQueryGeometry(widget, request, reply);
#else
  if ((request->request_mode & CWWidth) &&
      (request->request_mode & CWHeight) &&
      request->width == reply->width &&
      request->height == reply->height) {
    return XtGeometryYes;
  }
  if (reply->width == XtWidth(widget) &&
      reply->height == XtHeight(widget)) {
    return XtGeometryNo;
  }
  reply->request_mode = CWWidth | CWHeight;
  return XtGeometryAlmost;
#endif
}

static XtGeometryResult geometryManager(Widget widget,
					XtWidgetGeometry *request,
					XtWidgetGeometry *reply) {
  XPDFTreeWidget w = (XPDFTreeWidget)XtParent(widget);
  XPDFTreeWidgetClass cls = (XPDFTreeWidgetClass)XtClass(w);
  Dimension curWidth, curHeight, curBW;
  XtWidgetGeometry parentReq;
  XtGeometryResult result;

  // deny any requests for a new position
  if ((request->request_mode & CWX) || (request->request_mode & CWY)) {
    return XtGeometryNo;
  }

  // save the current geometry
  curWidth = w->core.width;
  curHeight = w->core.height;
  curBW = w->core.border_width;

  // make the requested changes
  if (request->request_mode & CWWidth) {
    w->core.width = request->width;
  }
  if (request->request_mode & CWHeight) {
    w->core.height = request->height;
  }
  if (request->request_mode & CWBorderWidth) {
    w->core.border_width = request->border_width;
  }

  // calculate the new ideal size
  parentReq.width = 0;
  parentReq.height = 0;
  if (cls->treeClass.calcSize) {
    (*cls->treeClass.calcSize)((Widget)w, widget,
			       &parentReq.width, &reply->height);
  } else {
    calcSize((Widget)w, widget, &parentReq.width, &reply->height);
  }

  // send geometry request to our parent
  parentReq.request_mode = CWWidth | CWHeight;
  if (request->request_mode & XtCWQueryOnly) {
    parentReq.request_mode |= XtCWQueryOnly;
  }
  result = XtMakeGeometryRequest((Widget)w, &parentReq, NULL);
  if (result == XtGeometryAlmost) {
    result = XtGeometryNo;
  }

  if (result == XtGeometryNo || (request->request_mode & XtCWQueryOnly)) {
    // restore the original geometry
    w->core.width = curWidth;
    w->core.height = curHeight;
    w->core.border_width = curBW;
  } else {
    if (cls->treeClass.layout) {
      (*cls->treeClass.layout)((Widget)w, widget);
    } else {
      layout((Widget)w, widget);
    }
  }

  return result;
}

static void changeManaged(Widget widget) {
  Dimension width, height;
  XPDFTreeWidgetClass cls = (XPDFTreeWidgetClass)XtClass(widget);

  // compute the ideal size
  if (!XtIsRealized(widget)) {
    width = XtWidth(widget);
    height = XtHeight(widget);
  } else {
    width = 0;
    height = 0;
  }
  if (cls->treeClass.calcSize) {
    (*cls->treeClass.calcSize)(widget, NULL, &width, &height);
  } else {
    calcSize(widget, NULL, &width, &height);
  }

  // make resize request to parent -- keep asking until we get a yes
  // or no
  while (XtMakeResizeRequest(widget, width, height, &width, &height)
	 == XtGeometryAlmost) ;

  // relayout
  if (cls->treeClass.layout) {
    (*cls->treeClass.layout)(widget, NULL);
  } else {
    layout(widget, NULL);
  }

#if XmVERSION > 1
  // update keyboard traversal
  XmeNavigChangeManaged(widget);
#else
  _XmNavigChangeManaged(widget);
#endif
}

static void initConstraint(Widget requestWidget, Widget newWidget,
			   ArgList args, Cardinal *numArgs) {
  XPDFTreeWidget w = (XPDFTreeWidget)XtParent(newWidget);
  XPDFTreeConstraint c;

  c = XPDFTreeCPart(newWidget);
  c->e = (XPDFTreeEntry *)gmalloc(sizeof(XPDFTreeEntry));
  c->e->widget = newWidget;
  c->e->children = NULL;
  c->e->next = NULL;
  if (c->entryParent) {
    insertChildOnList(c->e, &XPDFTreeCPart(c->entryParent)->e->children);
  } else {
    insertChildOnList(c->e, &w->tree.root);
  }
}

static void destroyConstraint(Widget widget) {
  deleteSubtree(widget);
}

static void deleteSubtree(Widget widget) {
  XPDFTreeWidget w = (XPDFTreeWidget)XtParent(widget);
  XPDFTreeConstraint c;

  c = XPDFTreeCPart(widget);
  if (!c->e) {
    return;
  }
  while (c->e->children) {
    deleteSubtree(c->e->children->widget);
  }
  if (c->entryParent) {
    deleteChildFromList(c->e, &XPDFTreeCPart(c->entryParent)->e->children);
  } else {
    deleteChildFromList(c->e, &w->tree.root);
  }
  gfree(c->e);
  c->e = NULL;
}

static Boolean constraintSetValues(Widget oldWidget, Widget requestWidget,
				   Widget newWidget,
				   ArgList args, Cardinal *numArgs) {
  XPDFTreeWidget w = (XPDFTreeWidget)XtParent(newWidget);
  XPDFTreeWidgetClass cls = (XPDFTreeWidgetClass)XtClass((Widget)w);
  XPDFTreeConstraint oc, nc;
  Boolean relayout;
  Dimension width, height;

  if (!XtIsManaged(newWidget)) {
    return False;
  }
  oc = XPDFTreeCPart(oldWidget);
  nc = XPDFTreeCPart(newWidget);
  relayout = False;
  if (nc->entryParent != oc->entryParent ||
      nc->entryPosition != oc->entryPosition) {
    if (oc->entryParent) {
      deleteChildFromList(oc->e, &XPDFTreeCPart(oc->entryParent)->e->children);
    } else {
      deleteChildFromList(oc->e, &w->tree.root);
    }
    if (nc->entryParent) {
      insertChildOnList(nc->e, &XPDFTreeCPart(nc->entryParent)->e->children);
    } else {
      insertChildOnList(nc->e, &w->tree.root);
    }
    relayout = True;
  } else if (nc->entryExpanded != oc->entryExpanded) {
    relayout = True;
  }

  if (relayout) {

    // calculate a new ideal size (reset the widget size first so
    // calcSize will compute a new one)
    width = 0;
    height = 0;
    if (cls->treeClass.calcSize) {
      (*cls->treeClass.calcSize)((Widget)w, NULL, &width, &height);
    } else {
      calcSize((Widget)w, NULL, &width, &height);
    }

    // make resize request to parent -- keep asking until we get a yes
    // or no
    while (XtMakeResizeRequest((Widget)w, width, height, &width, &height)
	   == XtGeometryAlmost) ;

    // relayout the widget
    if (cls->treeClass.layout) {
      (*cls->treeClass.layout)((Widget)w, NULL);
    } else {
      layout((Widget)w, NULL);
    }
  }

  return relayout;
}

static void insertChildOnList(XPDFTreeEntry *e, XPDFTreeEntry **listHead) {
  int pos;
  XPDFTreeEntry *e2;

  pos = XPDFTreeCPart(e->widget)->entryPosition;
  if (!*listHead || pos < XPDFTreeCPart((*listHead)->widget)->entryPosition) {
    e->next = *listHead;
    *listHead = e;
  } else {
    for (e2 = *listHead;
	 e2->next && pos >= XPDFTreeCPart(e2->next->widget)->entryPosition;
	 e2 = e2->next) ;
    e->next = e2->next;
    e2->next = e;
  }
}

static void deleteChildFromList(XPDFTreeEntry *e, XPDFTreeEntry **listHead) {
  XPDFTreeEntry **p;

  for (p = listHead; *p; p = &(*p)->next) {
    if (*p == e) {
      *p = e->next;
      e->next = NULL;
      return;
    }
  }
}

static void createGC(Widget widget) {
  XPDFTreeWidget w = (XPDFTreeWidget)widget;
  XGCValues gcValues;

  gcValues.foreground = w->manager.foreground;
  gcValues.line_width = 0;
  gcValues.line_style = LineSolid;
  w->tree.plainGC = XtGetGC(widget,
			    GCForeground | GCLineWidth | GCLineStyle,
			    &gcValues);

  gcValues.line_style = LineOnOffDash;
  gcValues.dashes = 1;
  gcValues.dash_offset = 0;
  w->tree.dottedGC = XtGetGC(widget,
			     GCForeground | GCLineWidth | GCLineStyle |
			         GCDashList | GCDashOffset,
			     &gcValues);
}

static void destroyGC(Widget widget) {
  XPDFTreeWidget w = (XPDFTreeWidget)widget;

  XtReleaseGC(widget, w->tree.plainGC);
  XtReleaseGC(widget, w->tree.dottedGC);
}

static void layout(Widget widget, Widget instigator) {
  XPDFTreeWidget w = (XPDFTreeWidget)widget;
  XPDFTreeEntry *e;
  Position x, y;

  x = w->tree.marginWidth + xpdfTreeIndent;
  y = w->tree.marginHeight;
  for (e = w->tree.root; e; e = e->next) {
    y = layoutSubtree(w, instigator, e, x, y, True);
  }
}

static int layoutSubtree(XPDFTreeWidget w, Widget instigator,
			 XPDFTreeEntry *e, Position x, Position y,
			 Boolean visible) {
  Widget ew;
  XPDFTreeEntry *child;
  XPDFTreeConstraint c;
  int dy;

  ew = e->widget;
  if (!XtIsManaged(ew)) {
    return y;
  }
  c = XPDFTreeCPart(ew);

  // place this entry
  if (ew) {
    if (visible) {
      if (ew == instigator) {
	ew->core.x = x;
	ew->core.y = y;
      } else {
#if XmVERSION > 1
	XmeConfigureObject(ew, x, y, ew->core.width, ew->core.height,
			   ew->core.border_width);
#else
	_XmConfigureObject(ew, x, y, ew->core.width, ew->core.height,
			   ew->core.border_width);
#endif
      }
      dy = ew->core.height + 2 * ew->core.border_width;
      // this is a kludge to avoid crashes if the widget becomes too
      // tall
      if ((int)y + dy > 32767) {
	y = 32767;
      } else {
	y += dy;
      }
    }
  }

  // place this entry's children
  x += xpdfTreeIndent;
  for (child = e->children; child; child = child->next) {
    y = layoutSubtree(w, instigator, child, x, y,
		      visible && (!c || c->entryExpanded));
  }

  return y;
}

static void calcSize(Widget widget, Widget instigator,
		     Dimension *totalWidth,
		     Dimension *totalHeight) {
  XPDFTreeWidget w = (XPDFTreeWidget)widget;
  XPDFTreeEntry *e;
  int h1;
  Dimension w1, w2, h2;

  w1 = h1 = 0;
  for (e = w->tree.root; e; e = e->next) {
    calcSubtreeSize(w, instigator, e, &w2, &h2);
    if (w2 > w1) {
      w1 = w2;
    }
    h1 += (int)h2;
  }
  w1 += xpdfTreeIndent + 2 * w->tree.marginWidth;
  h1 += 2 * (int)w->tree.marginHeight;
  if (h1 == 0) {
    h1 = 1;
  } else if (h1 > 32767) {
    // this is a kludge to avoid crashes if the widget becomes too
    // tall
    h1 = 32767;
  }
  if (!*totalWidth) {
    *totalWidth = w1;
  }
  if (!*totalHeight) {
    *totalHeight = (Dimension)h1;
  }
}

static void calcSubtreeSize(XPDFTreeWidget w, Widget instigator,
			    XPDFTreeEntry *e,
			    Dimension *width, Dimension *height) {
  Widget ew;
  XPDFTreeEntry *child;
  XPDFTreeConstraint c;
  XtWidgetGeometry geom;
  int h1;
  Dimension w1, w2, h2;
  
  ew = e->widget;
  if (!XtIsManaged(ew)) {
    *width = *height = 0;
    return;
  }
  c = XPDFTreeCPart(ew);

  // get size of this entry
  if (ew) {
    if (!XtIsManaged(ew)) {
      *width = *height = 0;
      return;
    }
    if (ew == instigator) {
      w1 = ew->core.width;
      h1 = (int)ew->core.height;
    } else {
      XtQueryGeometry(ew, NULL, &geom);
      w1 = (geom.request_mode & CWWidth) ? geom.width : ew->core.width;
      h1 = (int)((geom.request_mode & CWHeight) ? geom.height
		                                : ew->core.height);
    }
    h1 += 2 * (int)ew->core.border_width;
  } else {
    // root of tree
    w1 = 0;
    h1 = 0;
  }

  // if this entry is expanded, get size of all of its children
  if (c->entryExpanded) {
    for (child = e->children; child; child = child->next) {
      calcSubtreeSize(w, instigator, child, &w2, &h2);
      w2 += xpdfTreeIndent;
      if (w2 > w1) {
	w1 = w2;
      }
      h1 += (int)h2;
    }
  }

  // this is a kludge to avoid crashes if the widget becomes too tall
  if (h1 > 32767) {
    h1 = 32767;
  }
  *width = w1;
  *height = h1;
}

static Boolean needRelayout(Widget oldWidget, Widget newWidget) {
  XPDFTreeWidget ow = (XPDFTreeWidget)oldWidget;
  XPDFTreeWidget nw = (XPDFTreeWidget)newWidget;

  if (nw->tree.marginWidth != ow->tree.marginWidth ||
      nw->tree.marginHeight != ow->tree.marginHeight) {
    return True;
  }
  return False;
}

static void click(Widget widget, XEvent *event,
		  String *params, Cardinal *numParams) {
  XPDFTreeWidget w = (XPDFTreeWidget)widget;
  XButtonPressedEvent *bpe;
  XPDFTreeEntry *e;
  Boolean onExpandIcon;
  XPDFTreeConstraint c;
  XPDFTreeSelectCallbackStruct cbs;

  if (event->type != ButtonPress) {
    return;
  }
  bpe = (XButtonPressedEvent *)event;
  if (findPosition(w, bpe->x, bpe->y, &e, &onExpandIcon)) {
    if (onExpandIcon) {
      c = XPDFTreeCPart(e->widget);
      w->tree.redrawY = e->widget->core.y;
      XtVaSetValues(e->widget, XPDFNentryExpanded, !c->entryExpanded, NULL);
    } else {
      XmProcessTraversal(e->widget, XmTRAVERSE_CURRENT);
      XtCallActionProc(widget, "ManagerGadgetActivate", event, NULL, 0);
      cbs.reason = XmCR_ACTIVATE;
      cbs.event = event;
      cbs.selectedItem = e->widget;
      XtCallCallbackList(widget, w->tree.selectCallback, &cbs);
    }
  }
}

static Boolean findPosition(XPDFTreeWidget w, int x, int y,
			    XPDFTreeEntry **e, Boolean *onExpandIcon) {
  XPDFTreeEntry *e2;

  for (e2 = w->tree.root; e2; e2 = e2->next) {
    *e = e2;
    if (findPositionInSubtree(w, x, y, e, onExpandIcon)) {
      return True;
    }
  }
  return False;
}

// If (x,y) falls on either an expand/collapse icon or a label gadget,
// set *<e> and *<onExpandIcon> and return true.
static Boolean findPositionInSubtree(XPDFTreeWidget w, int x, int y,
				     XPDFTreeEntry **e,
				     Boolean *onExpandIcon) {
  Widget child;
  XPDFTreeConstraint c;
  XPDFTreeEntry *e2;
  int y1;

  child = (*e)->widget;
  y1 = child->core.y + child->core.height / 2;
  if (x >= child->core.x && x < child->core.x + child->core.width &&
      y >= child->core.y && y < child->core.y + child->core.height) {
    *onExpandIcon = False;
    return True;
  } else if (x >= child->core.x - 16 && x < child->core.x - 4 &&
	     y >= y1 - 6 && y < y1 + 6 &&
	     (*e)->children) {
    *onExpandIcon = True;
    return True;
  }
  c = XPDFTreeCPart(child);
  if (!c || c->entryExpanded) {
    for (e2 = (*e)->children; e2; e2 = e2->next) {
      *e = e2;
      if (findPositionInSubtree(w, x, y, e, onExpandIcon)) {
	return True;
      }
    }
  }
  return False;
}

Widget XPDFCreateTree(Widget parent, char *name,
		      ArgList argList, Cardinal numArgs) {
  return XtCreateWidget(name, xpdfTreeWidgetClass, parent, argList, numArgs);
}
