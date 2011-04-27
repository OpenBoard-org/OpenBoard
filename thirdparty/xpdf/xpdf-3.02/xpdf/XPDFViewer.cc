//========================================================================
//
// XPDFViewer.cc
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <X11/keysym.h>
#include <X11/cursorfont.h>
#ifdef HAVE_X11_XPM_H
#include <X11/xpm.h>
#endif
#if defined(__sgi) && (XmVERSION <= 1)
#define Object XtObject
#include <Sgm/HPanedW.h>
#undef Object
#endif
#include "gmem.h"
#include "gfile.h"
#include "GString.h"
#include "GList.h"
#include "Error.h"
#include "GlobalParams.h"
#include "PDFDoc.h"
#include "Link.h"
#include "ErrorCodes.h"
#include "Outline.h"
#include "UnicodeMap.h"
#ifndef DISABLE_OUTLINE
#define Object XtObject
#include "XPDFTree.h"
#undef Object
#endif
#include "XPDFApp.h"
#include "XPDFViewer.h"
#include "PSOutputDev.h"
#include "config.h"

// these macro defns conflict with xpdf's Object class
#ifdef LESSTIF_VERSION
#undef XtDisplay
#undef XtScreen
#undef XtWindow
#undef XtParent
#undef XtIsRealized
#endif

#if XmVERSION <= 1
#define XmSET   True
#define XmUNSET False
#endif

// hack around old X includes which are missing these symbols
#ifndef XK_Page_Up
#define XK_Page_Up              0xFF55
#endif
#ifndef XK_Page_Down
#define XK_Page_Down            0xFF56
#endif
#ifndef XK_KP_Home
#define XK_KP_Home              0xFF95
#endif
#ifndef XK_KP_Left
#define XK_KP_Left              0xFF96
#endif
#ifndef XK_KP_Up
#define XK_KP_Up                0xFF97
#endif
#ifndef XK_KP_Right
#define XK_KP_Right             0xFF98
#endif
#ifndef XK_KP_Down
#define XK_KP_Down              0xFF99
#endif
#ifndef XK_KP_Prior
#define XK_KP_Prior             0xFF9A
#endif
#ifndef XK_KP_Page_Up
#define XK_KP_Page_Up           0xFF9A
#endif
#ifndef XK_KP_Next
#define XK_KP_Next              0xFF9B
#endif
#ifndef XK_KP_Page_Down
#define XK_KP_Page_Down         0xFF9B
#endif
#ifndef XK_KP_End
#define XK_KP_End               0xFF9C
#endif
#ifndef XK_KP_Begin
#define XK_KP_Begin             0xFF9D
#endif
#ifndef XK_KP_Insert
#define XK_KP_Insert            0xFF9E
#endif
#ifndef XK_KP_Delete
#define XK_KP_Delete            0xFF9F
#endif

//------------------------------------------------------------------------
// GUI includes
//------------------------------------------------------------------------

#include "xpdfIcon.xpm"
#include "leftArrow.xbm"
#include "leftArrowDis.xbm"
#include "dblLeftArrow.xbm"
#include "dblLeftArrowDis.xbm"
#include "rightArrow.xbm"
#include "rightArrowDis.xbm"
#include "dblRightArrow.xbm"
#include "dblRightArrowDis.xbm"
#include "backArrow.xbm"
#include "backArrowDis.xbm"
#include "forwardArrow.xbm"
#include "forwardArrowDis.xbm"
#include "find.xbm"
#include "findDis.xbm"
#include "print.xbm"
#include "printDis.xbm"
#include "about.xbm"
#include "about-text.h"

//------------------------------------------------------------------------

struct ZoomMenuInfo {
  char *label;
  double zoom;
};

static ZoomMenuInfo zoomMenuInfo[nZoomMenuItems] = {
  { "400%",      400 },
  { "200%",      200 },
  { "150%",      150 },
  { "125%",      125 },
  { "100%",      100 },
  { "50%",        50 },
  { "25%",        25 },
  { "12.5%",      12.5 },
  { "fit page",  zoomPage },
  { "fit width", zoomWidth }
};

#define maxZoomIdx   0
#define defZoomIdx   3
#define minZoomIdx   7
#define zoomPageIdx  8
#define zoomWidthIdx 9

//------------------------------------------------------------------------

#define cmdMaxArgs 2

XPDFViewerCmd XPDFViewer::cmdTab[] = {
  { "about",                   0, gFalse, gFalse, &XPDFViewer::cmdAbout },
  { "closeOutline",            0, gFalse, gFalse, &XPDFViewer::cmdCloseOutline },
  { "closeWindow",             0, gFalse, gFalse, &XPDFViewer::cmdCloseWindow },
  { "continuousMode",          0, gFalse, gFalse, &XPDFViewer::cmdContinuousMode },
  { "endPan",                  0, gTrue,  gTrue,  &XPDFViewer::cmdEndPan },
  { "endSelection",            0, gTrue,  gTrue,  &XPDFViewer::cmdEndSelection },
  { "find",                    0, gTrue,  gFalse, &XPDFViewer::cmdFind },
  { "findNext",                0, gTrue,  gFalse, &XPDFViewer::cmdFindNext },
  { "focusToDocWin",           0, gFalse, gFalse, &XPDFViewer::cmdFocusToDocWin },
  { "focusToPageNum",          0, gFalse, gFalse, &XPDFViewer::cmdFocusToPageNum },
  { "followLink",              0, gTrue,  gTrue,  &XPDFViewer::cmdFollowLink },
  { "followLinkInNewWin",      0, gTrue,  gTrue,  &XPDFViewer::cmdFollowLinkInNewWin },
  { "followLinkInNewWinNoSel", 0, gTrue,  gTrue,  &XPDFViewer::cmdFollowLinkInNewWinNoSel },
  { "followLinkNoSel",         0, gTrue,  gTrue,  &XPDFViewer::cmdFollowLinkNoSel },
  { "fullScreenMode",          0, gFalse, gFalse, &XPDFViewer::cmdFullScreenMode },
  { "goBackward",              0, gFalse, gFalse, &XPDFViewer::cmdGoBackward },
  { "goForward",               0, gFalse, gFalse, &XPDFViewer::cmdGoForward },
  { "gotoDest",                1, gTrue,  gFalse, &XPDFViewer::cmdGotoDest },
  { "gotoLastPage",            0, gTrue,  gFalse, &XPDFViewer::cmdGotoLastPage },
  { "gotoLastPageNoScroll",    0, gTrue,  gFalse, &XPDFViewer::cmdGotoLastPageNoScroll },
  { "gotoPage",                1, gTrue,  gFalse, &XPDFViewer::cmdGotoPage },
  { "gotoPageNoScroll",        1, gTrue,  gFalse, &XPDFViewer::cmdGotoPageNoScroll },
  { "nextPage",                0, gTrue,  gFalse, &XPDFViewer::cmdNextPage },
  { "nextPageNoScroll",        0, gTrue,  gFalse, &XPDFViewer::cmdNextPageNoScroll },
  { "open",                    0, gFalse, gFalse, &XPDFViewer::cmdOpen },
  { "openFile",                1, gFalse, gFalse, &XPDFViewer::cmdOpenFile },
  { "openFileAtDest",          2, gFalse, gFalse, &XPDFViewer::cmdOpenFileAtDest },
  { "openFileAtDestInNewWin",  2, gFalse, gFalse, &XPDFViewer::cmdOpenFileAtDestInNewWin },
  { "openFileAtPage",          2, gFalse, gFalse, &XPDFViewer::cmdOpenFileAtPage },
  { "openFileAtPageInNewWin",  2, gFalse, gFalse, &XPDFViewer::cmdOpenFileAtPageInNewWin },
  { "openFileInNewWin",        1, gFalse, gFalse, &XPDFViewer::cmdOpenFileInNewWin },
  { "openInNewWin",            0, gFalse, gFalse, &XPDFViewer::cmdOpenInNewWin },
  { "openOutline",             0, gFalse, gFalse, &XPDFViewer::cmdOpenOutline },
  { "pageDown",                0, gTrue,  gFalse, &XPDFViewer::cmdPageDown },
  { "pageUp",                  0, gTrue,  gFalse, &XPDFViewer::cmdPageUp },
  { "postPopupMenu",           0, gFalse, gTrue,  &XPDFViewer::cmdPostPopupMenu },
  { "prevPage",                0, gTrue,  gFalse, &XPDFViewer::cmdPrevPage },
  { "prevPageNoScroll",        0, gTrue,  gFalse, &XPDFViewer::cmdPrevPageNoScroll },
  { "print",                   0, gTrue,  gFalse, &XPDFViewer::cmdPrint },
  { "quit",                    0, gFalse, gFalse, &XPDFViewer::cmdQuit },
  { "raise",                   0, gFalse, gFalse, &XPDFViewer::cmdRaise },
  { "redraw",                  0, gTrue,  gFalse, &XPDFViewer::cmdRedraw },
  { "reload",                  0, gTrue,  gFalse, &XPDFViewer::cmdReload },
  { "run",                     1, gFalse, gFalse, &XPDFViewer::cmdRun },
  { "scrollDown",              1, gTrue,  gFalse, &XPDFViewer::cmdScrollDown },
  { "scrollDownNextPage",      1, gTrue,  gFalse, &XPDFViewer::cmdScrollDownNextPage },
  { "scrollLeft",              1, gTrue,  gFalse, &XPDFViewer::cmdScrollLeft },
  { "scrollOutlineDown",       1, gTrue,  gFalse, &XPDFViewer::cmdScrollOutlineDown },
  { "scrollOutlineUp",         1, gTrue,  gFalse, &XPDFViewer::cmdScrollOutlineUp },
  { "scrollRight",             1, gTrue,  gFalse, &XPDFViewer::cmdScrollRight },
  { "scrollToBottomEdge",      0, gTrue,  gFalse, &XPDFViewer::cmdScrollToBottomEdge },
  { "scrollToBottomRight",     0, gTrue,  gFalse, &XPDFViewer::cmdScrollToBottomRight },
  { "scrollToLeftEdge",        0, gTrue,  gFalse, &XPDFViewer::cmdScrollToLeftEdge },
  { "scrollToRightEdge",       0, gTrue,  gFalse, &XPDFViewer::cmdScrollToRightEdge },
  { "scrollToTopEdge",         0, gTrue,  gFalse, &XPDFViewer::cmdScrollToTopEdge },
  { "scrollToTopLeft",         0, gTrue,  gFalse, &XPDFViewer::cmdScrollToTopLeft },
  { "scrollUp",                1, gTrue,  gFalse, &XPDFViewer::cmdScrollUp },
  { "scrollUpPrevPage",        1, gTrue,  gFalse, &XPDFViewer::cmdScrollUpPrevPage },
  { "singlePageMode",          0, gFalse, gFalse, &XPDFViewer::cmdSinglePageMode },
  { "startPan",                0, gTrue,  gTrue,  &XPDFViewer::cmdStartPan },
  { "startSelection",          0, gTrue,  gTrue,  &XPDFViewer::cmdStartSelection },
  { "toggleContinuousMode",    0, gFalse, gFalse, &XPDFViewer::cmdToggleContinuousMode },
  { "toggleFullScreenMode",    0, gFalse, gFalse, &XPDFViewer::cmdToggleFullScreenMode },
  { "toggleOutline",           0, gFalse, gFalse, &XPDFViewer::cmdToggleOutline },
  { "windowMode",              0, gFalse, gFalse, &XPDFViewer::cmdWindowMode },
  { "zoomFitPage",             0, gFalse, gFalse, &XPDFViewer::cmdZoomFitPage },
  { "zoomFitWidth",            0, gFalse, gFalse, &XPDFViewer::cmdZoomFitWidth },
  { "zoomIn",                  0, gFalse, gFalse, &XPDFViewer::cmdZoomIn },
  { "zoomOut",                 0, gFalse, gFalse, &XPDFViewer::cmdZoomOut },
  { "zoomPercent",             1, gFalse, gFalse, &XPDFViewer::cmdZoomPercent },
  { "zoomToSelection",         0, gTrue,  gFalse, &XPDFViewer::cmdZoomToSelection }
};

#define nCmds (sizeof(cmdTab) / sizeof(XPDFViewerCmd))

//------------------------------------------------------------------------

XPDFViewer::XPDFViewer(XPDFApp *appA, GString *fileName,
		       int pageA, GString *destName, GBool fullScreen,
		       GString *ownerPassword, GString *userPassword) {
  LinkDest *dest;
  int pg;
  double z;

  app = appA;
  win = NULL;
  core = NULL;
  ok = gFalse;
#ifndef DISABLE_OUTLINE
  outlineLabels = NULL;
  outlineLabelsLength = outlineLabelsSize = 0;
  outlinePaneWidth = 175;
#endif

  // do Motif-specific initialization and create the window;
  // this also creates the core object
  initWindow(fullScreen);
  initAboutDialog();
  initFindDialog();
  initPrintDialog();
  openDialog = NULL;
  saveAsDialog = NULL;

  dest = NULL; // make gcc happy
  pg = pageA; // make gcc happy

  if (fileName) {
    if (loadFile(fileName, ownerPassword, userPassword)) {
      getPageAndDest(pageA, destName, &pg, &dest);
#ifndef DISABLE_OUTLINE
      if (outlineScroll != None &&
	  core->getDoc()->getOutline()->getItems() &&
	  core->getDoc()->getOutline()->getItems()->getLength() > 0) {
	XtVaSetValues(outlineScroll, XmNwidth, outlinePaneWidth, NULL);
      }
#endif
    } else {
      return;
    }
  }
  core->resizeToPage(pg);

  // map the window -- we do this after calling resizeToPage to avoid
  // an annoying on-screen resize
  mapWindow();

  // display the first page
  z = core->getZoom();
  if (dest) {
    displayDest(dest, z, core->getRotate(), gTrue);
    delete dest;
  } else {
    displayPage(pg, z, core->getRotate(), gTrue, gTrue);
  }

  ok = gTrue;
}

XPDFViewer::XPDFViewer(XPDFApp *appA, PDFDoc *doc, int pageA,
		       GString *destName, GBool fullScreen) {
  LinkDest *dest;
  int pg;
  double z;

  app = appA;
  win = NULL;
  core = NULL;
  ok = gFalse;
#ifndef DISABLE_OUTLINE
  outlineLabels = NULL;
  outlineLabelsLength = outlineLabelsSize = 0;
  outlinePaneWidth = 175;
#endif

  // do Motif-specific initialization and create the window;
  // this also creates the core object
  initWindow(fullScreen);
  initAboutDialog();
  initFindDialog();
  initPrintDialog();
  openDialog = NULL;
  saveAsDialog = NULL;

  dest = NULL; // make gcc happy
  pg = pageA; // make gcc happy

  if (doc) {
    core->loadDoc(doc);
    getPageAndDest(pageA, destName, &pg, &dest);
#ifndef DISABLE_OUTLINE
    if (outlineScroll != None &&
	core->getDoc()->getOutline()->getItems() &&
	core->getDoc()->getOutline()->getItems()->getLength() > 0) {
      XtVaSetValues(outlineScroll, XmNwidth, outlinePaneWidth, NULL);
    }
#endif
  }
  core->resizeToPage(pg);

  // map the window -- we do this after calling resizeToPage to avoid
  // an annoying on-screen resize
  mapWindow();

  // display the first page
  z = core->getZoom();
  if (dest) {
    displayDest(dest, z, core->getRotate(), gTrue);
    delete dest;
  } else {
    displayPage(pg, z, core->getRotate(), gTrue, gTrue);
  }

  ok = gTrue;
}

XPDFViewer::~XPDFViewer() {
  delete core;
  XmFontListFree(aboutBigFont);
  XmFontListFree(aboutVersionFont);
  XmFontListFree(aboutFixedFont);
  closeWindow();
#ifndef DISABLE_OUTLINE
  if (outlineLabels) {
    gfree(outlineLabels);
  }
#endif
}

void XPDFViewer::open(GString *fileName, int pageA, GString *destName) {
  LinkDest *dest;
  int pg;
  double z;

  if (!core->getDoc() || fileName->cmp(core->getDoc()->getFileName())) {
    if (!loadFile(fileName, NULL, NULL)) {
      return;
    }
  }
  getPageAndDest(pageA, destName, &pg, &dest);
  z = core->getZoom();
  if (dest) {
    displayDest(dest, z, core->getRotate(), gTrue);
    delete dest;
  } else {
    displayPage(pg, z, core->getRotate(), gTrue, gTrue);
  }
}

void XPDFViewer::clear() {
  char *title;
  XmString s;

  core->clear();

  // set up title, number-of-pages display
  title = app->getTitle() ? app->getTitle()->getCString()
                          : (char *)xpdfAppName;
  XtVaSetValues(win, XmNtitle, title, XmNiconName, title, NULL);
  s = XmStringCreateLocalized("");
  XtVaSetValues(pageNumText, XmNlabelString, s, NULL);
  XmStringFree(s);
  s = XmStringCreateLocalized(" of 0");
  XtVaSetValues(pageCountLabel, XmNlabelString, s, NULL);
  XmStringFree(s);

  // disable buttons
  XtVaSetValues(prevTenPageBtn, XmNsensitive, False, NULL);
  XtVaSetValues(prevPageBtn, XmNsensitive, False, NULL);
  XtVaSetValues(nextTenPageBtn, XmNsensitive, False, NULL);
  XtVaSetValues(nextPageBtn, XmNsensitive, False, NULL);

  // remove the old outline
#ifndef DISABLE_OUTLINE
  setupOutline();
#endif
}

//------------------------------------------------------------------------
// load / display
//------------------------------------------------------------------------

GBool XPDFViewer::loadFile(GString *fileName, GString *ownerPassword,
			   GString *userPassword) {
  return core->loadFile(fileName, ownerPassword, userPassword) == errNone;
}

void XPDFViewer::reloadFile() {
  int pg;

  if (!core->getDoc()) {
    return;
  }
  pg = core->getPageNum();
  loadFile(core->getDoc()->getFileName());
  if (pg > core->getDoc()->getNumPages()) {
    pg = core->getDoc()->getNumPages();
  }
  displayPage(pg, core->getZoom(), core->getRotate(), gFalse, gFalse);
}

void XPDFViewer::displayPage(int pageA, double zoomA, int rotateA,
			     GBool scrollToTop, GBool addToHist) {
  core->displayPage(pageA, zoomA, rotateA, scrollToTop, addToHist);
}

void XPDFViewer::displayDest(LinkDest *dest, double zoomA, int rotateA,
			     GBool addToHist) {
  core->displayDest(dest, zoomA, rotateA, addToHist);
}

void XPDFViewer::getPageAndDest(int pageA, GString *destName,
				int *pageOut, LinkDest **destOut) {
  Ref pageRef;

  // find the page number for a named destination
  *pageOut = pageA;
  *destOut = NULL;
  if (destName && (*destOut = core->getDoc()->findDest(destName))) {
    if ((*destOut)->isPageRef()) {
      pageRef = (*destOut)->getPageRef();
      *pageOut = core->getDoc()->findPage(pageRef.num, pageRef.gen);
    } else {
      *pageOut = (*destOut)->getPageNum();
    }
  }

  if (*pageOut <= 0) {
    *pageOut = 1;
  }
  if (*pageOut > core->getDoc()->getNumPages()) {
    *pageOut = core->getDoc()->getNumPages();
  }
}

//------------------------------------------------------------------------
// hyperlinks / actions
//------------------------------------------------------------------------

void XPDFViewer::doLink(int wx, int wy, GBool onlyIfNoSelection,
			GBool newWin) {
  XPDFViewer *newViewer;
  LinkAction *action;
  int pg, selPg;
  double xu, yu, selULX, selULY, selLRX, selLRY;

  if (core->getHyperlinksEnabled() &&
      core->cvtWindowToUser(wx, wy, &pg, &xu, &yu) &&
      !(onlyIfNoSelection &&
	core->getSelection(&selPg, &selULX, &selULY, &selLRX, &selLRY))) {
    if ((action = core->findLink(pg, xu, yu))) {
      if (newWin &&
	  core->getDoc()->getFileName() &&
	  (action->getKind() == actionGoTo ||
	   action->getKind() == actionGoToR ||
	   (action->getKind() == actionNamed &&
	    ((LinkNamed *)action)->getName()->cmp("Quit")))) {
	newViewer = app->open(core->getDoc()->getFileName());
	newViewer->core->doAction(action);
      } else {
	core->doAction(action);
      }
    }
  }
}

void XPDFViewer::actionCbk(void *data, char *action) {
  XPDFViewer *viewer = (XPDFViewer *)data;

  if (!strcmp(action, "Quit")) {
    viewer->app->quit();
  }
}

//------------------------------------------------------------------------
// keyboard/mouse input
//------------------------------------------------------------------------

void XPDFViewer::keyPressCbk(void *data, KeySym key, Guint modifiers,
			     XEvent *event) {
  XPDFViewer *viewer = (XPDFViewer *)data;
  int keyCode;
  GList *cmds;
  int i;

  if (key >= 0x20 && key <= 0xfe) {
    keyCode = (int)key;
  } else if (key == XK_Tab ||
	     key == XK_KP_Tab) {
    keyCode = xpdfKeyCodeTab;
  } else if (key == XK_Return) {
    keyCode = xpdfKeyCodeReturn;
  } else if (key == XK_KP_Enter) {
    keyCode = xpdfKeyCodeEnter;
  } else if (key == XK_BackSpace) {
    keyCode = xpdfKeyCodeBackspace;
  } else if (key == XK_Insert ||
	     key == XK_KP_Insert) {
    keyCode = xpdfKeyCodeInsert;
  } else if (key == XK_Delete ||
	     key == XK_KP_Delete) {
    keyCode = xpdfKeyCodeDelete;
  } else if (key == XK_Home ||
	     key == XK_KP_Home) {
    keyCode = xpdfKeyCodeHome;
  } else if (key == XK_End ||
	     key == XK_KP_End) {
    keyCode = xpdfKeyCodeEnd;
  } else if (key == XK_Page_Up ||
	     key == XK_KP_Page_Up) {
    keyCode = xpdfKeyCodePgUp;
  } else if (key == XK_Page_Down ||
	     key == XK_KP_Page_Down) {
    keyCode = xpdfKeyCodePgDn;
  } else if (key == XK_Left ||
	     key == XK_KP_Left) {
    keyCode = xpdfKeyCodeLeft;
  } else if (key == XK_Right ||
	     key == XK_KP_Right) {
    keyCode = xpdfKeyCodeRight;
  } else if (key == XK_Up ||
	     key == XK_KP_Up) {
    keyCode = xpdfKeyCodeUp;
  } else if (key == XK_Down ||
	     key == XK_KP_Down) {
    keyCode = xpdfKeyCodeDown;
  } else if (key >= XK_F1 && key <= XK_F35) {
    keyCode = xpdfKeyCodeF1 + (key - XK_F1);
  } else {
    return;
  }

  if ((cmds = globalParams->getKeyBinding(keyCode,
					  viewer->getModifiers(modifiers),
					  viewer->getContext(modifiers)))) {
    for (i = 0; i < cmds->getLength(); ++i) {
      viewer->execCmd((GString *)cmds->get(i), event);
    }
    deleteGList(cmds, GString);
  }
}

void XPDFViewer::mouseCbk(void *data, XEvent *event) {
  XPDFViewer *viewer = (XPDFViewer *)data;
  int keyCode;
  GList *cmds;
  int i;

  if (event->type == ButtonPress) {
    if (event->xbutton.button >= 1 && event->xbutton.button <= 7) {
      keyCode = xpdfKeyCodeMousePress1 + event->xbutton.button - 1;
    } else {
      return;
    }
  } else if (event->type == ButtonRelease) {
    if (event->xbutton.button >= 1 && event->xbutton.button <= 7) {
      keyCode = xpdfKeyCodeMouseRelease1 + event->xbutton.button - 1;
    } else {
      return;
    }
  } else {
    return;
  }

  if ((cmds = globalParams->getKeyBinding(keyCode,
					  viewer->getModifiers(
						      event->xkey.state),
					  viewer->getContext(
						      event->xkey.state)))) {
    for (i = 0; i < cmds->getLength(); ++i) {
      viewer->execCmd((GString *)cmds->get(i), event);
    }
    deleteGList(cmds, GString);
  }
}

int XPDFViewer::getModifiers(Guint modifiers) {
  int mods;

  mods = 0;
  if (modifiers & ShiftMask) {
    mods |= xpdfKeyModShift;
  }
  if (modifiers & ControlMask) {
    mods |= xpdfKeyModCtrl;
  }
  if (modifiers & Mod1Mask) {
    mods |= xpdfKeyModAlt;
  }
  return mods;
}

int XPDFViewer::getContext(Guint modifiers) {
  int context;

  context = (core->getFullScreen() ? xpdfKeyContextFullScreen
                                   : xpdfKeyContextWindow) |
            (core->getContinuousMode() ? xpdfKeyContextContinuous
                                       : xpdfKeyContextSinglePage) |
            (core->getLinkAction() ? xpdfKeyContextOverLink
                                   : xpdfKeyContextOffLink) |
            ((modifiers & Mod5Mask) ? xpdfKeyContextScrLockOn
	                            : xpdfKeyContextScrLockOff);
  return context;
}

void XPDFViewer::execCmd(GString *cmd, XEvent *event) {
  GString *name;
  GString *args[cmdMaxArgs];
  char *p0, *p1;
  int nArgs, i;
  int a, b, m, cmp;

  //----- parse the command
  name = NULL;
  nArgs = 0;
  for (i = 0; i < cmdMaxArgs; ++i) {
    args[i] = NULL;
  }
  p0 = cmd->getCString();
  for (p1 = p0; *p1 && isalnum(*p1); ++p1) ;
  if (p1 == p0) {
    goto err1;
  }
  name = new GString(p0, p1 - p0);
  if (*p1 == '(') {
    while (nArgs < cmdMaxArgs) {
      p0 = p1 + 1;
      for (p1 = p0; *p1 && *p1 != ',' && *p1 != ')'; ++p1) ;
      args[nArgs++] = new GString(p0, p1 - p0);
      if (*p1 != ',') {
	break;
      }
    }
    if (*p1 != ')') {
      goto err1;
    }
    ++p1;
  }
  if (*p1) {
    goto err1;
  }
  
  //----- find the command
  a = -1;
  b = nCmds;
  // invariant: cmdTab[a].name < name < cmdTab[b].name
  while (b - a > 1) {
    m = (a + b) / 2;
    cmp = strcmp(cmdTab[m].name, name->getCString());
    if (cmp < 0) {
      a = m;
    } else if (cmp > 0) {
      b = m;
    } else {
      a = b = m;
    }
  }
  if (cmp != 0) {
    goto err1;
  }

  //----- execute the command
  if (nArgs != cmdTab[a].nArgs ||
      (cmdTab[a].requiresEvent && !event)) {
    goto err1;
  }
  if (cmdTab[a].requiresDoc && !core->getDoc()) {
    // don't issue an error message for this -- it happens, e.g., when
    // clicking in a window with no open PDF file
    goto err2;
  }
  (this->*cmdTab[a].func)(args, nArgs, event);

  //----- clean up
  delete name;
  for (i = 0; i < nArgs; ++i) {
    if (args[i]) {
      delete args[i];
    }
  }
  return;

 err1:
  error(-1, "Invalid command syntax: '%s'", cmd->getCString());
 err2:
  if (name) {
    delete name;
  }
  for (i = 0; i < nArgs; ++i) {
    if (args[i]) {
      delete args[i];
    }
  }
}

//------------------------------------------------------------------------
// command functions
//------------------------------------------------------------------------

static int mouseX(XEvent *event) {
  switch (event->type) {
  case ButtonPress:
  case ButtonRelease:
    return event->xbutton.x;
  case KeyPress:
    return event->xkey.x;
  }
  return 0;
}

static int mouseY(XEvent *event) {
  switch (event->type) {
  case ButtonPress:
  case ButtonRelease:
    return event->xbutton.y;
  case KeyPress:
    return event->xkey.y;
  }
  return 0;
}

void XPDFViewer::cmdAbout(GString *args[], int nArgs,
			  XEvent *event) {
  XtManageChild(aboutDialog);
}

void XPDFViewer::cmdCloseOutline(GString *args[], int nArgs,
				 XEvent *event) {
#ifndef DISABLE_OUTLINE
  Dimension w;

  if (outlineScroll == None) {
    return;
  }
  XtVaGetValues(outlineScroll, XmNwidth, &w, NULL);
  if (w > 1) {
    outlinePaneWidth = w;
    // this ugly kludge is apparently the only way to resize the panes
    // within an XmPanedWindow
    XtVaSetValues(outlineScroll, XmNpaneMinimum, 1,
		  XmNpaneMaximum, 1, NULL);
    XtVaSetValues(outlineScroll, XmNpaneMinimum, 1,
		  XmNpaneMaximum, 10000, NULL);
  }
#endif
}

void XPDFViewer::cmdCloseWindow(GString *args[], int nArgs,
				XEvent *event) {
  app->close(this, gFalse);
}

void XPDFViewer::cmdContinuousMode(GString *args[], int nArgs,
				   XEvent *event) {
  Widget btn;

  if (core->getContinuousMode()) {
    return;
  }
  core->setContinuousMode(gTrue);

  btn = XtNameToWidget(popupMenu, "continuousMode");
  XtVaSetValues(btn, XmNset, XmSET, NULL);
}

void XPDFViewer::cmdEndPan(GString *args[], int nArgs,
			   XEvent *event) {
  core->endPan(mouseX(event), mouseY(event));
}

void XPDFViewer::cmdEndSelection(GString *args[], int nArgs,
				 XEvent *event) {
  core->endSelection(mouseX(event), mouseY(event));
}

void XPDFViewer::cmdFind(GString *args[], int nArgs,
			 XEvent *event) {
  mapFindDialog();
}

void XPDFViewer::cmdFindNext(GString *args[], int nArgs,
			     XEvent *event) {
  doFind(gTrue);
}

void XPDFViewer::cmdFocusToDocWin(GString *args[], int nArgs,
				  XEvent *event) {
  core->takeFocus();
}

void XPDFViewer::cmdFocusToPageNum(GString *args[], int nArgs,
				   XEvent *event) {
  XmTextFieldSetSelection(pageNumText, 0,
			  strlen(XmTextFieldGetString(pageNumText)),
			  XtLastTimestampProcessed(display));
  XmProcessTraversal(pageNumText, XmTRAVERSE_CURRENT);
}

void XPDFViewer::cmdFollowLink(GString *args[], int nArgs,
			       XEvent *event) {
  doLink(mouseX(event), mouseY(event), gFalse, gFalse);
}

void XPDFViewer::cmdFollowLinkInNewWin(GString *args[], int nArgs,
				       XEvent *event) {
  doLink(mouseX(event), mouseY(event), gFalse, gTrue);
}

void XPDFViewer::cmdFollowLinkInNewWinNoSel(GString *args[], int nArgs,
					    XEvent *event) {
  doLink(mouseX(event), mouseY(event), gTrue, gTrue);
}

void XPDFViewer::cmdFollowLinkNoSel(GString *args[], int nArgs,
				    XEvent *event) {
  doLink(mouseX(event), mouseY(event), gTrue, gFalse);
}

void XPDFViewer::cmdFullScreenMode(GString *args[], int nArgs,
				   XEvent *event) {
  PDFDoc *doc;
  XPDFViewer *viewer;
  int pg;
  Widget btn;

  if (core->getFullScreen()) {
    return;
  }
  pg = core->getPageNum();
  XtPopdown(win);
  doc = core->takeDoc(gFalse);
  viewer = app->reopen(this, doc, pg, gTrue);

  btn = XtNameToWidget(viewer->popupMenu, "fullScreen");
  XtVaSetValues(btn, XmNset, XmSET, NULL);
}

void XPDFViewer::cmdGoBackward(GString *args[], int nArgs,
			       XEvent *event) {
  core->goBackward();
}

void XPDFViewer::cmdGoForward(GString *args[], int nArgs,
			      XEvent *event) {
  core->goForward();
}

void XPDFViewer::cmdGotoDest(GString *args[], int nArgs,
			     XEvent *event) {
  int pg;
  LinkDest *dest;

  getPageAndDest(1, args[0], &pg, &dest);
  if (dest) {
    displayDest(dest, core->getZoom(), core->getRotate(), gTrue);
    delete dest;
  }
}

void XPDFViewer::cmdGotoLastPage(GString *args[], int nArgs,
				 XEvent *event) {
  displayPage(core->getDoc()->getNumPages(),
	      core->getZoom(), core->getRotate(),
	      gTrue, gTrue);
}

void XPDFViewer::cmdGotoLastPageNoScroll(GString *args[], int nArgs,
					 XEvent *event) {
  displayPage(core->getDoc()->getNumPages(),
	      core->getZoom(), core->getRotate(),
	      gFalse, gTrue);
}

void XPDFViewer::cmdGotoPage(GString *args[], int nArgs,
			     XEvent *event) {
  int pg;

  pg = atoi(args[0]->getCString());
  if (pg < 1 || pg > core->getDoc()->getNumPages()) {
    return;
  }
  displayPage(pg, core->getZoom(), core->getRotate(), gTrue, gTrue);
}

void XPDFViewer::cmdGotoPageNoScroll(GString *args[], int nArgs,
				     XEvent *event) {
  int pg;

  pg = atoi(args[0]->getCString());
  if (pg < 1 || pg > core->getDoc()->getNumPages()) {
    return;
  }
  displayPage(pg, core->getZoom(), core->getRotate(), gFalse, gTrue);
}

void XPDFViewer::cmdNextPage(GString *args[], int nArgs,
			     XEvent *event) {
  core->gotoNextPage(1, gTrue);
}

void XPDFViewer::cmdNextPageNoScroll(GString *args[], int nArgs,
				     XEvent *event) {
  core->gotoNextPage(1, gFalse);
}

void XPDFViewer::cmdOpen(GString *args[], int nArgs,
			 XEvent *event) {
  mapOpenDialog(gFalse);
}

void XPDFViewer::cmdOpenFile(GString *args[], int nArgs,
			     XEvent *event) {
  open(args[0], 1, NULL);
}

void XPDFViewer::cmdOpenFileAtDest(GString *args[], int nArgs,
				   XEvent *event) {
  open(args[0], 1, args[1]);
}

void XPDFViewer::cmdOpenFileAtDestInNewWin(GString *args[], int nArgs,
					   XEvent *event) {
  app->openAtDest(args[0], args[1]);
}

void XPDFViewer::cmdOpenFileAtPage(GString *args[], int nArgs,
				   XEvent *event) {
  open(args[0], atoi(args[1]->getCString()), NULL);
}

void XPDFViewer::cmdOpenFileAtPageInNewWin(GString *args[], int nArgs,
					   XEvent *event) {
  app->open(args[0], atoi(args[1]->getCString()));
}

void XPDFViewer::cmdOpenFileInNewWin(GString *args[], int nArgs,
				     XEvent *event) {
  app->open(args[0]);
}

void XPDFViewer::cmdOpenInNewWin(GString *args[], int nArgs,
				 XEvent *event) {
  mapOpenDialog(gTrue);
}

void XPDFViewer::cmdOpenOutline(GString *args[], int nArgs,
				XEvent *event) {
#ifndef DISABLE_OUTLINE
  Dimension w;

  if (outlineScroll == None) {
    return;
  }
  XtVaGetValues(outlineScroll, XmNwidth, &w, NULL);
  if (w == 1) {
    // this ugly kludge is apparently the only way to resize the panes
    // within an XmPanedWindow
    XtVaSetValues(outlineScroll, XmNpaneMinimum, outlinePaneWidth,
		  XmNpaneMaximum, outlinePaneWidth, NULL);
    XtVaSetValues(outlineScroll, XmNpaneMinimum, 1,
		  XmNpaneMaximum, 10000, NULL);
  }
#endif
}

void XPDFViewer::cmdPageDown(GString *args[], int nArgs,
			     XEvent *event) {
  core->scrollPageDown();
}

void XPDFViewer::cmdPageUp(GString *args[], int nArgs,
			   XEvent *event) {
  core->scrollPageUp();
}

void XPDFViewer::cmdPostPopupMenu(GString *args[], int nArgs,
				  XEvent *event) {
  XmMenuPosition(popupMenu, event->type == ButtonPress ? &event->xbutton
		                                       : (XButtonEvent *)NULL);
  XtManageChild(popupMenu);

  // this is magic (taken from DDD) - weird things happen if this
  // call isn't made (this is done in two different places, in hopes
  // of squashing this stupid bug)
  XtUngrabButton(core->getDrawAreaWidget(), AnyButton, AnyModifier);
}

void XPDFViewer::cmdPrevPage(GString *args[], int nArgs,
			     XEvent *event) {
  core->gotoPrevPage(1, gTrue, gFalse);
}

void XPDFViewer::cmdPrevPageNoScroll(GString *args[], int nArgs,
				     XEvent *event) {
  core->gotoPrevPage(1, gFalse, gFalse);
}

void XPDFViewer::cmdPrint(GString *args[], int nArgs,
			  XEvent *event) {
  XtManageChild(printDialog);
}

void XPDFViewer::cmdQuit(GString *args[], int nArgs,
			 XEvent *event) {
  app->quit();
}

void XPDFViewer::cmdRaise(GString *args[], int nArgs,
			  XEvent *event) {
  XMapRaised(display, XtWindow(win));
  XFlush(display);
}

void XPDFViewer::cmdRedraw(GString *args[], int nArgs,
			   XEvent *event) {
  displayPage(core->getPageNum(), core->getZoom(), core->getRotate(),
	      gFalse, gFalse);
}

void XPDFViewer::cmdReload(GString *args[], int nArgs,
			   XEvent *event) {
  reloadFile();
}

void XPDFViewer::cmdRun(GString *args[], int nArgs,
			XEvent *event) {
  GString *fmt, *cmd, *s;
  LinkAction *action;
  double selLRX, selLRY, selURX, selURY;
  int selPage;
  GBool gotSel;
  char buf[64];
  char *p;
  char c0, c1;
  int i;

  cmd = new GString();
  fmt = args[0];
  i = 0;
  gotSel = gFalse;
  while (i < fmt->getLength()) {
    c0 = fmt->getChar(i);
    if (c0 == '%' && i+1 < fmt->getLength()) {
      c1 = fmt->getChar(i+1);
      switch (c1) {
      case 'f':
	if (core->getDoc() && (s = core->getDoc()->getFileName())) {
	  cmd->append(s);
	}
	break;
      case 'b':
	if (core->getDoc() && (s = core->getDoc()->getFileName())) {
	  if ((p = strrchr(s->getCString(), '.'))) {
	    cmd->append(s->getCString(), p - s->getCString());
	  } else {
	    cmd->append(s);
	  }
	}
	break;
      case 'u':
	if ((action = core->getLinkAction()) &&
	    action->getKind() == actionURI) {
	  s = core->mungeURL(((LinkURI *)action)->getURI());
	  cmd->append(s);
	  delete s;
	}
	break;
      case 'x':
      case 'y':
      case 'X':
      case 'Y':
	if (!gotSel) {
	  if (!core->getSelection(&selPage, &selURX, &selURY,
				  &selLRX, &selLRY)) {
	    selPage = 0;
	    selURX = selURY = selLRX = selLRY = 0;
	  }
	  gotSel = gTrue;
	}
	sprintf(buf, "%g",
		(c1 == 'x') ? selURX :
		(c1 == 'y') ? selURY :
		(c1 == 'X') ? selLRX : selLRY);
	cmd->append(buf);
	break;
      default:
	cmd->append(c1);
	break;
      }
      i += 2;
    } else {
      cmd->append(c0);
      ++i;
    }
  }
#ifdef VMS
  cmd->insert(0, "spawn/nowait ");
#elif defined(__EMX__)
  cmd->insert(0, "start /min /n ");
#else
  cmd->append(" &");
#endif
  system(cmd->getCString());
  delete cmd;
}

void XPDFViewer::cmdScrollDown(GString *args[], int nArgs,
			       XEvent *event) {
  core->scrollDown(atoi(args[0]->getCString()));
}

void XPDFViewer::cmdScrollDownNextPage(GString *args[], int nArgs,
				       XEvent *event) {
  core->scrollDownNextPage(atoi(args[0]->getCString()));
}

void XPDFViewer::cmdScrollLeft(GString *args[], int nArgs,
			       XEvent *event) {
  core->scrollLeft(atoi(args[0]->getCString()));
}

void XPDFViewer::cmdScrollOutlineDown(GString *args[], int nArgs,
				      XEvent *event) {
#ifndef DISABLE_OUTLINE
  Widget sb;
  int val, inc, pageInc, m, slider;

  if (outlineScroll == None) {
    return;
  }
  if ((sb = XtNameToWidget(outlineScroll, "VertScrollBar"))) {
    XtVaGetValues(sb, XmNvalue, &val, XmNincrement, &inc,
		  XmNpageIncrement, &pageInc, XmNmaximum, &m,
		  XmNsliderSize, &slider, NULL);
    if ((val += inc * atoi(args[0]->getCString())) > m - slider) {
      val = m - slider;
    }
    XmScrollBarSetValues(sb, val, slider, inc, pageInc, True);
  }
#endif
}

void XPDFViewer::cmdScrollOutlineUp(GString *args[], int nArgs,
				    XEvent *event) {
#ifndef DISABLE_OUTLINE
  Widget sb;
  int val, inc, pageInc, m, slider;

  if (outlineScroll == None) {
    return;
  }
  if ((sb = XtNameToWidget(outlineScroll, "VertScrollBar"))) {
    XtVaGetValues(sb, XmNvalue, &val, XmNincrement, &inc,
		  XmNpageIncrement, &pageInc, XmNminimum, &m,
		  XmNsliderSize, &slider, NULL);
    if ((val -= inc * atoi(args[0]->getCString())) < m) {
      val = m;
    }
    XmScrollBarSetValues(sb, val, slider, inc, pageInc, True);
  }
#endif
}

void XPDFViewer::cmdScrollRight(GString *args[], int nArgs,
				XEvent *event) {
  core->scrollRight(atoi(args[0]->getCString()));
}

void XPDFViewer::cmdScrollToBottomEdge(GString *args[], int nArgs,
				       XEvent *event) {
  core->scrollToBottomEdge();
}

void XPDFViewer::cmdScrollToBottomRight(GString *args[], int nArgs,
					XEvent *event) {
  core->scrollToBottomRight();
}

void XPDFViewer::cmdScrollToLeftEdge(GString *args[], int nArgs,
				     XEvent *event) {
  core->scrollToLeftEdge();
}

void XPDFViewer::cmdScrollToRightEdge(GString *args[], int nArgs,
				      XEvent *event) {
  core->scrollToRightEdge();
}

void XPDFViewer::cmdScrollToTopEdge(GString *args[], int nArgs,
				    XEvent *event) {
  core->scrollToTopEdge();
}

void XPDFViewer::cmdScrollToTopLeft(GString *args[], int nArgs,
				    XEvent *event) {
  core->scrollToTopLeft();
}

void XPDFViewer::cmdScrollUp(GString *args[], int nArgs,
			     XEvent *event) {
  core->scrollUp(atoi(args[0]->getCString()));
}

void XPDFViewer::cmdScrollUpPrevPage(GString *args[], int nArgs,
				     XEvent *event) {
  core->scrollUpPrevPage(atoi(args[0]->getCString()));
}

void XPDFViewer::cmdSinglePageMode(GString *args[], int nArgs,
				   XEvent *event) {
  Widget btn;

  if (!core->getContinuousMode()) {
    return;
  }
  core->setContinuousMode(gFalse);

  btn = XtNameToWidget(popupMenu, "continuousMode");
  XtVaSetValues(btn, XmNset, XmUNSET, NULL);
}

void XPDFViewer::cmdStartPan(GString *args[], int nArgs,
			     XEvent *event) {
  core->startPan(mouseX(event), mouseY(event));
}

void XPDFViewer::cmdStartSelection(GString *args[], int nArgs,
				   XEvent *event) {
  core->startSelection(mouseX(event), mouseY(event));
}

void XPDFViewer::cmdToggleContinuousMode(GString *args[], int nArgs,
					 XEvent *event) {
  if (core->getContinuousMode()) {
    cmdSinglePageMode(NULL, 0, event);
  } else {
    cmdContinuousMode(NULL, 0, event);
  }
}

void XPDFViewer::cmdToggleFullScreenMode(GString *args[], int nArgs,
					 XEvent *event) {
  if (core->getFullScreen()) {
    cmdWindowMode(NULL, 0, event);
  } else {
    cmdFullScreenMode(NULL, 0, event);
  }
}

void XPDFViewer::cmdToggleOutline(GString *args[], int nArgs,
				  XEvent *event) {
#ifndef DISABLE_OUTLINE
  Dimension w;

  if (outlineScroll == None) {
    return;
  }
  XtVaGetValues(outlineScroll, XmNwidth, &w, NULL);
  if (w > 1) {
    cmdCloseOutline(NULL, 0, event);
  } else {
    cmdOpenOutline(NULL, 0, event);
  }
#endif
}

void XPDFViewer::cmdWindowMode(GString *args[], int nArgs,
			       XEvent *event) {
  PDFDoc *doc;
  XPDFViewer *viewer;
  int pg;
  Widget btn;

  if (!core->getFullScreen()) {
    return;
  }
  pg = core->getPageNum();
  XtPopdown(win);
  doc = core->takeDoc(gFalse);
  viewer = app->reopen(this, doc, pg, gFalse);

  btn = XtNameToWidget(viewer->popupMenu, "fullScreen");
  XtVaSetValues(btn, XmNset, XmUNSET, NULL);
}

void XPDFViewer::cmdZoomFitPage(GString *args[], int nArgs,
				XEvent *event) {
  if (core->getZoom() != zoomPage) {
    setZoomIdx(zoomPageIdx);
    displayPage(core->getPageNum(), zoomPage,
		core->getRotate(), gTrue, gFalse);
  }
}

void XPDFViewer::cmdZoomFitWidth(GString *args[], int nArgs,
				 XEvent *event) {
  if (core->getZoom() != zoomWidth) {
    setZoomIdx(zoomWidthIdx);
    displayPage(core->getPageNum(), zoomWidth,
		core->getRotate(), gTrue, gFalse);
  }
}

void XPDFViewer::cmdZoomIn(GString *args[], int nArgs,
			   XEvent *event) {
  int z;

  z = getZoomIdx();
  if (z <= minZoomIdx && z > maxZoomIdx) {
    --z;
    setZoomIdx(z);
    displayPage(core->getPageNum(), zoomMenuInfo[z].zoom,
		core->getRotate(), gTrue, gFalse);
  }
}

void XPDFViewer::cmdZoomOut(GString *args[], int nArgs,
			    XEvent *event) {
  int z;

  z = getZoomIdx();
  if (z < minZoomIdx && z >= maxZoomIdx) {
    ++z;
    setZoomIdx(z);
    displayPage(core->getPageNum(), zoomMenuInfo[z].zoom,
		core->getRotate(), gTrue, gFalse);
  }
}

void XPDFViewer::cmdZoomPercent(GString *args[], int nArgs,
				XEvent *event) {
  double z;

  z = atof(args[0]->getCString());
  setZoomVal(z);
  displayPage(core->getPageNum(), z, core->getRotate(), gTrue, gFalse);
}

void XPDFViewer::cmdZoomToSelection(GString *args[], int nArgs,
				    XEvent *event) {
  int pg;
  double ulx, uly, lrx, lry;

  if (core->getSelection(&pg, &ulx, &uly, &lrx, &lry)) {
    core->zoomToRect(pg, ulx, uly, lrx, lry);
  }
}

//------------------------------------------------------------------------
// GUI code: main window
//------------------------------------------------------------------------

void XPDFViewer::initWindow(GBool fullScreen) {
  Colormap colormap;
  XColor xcol;
  Atom state, val;
  Arg args[20];
  int n;
  char *title;

  display = XtDisplay(app->getAppShell());
  screenNum = XScreenNumberOfScreen(XtScreen(app->getAppShell()));

  toolBar = None;
#ifndef DISABLE_OUTLINE
  outlineScroll = None;
#endif

  // private colormap
  if (app->getInstallCmap()) {
    XtVaGetValues(app->getAppShell(), XmNcolormap, &colormap, NULL);
    // ensure that BlackPixel and WhitePixel are reserved in the
    // new colormap
    xcol.red = xcol.green = xcol.blue = 0;
    XAllocColor(display, colormap, &xcol);
    xcol.red = xcol.green = xcol.blue = 65535;
    XAllocColor(display, colormap, &xcol);
    colormap = XCopyColormapAndFree(display, colormap);
  }

  // top-level window
  n = 0;
  title = app->getTitle() ? app->getTitle()->getCString()
                          : (char *)xpdfAppName;
  XtSetArg(args[n], XmNtitle, title); ++n;
  XtSetArg(args[n], XmNiconName, title); ++n;
  XtSetArg(args[n], XmNminWidth, 100); ++n;
  XtSetArg(args[n], XmNminHeight, 100); ++n;
  XtSetArg(args[n], XmNbaseWidth, 0); ++n;
  XtSetArg(args[n], XmNbaseHeight, 0); ++n;
  XtSetArg(args[n], XmNdeleteResponse, XmDO_NOTHING); ++n;
  win = XtCreatePopupShell("win", topLevelShellWidgetClass,
			   app->getAppShell(), args, n);
  if (app->getInstallCmap()) {
    XtVaSetValues(win, XmNcolormap, colormap, NULL);
  }
  XmAddWMProtocolCallback(win, XInternAtom(display, "WM_DELETE_WINDOW", False),
			  &closeMsgCbk, this);

  // create the full-screen window
  if (fullScreen) {
    initCore(win, gTrue);

  // create the normal (non-full-screen) window
  } else {
    if (app->getGeometry()) {
      n = 0;
      XtSetArg(args[n], XmNgeometry, app->getGeometry()->getCString()); ++n;
      XtSetValues(win, args, n);
    }

    n = 0;
    form = XmCreateForm(win, "form", args, n);
    XtManageChild(form);

#ifdef DISABLE_OUTLINE
    initToolbar(form);
    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
    XtSetValues(toolBar, args, n);

    initCore(form, gFalse);
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); ++n;
    XtSetArg(args[n], XmNbottomWidget, toolBar); ++n;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
    XtSetValues(core->getWidget(), args, n);
#else
    initToolbar(form);
    n = 0;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
    XtSetValues(toolBar, args, n);

    initPanedWin(form);
    n = 0;
    XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); ++n;
    XtSetArg(args[n], XmNbottomWidget, toolBar); ++n;
    XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
    XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
    XtSetValues(panedWin, args, n);

    initCore(panedWin, fullScreen);
    n = 0;
    XtSetArg(args[n], XmNpositionIndex, 1); ++n;
    XtSetArg(args[n], XmNallowResize, True); ++n;
    XtSetArg(args[n], XmNpaneMinimum, 1); ++n;
    XtSetArg(args[n], XmNpaneMaximum, 10000); ++n;
    XtSetValues(core->getWidget(), args, n);
#endif
  }

  // set the zoom menu to match the initial zoom setting
  setZoomVal(core->getZoom());

  // set traversal order
  XtVaSetValues(core->getDrawAreaWidget(),
		XmNnavigationType, XmEXCLUSIVE_TAB_GROUP, NULL);
  if (toolBar != None) {
    XtVaSetValues(backBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(prevTenPageBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(prevPageBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(nextPageBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(nextTenPageBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(forwardBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(pageNumText, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(zoomWidget, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(findBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(printBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(aboutBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
    XtVaSetValues(quitBtn, XmNnavigationType, XmEXCLUSIVE_TAB_GROUP,
		  NULL);
  }

  initPopupMenu();

  if (fullScreen) {
    // Set both the old-style Motif decorations hint and the new-style
    // _NET_WM_STATE property.  This is redundant, but might be useful
    // for older window managers.  We also set the geometry to +0+0 to
    // avoid interactive placement.  (Note: we need to realize the
    // shell, so it has a Window on which to set the _NET_WM_STATE
    // property, but we don't want to map it until later, so we set
    // mappedWhenManaged to false.)
    n = 0;
    XtSetArg(args[n], XmNmappedWhenManaged, False); ++n;
    XtSetArg(args[n], XmNmwmDecorations, 0); ++n;
    XtSetArg(args[n], XmNgeometry, "+0+0"); ++n;
    XtSetValues(win, args, n);
    XtRealizeWidget(win);
    state = XInternAtom(display, "_NET_WM_STATE", False);
    val = XInternAtom(display, "_NET_WM_STATE_FULLSCREEN", False);
    XChangeProperty(display, XtWindow(win), state, XA_ATOM, 32,
		    PropModeReplace, (Guchar *)&val, 1);
  }
}

void XPDFViewer::initToolbar(Widget parent) {
  Widget label, lastBtn;
#ifndef USE_COMBO_BOX
  Widget btn;
#endif
  Arg args[20];
  int n;
  XmString s, emptyString;
  int i;

  // toolbar
  n = 0;
  toolBar = XmCreateForm(parent, "toolBar", args, n);
  XtManageChild(toolBar);

  // create an empty string -- this is used for buttons that will get
  // pixmaps later
  emptyString = XmStringCreateLocalized("");

  // page movement buttons
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  XtSetArg(args[n], XmNsensitive, False); ++n;
  XtSetArg(args[n], XmNlabelString, emptyString); ++n;
  backBtn = XmCreatePushButton(toolBar, "back", args, n);
  addToolTip(backBtn, "Back");
  XtManageChild(backBtn);
  XtAddCallback(backBtn, XmNactivateCallback,
		&backCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, backBtn); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  XtSetArg(args[n], XmNsensitive, False); ++n;
  XtSetArg(args[n], XmNlabelString, emptyString); ++n;
  prevTenPageBtn = XmCreatePushButton(toolBar, "prevTenPage", args, n);
  addToolTip(prevTenPageBtn, "-10 pages");
  XtManageChild(prevTenPageBtn);
  XtAddCallback(prevTenPageBtn, XmNactivateCallback,
		&prevTenPageCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, prevTenPageBtn); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  XtSetArg(args[n], XmNsensitive, False); ++n;
  XtSetArg(args[n], XmNlabelString, emptyString); ++n;
  prevPageBtn = XmCreatePushButton(toolBar, "prevPage", args, n);
  addToolTip(prevPageBtn, "Previous page");
  XtManageChild(prevPageBtn);
  XtAddCallback(prevPageBtn, XmNactivateCallback,
		&prevPageCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, prevPageBtn); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  XtSetArg(args[n], XmNsensitive, False); ++n;
  XtSetArg(args[n], XmNlabelString, emptyString); ++n;
  nextPageBtn = XmCreatePushButton(toolBar, "nextPage", args, n);
  addToolTip(nextPageBtn, "Next page");
  XtManageChild(nextPageBtn);
  XtAddCallback(nextPageBtn, XmNactivateCallback,
		&nextPageCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, nextPageBtn); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  XtSetArg(args[n], XmNsensitive, False); ++n;
  XtSetArg(args[n], XmNlabelString, emptyString); ++n;
  nextTenPageBtn = XmCreatePushButton(toolBar, "nextTenPage", args, n);
  addToolTip(nextTenPageBtn, "+10 pages");
  XtManageChild(nextTenPageBtn);
  XtAddCallback(nextTenPageBtn, XmNactivateCallback,
		&nextTenPageCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, nextTenPageBtn); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  XtSetArg(args[n], XmNsensitive, False); ++n;
  XtSetArg(args[n], XmNlabelString, emptyString); ++n;
  forwardBtn = XmCreatePushButton(toolBar, "forward", args, n);
  addToolTip(forwardBtn, "Forward");
  XtManageChild(forwardBtn);
  XtAddCallback(forwardBtn, XmNactivateCallback,
		&forwardCbk, (XtPointer)this);

  // page number display
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, forwardBtn); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  s = XmStringCreateLocalized("Page ");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  label = XmCreateLabel(toolBar, "pageLabel", args, n);
  XmStringFree(s);
  XtManageChild(label);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, label); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 3); ++n;
  XtSetArg(args[n], XmNmarginHeight, 3); ++n;
  XtSetArg(args[n], XmNcolumns, 5); ++n;
  pageNumText = XmCreateTextField(toolBar, "pageNum", args, n);
  XtManageChild(pageNumText);
  XtAddCallback(pageNumText, XmNactivateCallback,
		&pageNumCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, pageNumText); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  s = XmStringCreateLocalized(" of 00000");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
  XtSetArg(args[n], XmNrecomputeSize, False); ++n;
  pageCountLabel = XmCreateLabel(toolBar, "pageCountLabel", args, n);
  XmStringFree(s);
  XtManageChild(pageCountLabel);
  s = XmStringCreateLocalized(" of 0");
  XtVaSetValues(pageCountLabel, XmNlabelString, s, NULL);
  XmStringFree(s);

  // zoom menu
#if USE_COMBO_BOX
  XmString st[nZoomMenuItems];
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, pageCountLabel); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 0); ++n;
  XtSetArg(args[n], XmNmarginHeight, 0); ++n;
  XtSetArg(args[n], XmNcomboBoxType, XmDROP_DOWN_COMBO_BOX); ++n;
  XtSetArg(args[n], XmNpositionMode, XmONE_BASED); ++n;
  XtSetArg(args[n], XmNcolumns, 7); ++n;
  for (i = 0; i < nZoomMenuItems; ++i) {
    st[i] = XmStringCreateLocalized(zoomMenuInfo[i].label);
  }
  XtSetArg(args[n], XmNitems, st); ++n;
  XtSetArg(args[n], XmNitemCount, nZoomMenuItems); ++n;
  zoomComboBox = XmCreateComboBox(toolBar, "zoomComboBox", args, n);
  for (i = 0; i < nZoomMenuItems; ++i) {
    XmStringFree(st[i]);
  }
  addToolTip(zoomComboBox, "Zoom");
  XtAddCallback(zoomComboBox, XmNselectionCallback,
		&zoomComboBoxCbk, (XtPointer)this);
  XtManageChild(zoomComboBox);
  zoomWidget = zoomComboBox;
#else
  Widget menuPane;
  char buf[16];
  n = 0;
  menuPane = XmCreatePulldownMenu(toolBar, "zoomMenuPane", args, n);
  for (i = 0; i < nZoomMenuItems; ++i) {
    n = 0;
    s = XmStringCreateLocalized(zoomMenuInfo[i].label);
    XtSetArg(args[n], XmNlabelString, s); ++n;
    XtSetArg(args[n], XmNuserData, (XtPointer)i); ++n;
    sprintf(buf, "zoom%d", i);
    btn = XmCreatePushButton(menuPane, buf, args, n);
    XmStringFree(s);
    XtManageChild(btn);
    XtAddCallback(btn, XmNactivateCallback,
		  &zoomMenuCbk, (XtPointer)this);
    zoomMenuBtns[i] = btn;
  }
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, pageCountLabel); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 0); ++n;
  XtSetArg(args[n], XmNmarginHeight, 0); ++n;
  XtSetArg(args[n], XmNsubMenuId, menuPane); ++n;
  zoomMenu = XmCreateOptionMenu(toolBar, "zoomMenu", args, n);
  addToolTip(zoomMenu, "Zoom");
  XtManageChild(zoomMenu);
  zoomWidget = zoomMenu;
#endif

  // find/print/about buttons
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, zoomWidget); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  XtSetArg(args[n], XmNlabelString, emptyString); ++n;
  findBtn = XmCreatePushButton(toolBar, "find", args, n);
  addToolTip(findBtn, "Find");
  XtManageChild(findBtn);
  XtAddCallback(findBtn, XmNactivateCallback,
		&findCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, findBtn); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  XtSetArg(args[n], XmNlabelString, emptyString); ++n;
  printBtn = XmCreatePushButton(toolBar, "print", args, n);
  addToolTip(printBtn, "Print");
  XtManageChild(printBtn);
  XtAddCallback(printBtn, XmNactivateCallback,
		&printCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, printBtn); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  XtSetArg(args[n], XmNlabelString, emptyString); ++n;
  aboutBtn = XmCreatePushButton(toolBar, "about", args, n);
  addToolTip(aboutBtn, "About / help");
  XtManageChild(aboutBtn);
  XtAddCallback(aboutBtn, XmNactivateCallback,
		&aboutCbk, (XtPointer)this);
  lastBtn = aboutBtn;

  // quit button
  n = 0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNmarginWidth, 6); ++n;
  s = XmStringCreateLocalized("Quit");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  quitBtn = XmCreatePushButton(toolBar, "quit", args, n);
  XmStringFree(s);
  XtManageChild(quitBtn);
  XtAddCallback(quitBtn, XmNactivateCallback,
		&quitCbk, (XtPointer)this);

  // link label
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, lastBtn); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNrightWidget, quitBtn); ++n;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  s = XmStringCreateLocalized("");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  XtSetArg(args[n], XmNrecomputeSize, True); ++n;
  XtSetArg(args[n], XmNalignment, XmALIGNMENT_BEGINNING); ++n;
  linkLabel = XmCreateLabel(toolBar, "linkLabel", args, n);
  XmStringFree(s);
  XtManageChild(linkLabel);

  XmStringFree(emptyString);
}

#ifndef DISABLE_OUTLINE
void XPDFViewer::initPanedWin(Widget parent) {
  Widget clipWin;
  Arg args[20];
  int n;

  // paned window
  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
#if defined(__sgi) && (XmVERSION <= 1)
  panedWin = SgCreateHorzPanedWindow(parent, "panedWin", args, n);
#else
  panedWin = XmCreatePanedWindow(parent, "panedWin", args, n);
#endif
  XtManageChild(panedWin);

  // scrolled window for outline container
  n = 0;
  XtSetArg(args[n], XmNpositionIndex, 0); ++n;
  XtSetArg(args[n], XmNallowResize, True); ++n;
  XtSetArg(args[n], XmNpaneMinimum, 1); ++n;
  XtSetArg(args[n], XmNpaneMaximum, 10000); ++n;
#if !(defined(__sgi) && (XmVERSION <= 1))
  XtSetArg(args[n], XmNwidth, 1); ++n;
#endif
  XtSetArg(args[n], XmNscrollingPolicy, XmAUTOMATIC); ++n;
  outlineScroll = XmCreateScrolledWindow(panedWin, "outlineScroll", args, n);
  XtManageChild(outlineScroll);
  XtVaGetValues(outlineScroll, XmNclipWindow, &clipWin, NULL);
  XtVaSetValues(clipWin, XmNbackground, app->getPaperPixel(), NULL);

  // outline tree
  n = 0;
  XtSetArg(args[n], XmNbackground, app->getPaperPixel()); ++n;
  outlineTree = XPDFCreateTree(outlineScroll, "outlineTree", args, n);
  XtManageChild(outlineTree);
  XtAddCallback(outlineTree, XPDFNselectionCallback, &outlineSelectCbk,
		(XtPointer)this);
}
#endif

void XPDFViewer::initCore(Widget parent, GBool fullScreen) {
  core = new XPDFCore(win, parent,
		      app->getPaperRGB(), app->getPaperPixel(),
		      app->getMattePixel(fullScreen),
		      fullScreen, app->getReverseVideo(),
		      app->getInstallCmap(), app->getRGBCubeSize());
  core->setUpdateCbk(&updateCbk, this);
  core->setActionCbk(&actionCbk, this);
  core->setKeyPressCbk(&keyPressCbk, this);
  core->setMouseCbk(&mouseCbk, this);
}

void XPDFViewer::initPopupMenu() {
  Widget btn;
  Arg args[20];
  int n;
  XmString s, s2;

  n = 0;
#if XmVersion < 1002
  // older versions of Motif need this, newer ones choke on it,
  // sometimes not displaying the menu at all, maybe depending on the
  // state of the NumLock key (taken from DDD)
  XtSetArg(args[n], XmNmenuPost, "<Btn3Down>"); ++n;
#endif
  popupMenu = XmCreatePopupMenu(core->getDrawAreaWidget(), "popupMenu",
				args, n);
  n = 0;
  s = XmStringCreateLocalized("Open...");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  s2 = XmStringCreateLocalized("O");
  XtSetArg(args[n], XmNacceleratorText, s2); ++n;
  btn = XmCreatePushButton(popupMenu, "open", args, n);
  XmStringFree(s);
  XmStringFree(s2);
  XtManageChild(btn);
  XtAddCallback(btn, XmNactivateCallback,
		&openCbk, (XtPointer)this);
  n = 0;
  s = XmStringCreateLocalized("Open in new window...");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  btn = XmCreatePushButton(popupMenu, "openInNewWindow", args, n);
  XmStringFree(s);
  XtManageChild(btn);
  XtAddCallback(btn, XmNactivateCallback,
		&openInNewWindowCbk, (XtPointer)this);
  n = 0;
  s = XmStringCreateLocalized("Reload");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  s2 = XmStringCreateLocalized("R");
  XtSetArg(args[n], XmNacceleratorText, s2); ++n;
  btn = XmCreatePushButton(popupMenu, "reload", args, n);
  XmStringFree(s);
  XmStringFree(s2);
  XtManageChild(btn);
  XtAddCallback(btn, XmNactivateCallback,
		&reloadCbk, (XtPointer)this);
  n = 0;
  s = XmStringCreateLocalized("Save as...");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  btn = XmCreatePushButton(popupMenu, "saveAs", args, n);
  XmStringFree(s);
  XtManageChild(btn);
  XtAddCallback(btn, XmNactivateCallback,
		&saveAsCbk, (XtPointer)this);
  n = 0;
  btn = XmCreateSeparator(popupMenu, "sep1", args, n);
  XtManageChild(btn);
  n = 0;
  s = XmStringCreateLocalized("Continuous view");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  XtSetArg(args[n], XmNindicatorType, XmN_OF_MANY); ++n;
  XtSetArg(args[n], XmNvisibleWhenOff, True); ++n;
  XtSetArg(args[n], XmNset, core->getContinuousMode() ? XmSET : XmUNSET); ++n;
  btn = XmCreateToggleButton(popupMenu, "continuousMode", args, n);
  XmStringFree(s);
  XtManageChild(btn);
  XtAddCallback(btn, XmNvalueChangedCallback,
		&continuousModeToggleCbk, (XtPointer)this);
  n = 0;
  s = XmStringCreateLocalized("Full screen");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  XtSetArg(args[n], XmNindicatorType, XmN_OF_MANY); ++n;
  XtSetArg(args[n], XmNvisibleWhenOff, True); ++n;
  XtSetArg(args[n], XmNset, core->getFullScreen() ? XmSET : XmUNSET); ++n;
  btn = XmCreateToggleButton(popupMenu, "fullScreen", args, n);
  XmStringFree(s);
  XtManageChild(btn);
  XtAddCallback(btn, XmNvalueChangedCallback,
		&fullScreenToggleCbk, (XtPointer)this);
  n = 0;
  s = XmStringCreateLocalized("Rotate counterclockwise");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  btn = XmCreatePushButton(popupMenu, "rotateCCW", args, n);
  XmStringFree(s);
  XtManageChild(btn);
  XtAddCallback(btn, XmNactivateCallback,
		&rotateCCWCbk, (XtPointer)this);
  n = 0;
  s = XmStringCreateLocalized("Rotate clockwise");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  btn = XmCreatePushButton(popupMenu, "rotateCW", args, n);
  XmStringFree(s);
  XtManageChild(btn);
  XtAddCallback(btn, XmNactivateCallback,
		&rotateCWCbk, (XtPointer)this);
  n = 0;
  s = XmStringCreateLocalized("Zoom to selection");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  btn = XmCreatePushButton(popupMenu, "zoomToSelection", args, n);
  XmStringFree(s);
  XtManageChild(btn);
  XtAddCallback(btn, XmNactivateCallback,
		&zoomToSelectionCbk, (XtPointer)this);
  n = 0;
  btn = XmCreateSeparator(popupMenu, "sep2", args, n);
  XtManageChild(btn);
  n = 0;
  s = XmStringCreateLocalized("Close");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  s2 = XmStringCreateLocalized("Ctrl+W");
  XtSetArg(args[n], XmNacceleratorText, s2); ++n;
  btn = XmCreatePushButton(popupMenu, "close", args, n);
  XmStringFree(s);
  XmStringFree(s2);
  XtManageChild(btn);
  XtAddCallback(btn, XmNactivateCallback,
		&closeCbk, (XtPointer)this);
  n = 0;
  s = XmStringCreateLocalized("Quit");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  s2 = XmStringCreateLocalized("Q");
  XtSetArg(args[n], XmNacceleratorText, s2); ++n;
  btn = XmCreatePushButton(popupMenu, "quit", args, n);
  XmStringFree(s);
  XmStringFree(s2);
  XtManageChild(btn);
  XtAddCallback(btn, XmNactivateCallback,
		&quitCbk, (XtPointer)this);

  // this is magic (taken from DDD) - weird things happen if this
  // call isn't made
  XtUngrabButton(core->getDrawAreaWidget(), AnyButton, AnyModifier);
}

void XPDFViewer::addToolTip(Widget widget, char *text) {
#ifdef XmNtoolTipString
  XmString s;
  Cardinal n, i;
  WidgetList children;

  if (XtIsComposite(widget)) {
    XtVaGetValues(widget, XmNnumChildren, &n, XmNchildren, &children, NULL);
    for (i = 0; i < n; ++i) {
      addToolTip(children[i], text);
    }
  } else {
    s = XmStringCreateLocalized(text);
    XtVaSetValues(widget, XmNtoolTipString, s, NULL);
    XmStringFree(s);
  }
#endif
}

void XPDFViewer::mapWindow() {
#ifdef HAVE_X11_XPM_H
  Pixmap iconPixmap;
#endif
  int depth;
  Pixel fg, bg, arm;

  // show the window
  XtPopup(win, XtGrabNone);
  core->takeFocus();

  // create the icon
#ifdef HAVE_X11_XPM_H
  if (XpmCreatePixmapFromData(display, XtWindow(win), xpdfIcon,
			      &iconPixmap, NULL, NULL) == XpmSuccess) {
    XtVaSetValues(win, XmNiconPixmap, iconPixmap, NULL);
  }
#endif

  // set button bitmaps (must be done after the window is mapped)
  if (toolBar != None) {
    XtVaGetValues(backBtn, XmNdepth, &depth,
		  XmNforeground, &fg, XmNbackground, &bg,
		  XmNarmColor, &arm, NULL);
    XtVaSetValues(backBtn, XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)backArrow_bits,
					      backArrow_width,
					      backArrow_height,
					      fg, bg, depth),
		  XmNarmPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)backArrow_bits,
					      backArrow_width,
					      backArrow_height,
					      fg, arm, depth),
		  XmNlabelInsensitivePixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)backArrowDis_bits,
					      backArrowDis_width,
					      backArrowDis_height,
					      fg, bg, depth),
		  NULL);
    XtVaSetValues(prevTenPageBtn, XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)dblLeftArrow_bits,
					      dblLeftArrow_width,
					      dblLeftArrow_height,
					      fg, bg, depth),
		  XmNarmPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)dblLeftArrow_bits,
					      dblLeftArrow_width,
					      dblLeftArrow_height,
					      fg, arm, depth),
		  XmNlabelInsensitivePixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)dblLeftArrowDis_bits,
					      dblLeftArrowDis_width,
					      dblLeftArrowDis_height,
					      fg, bg, depth),
		  NULL);
    XtVaSetValues(prevPageBtn, XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)leftArrow_bits,
					      leftArrow_width,
					      leftArrow_height,
					      fg, bg, depth),
		  XmNarmPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)leftArrow_bits,
					      leftArrow_width,
					      leftArrow_height,
					      fg, arm, depth),
		  XmNlabelInsensitivePixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)leftArrowDis_bits,
					      leftArrowDis_width,
					      leftArrowDis_height,
					      fg, bg, depth),
		  NULL);
    XtVaSetValues(nextPageBtn, XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)rightArrow_bits,
					      rightArrow_width,
					      rightArrow_height,
					      fg, bg, depth),
		  XmNarmPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)rightArrow_bits,
					      rightArrow_width,
					      rightArrow_height,
					      fg, arm, depth),
		  XmNlabelInsensitivePixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)rightArrowDis_bits,
					      rightArrowDis_width,
					      rightArrowDis_height,
					      fg, bg, depth),
		  NULL);
    XtVaSetValues(nextTenPageBtn, XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)dblRightArrow_bits,
					      dblRightArrow_width,
					      dblRightArrow_height,
					      fg, bg, depth),
		  XmNarmPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)dblRightArrow_bits,
					      dblRightArrow_width,
					      dblRightArrow_height,
					      fg, arm, depth),
		  XmNlabelInsensitivePixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)dblRightArrowDis_bits,
					      dblRightArrowDis_width,
					      dblRightArrowDis_height,
					      fg, bg, depth),
		  NULL);
    XtVaSetValues(forwardBtn, XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)forwardArrow_bits,
					      forwardArrow_width,
					      forwardArrow_height,
					      fg, bg, depth),
		  XmNarmPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)forwardArrow_bits,
					      forwardArrow_width,
					      forwardArrow_height,
					      fg, arm, depth),
		  XmNlabelInsensitivePixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)forwardArrowDis_bits,
					      forwardArrowDis_width,
					      forwardArrowDis_height,
					      fg, bg, depth),
		  NULL);
    XtVaSetValues(findBtn, XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)find_bits,
					      find_width,
					      find_height,
					      fg, bg, depth),
		  XmNarmPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)find_bits,
					      find_width,
					      find_height,
					      fg, arm, depth),
		  XmNlabelInsensitivePixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)findDis_bits,
					      findDis_width,
					      findDis_height,
					      fg, bg, depth),
		  NULL);
    XtVaSetValues(printBtn, XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)print_bits,
					      print_width,
					      print_height,
					      fg, bg, depth),
		  XmNarmPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)print_bits,
					      print_width,
					      print_height,
					      fg, arm, depth),
		  XmNlabelInsensitivePixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)printDis_bits,
					      printDis_width,
					      printDis_height,
					      fg, bg, depth),
		  NULL);
    XtVaSetValues(aboutBtn, XmNlabelType, XmPIXMAP,
		  XmNlabelPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)about_bits,
					      about_width,
					      about_height,
					      fg, bg, depth),
		  XmNarmPixmap,
		  XCreatePixmapFromBitmapData(display, XtWindow(toolBar),
					      (char *)about_bits,
					      about_width,
					      about_height,
					      fg, arm, depth),
		  NULL);
  }
}

void XPDFViewer::closeWindow() {
  XtPopdown(win);
  XtDestroyWidget(win);
}

int XPDFViewer::getZoomIdx() {
  int i;

  for (i = 0; i < nZoomMenuItems; ++i) {
    if (core->getZoom() == zoomMenuInfo[i].zoom) {
      return i;
    }
  }
  return -1;
}

void XPDFViewer::setZoomIdx(int idx) {
  if (toolBar == None) {
    return;
  }
#if USE_COMBO_BOX
  XtVaSetValues(zoomComboBox, XmNselectedPosition, idx + 1, NULL);
#else
  XtVaSetValues(zoomMenu, XmNmenuHistory, zoomMenuBtns[idx], NULL);
#endif
}

void XPDFViewer::setZoomVal(double z) {
  if (toolBar == None) {
    return;
  }

#if USE_COMBO_BOX
  char buf[32];
  XmString s;
  int i;

  for (i = 0; i < nZoomMenuItems; ++i) {
    if (z == zoomMenuInfo[i].zoom) {
      XtVaSetValues(zoomComboBox, XmNselectedPosition, i + 1, NULL);
      return;
    }
  }
  sprintf(buf, "%d%%", (int)z);
  s = XmStringCreateLocalized(buf);
  XtVaSetValues(zoomComboBox, XmNselectedItem, s, NULL);
  XmStringFree(s);
#else
  int i;

  for (i = 0; i < nZoomMenuItems; ++i) {
    if (z == zoomMenuInfo[i].zoom) {
      XtVaSetValues(zoomMenu, XmNmenuHistory, zoomMenuBtns[i], NULL);
      return;
    }
  }
  for (i = maxZoomIdx; i < minZoomIdx; ++i) {
    if (z > zoomMenuInfo[i].zoom) {
      break;
    }
  }
  XtVaSetValues(zoomMenu, XmNmenuHistory, zoomMenuBtns[i], NULL);
#endif
}

void XPDFViewer::prevPageCbk(Widget widget, XtPointer ptr,
			     XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->core->gotoPrevPage(1, gTrue, gFalse);
  viewer->core->takeFocus();
}

void XPDFViewer::prevTenPageCbk(Widget widget, XtPointer ptr,
				XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->core->gotoPrevPage(10, gTrue, gFalse);
  viewer->core->takeFocus();
}

void XPDFViewer::nextPageCbk(Widget widget, XtPointer ptr,
			     XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->core->gotoNextPage(1, gTrue);
  viewer->core->takeFocus();
}

void XPDFViewer::nextTenPageCbk(Widget widget, XtPointer ptr,
				XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->core->gotoNextPage(10, gTrue);
  viewer->core->takeFocus();
}

void XPDFViewer::backCbk(Widget widget, XtPointer ptr,
			 XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->core->goBackward();
  viewer->core->takeFocus();
}

void XPDFViewer::forwardCbk(Widget widget, XtPointer ptr,
			    XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->core->goForward();
  viewer->core->takeFocus();
}

#if USE_COMBO_BOX

void XPDFViewer::zoomComboBoxCbk(Widget widget, XtPointer ptr,
				 XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  XmComboBoxCallbackStruct *data = (XmComboBoxCallbackStruct *)callData;
  double z;
  char *s;
  XmStringContext context;
  XmStringCharSet charSet;
  XmStringDirection dir;
  Boolean sep;

  z = viewer->core->getZoom();
  if (data->item_position == 0) {
    XmStringInitContext(&context, data->item_or_text);
    if (XmStringGetNextSegment(context, &s, &charSet, &dir, &sep)) {
      z = atof(s);
      if (z <= 1) {
	z = defZoom;
      }
      XtFree(charSet);
      XtFree(s);
    }
    XmStringFreeContext(context);
  } else {
    z = zoomMenuInfo[data->item_position - 1].zoom;
  }
  // only redraw if this was triggered by an event; otherwise
  // the caller is responsible for doing the redraw
  if (z != viewer->core->getZoom() && data->event) {
    viewer->displayPage(viewer->core->getPageNum(), z,
			viewer->core->getRotate(), gTrue, gFalse);
  }
  viewer->core->takeFocus();
}

#else // USE_COMBO_BOX

void XPDFViewer::zoomMenuCbk(Widget widget, XtPointer ptr,
			     XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  XmPushButtonCallbackStruct *data = (XmPushButtonCallbackStruct *)callData;
  XtPointer userData;
  double z;

  XtVaGetValues(widget, XmNuserData, &userData, NULL);
  z = zoomMenuInfo[(long)userData].zoom;
  // only redraw if this was triggered by an event; otherwise
  // the caller is responsible for doing the redraw
  if (z != viewer->core->getZoom() && data->event) {
    viewer->displayPage(viewer->core->getPageNum(), z,
			viewer->core->getRotate(), gTrue, gFalse);
  }
  viewer->core->takeFocus();
}

#endif // USE_COMBO_BOX

void XPDFViewer::findCbk(Widget widget, XtPointer ptr,
			 XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  if (!viewer->core->getDoc()) {
    return;
  }
  viewer->mapFindDialog();
}

void XPDFViewer::printCbk(Widget widget, XtPointer ptr,
			  XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  if (!viewer->core->getDoc()) {
    return;
  }
  XtManageChild(viewer->printDialog);
}

void XPDFViewer::aboutCbk(Widget widget, XtPointer ptr,
			  XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  XtManageChild(viewer->aboutDialog);
}

void XPDFViewer::quitCbk(Widget widget, XtPointer ptr,
			 XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->app->quit();
}

void XPDFViewer::openCbk(Widget widget, XtPointer ptr,
			 XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->mapOpenDialog(gFalse);
}

void XPDFViewer::openInNewWindowCbk(Widget widget, XtPointer ptr,
				    XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->mapOpenDialog(gTrue);
}

void XPDFViewer::reloadCbk(Widget widget, XtPointer ptr,
			 XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->reloadFile();
}

void XPDFViewer::saveAsCbk(Widget widget, XtPointer ptr,
			   XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  if (!viewer->core->getDoc()) {
    return;
  }
  viewer->mapSaveAsDialog();
}

void XPDFViewer::continuousModeToggleCbk(Widget widget, XtPointer ptr,
					 XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  XmToggleButtonCallbackStruct *data =
      (XmToggleButtonCallbackStruct *)callData;

  viewer->core->setContinuousMode(data->set == XmSET);
}

void XPDFViewer::fullScreenToggleCbk(Widget widget, XtPointer ptr,
				     XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  XmToggleButtonCallbackStruct *data =
      (XmToggleButtonCallbackStruct *)callData;

  if (data->set == XmSET) {
    viewer->cmdFullScreenMode(NULL, 0, NULL);
  } else {
    viewer->cmdWindowMode(NULL, 0, NULL);
  }
}

void XPDFViewer::rotateCCWCbk(Widget widget, XtPointer ptr,
			      XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  int r;

  r = viewer->core->getRotate();
  r = (r == 0) ? 270 : r - 90;
  viewer->displayPage(viewer->core->getPageNum(), viewer->core->getZoom(),
		      r, gTrue, gFalse);
}

void XPDFViewer::rotateCWCbk(Widget widget, XtPointer ptr,
			     XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  int r;

  r = viewer->core->getRotate();
  r = (r == 270) ? 0 : r + 90;
  viewer->displayPage(viewer->core->getPageNum(), viewer->core->getZoom(),
		      r, gTrue, gFalse);
}

void XPDFViewer::zoomToSelectionCbk(Widget widget, XtPointer ptr,
				    XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  int pg;
  double ulx, uly, lrx, lry;

  if (viewer->core->getSelection(&pg, &ulx, &uly, &lrx, &lry)) {
    viewer->core->zoomToRect(pg, ulx, uly, lrx, lry);
  }
}

void XPDFViewer::closeCbk(Widget widget, XtPointer ptr,
			  XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->app->close(viewer, gFalse);
}

void XPDFViewer::closeMsgCbk(Widget widget, XtPointer ptr,
			     XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->app->close(viewer, gTrue);
}

void XPDFViewer::pageNumCbk(Widget widget, XtPointer ptr,
			    XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  char *s, *p;
  int pg;
  char buf[20];

  if (!viewer->core->getDoc()) {
    goto err;
  }
  s = XmTextFieldGetString(viewer->pageNumText);
  for (p = s; *p; ++p) {
    if (!isdigit(*p)) {
      goto err;
    }
  }
  pg = atoi(s);
  if (pg < 1 || pg > viewer->core->getDoc()->getNumPages()) {
    goto err;
  }
  viewer->displayPage(pg, viewer->core->getZoom(),
		      viewer->core->getRotate(), gFalse, gTrue);
  viewer->core->takeFocus();
  return;

 err:
  XBell(viewer->display, 0);
  sprintf(buf, "%d", viewer->core->getPageNum());
  XmTextFieldSetString(viewer->pageNumText, buf);
}

void XPDFViewer::updateCbk(void *data, GString *fileName,
			   int pageNum, int numPages, char *linkString) {
  XPDFViewer *viewer = (XPDFViewer *)data;
  GString *title;
  char buf[20];
  XmString s;

  if (fileName) {
    if (!(title = viewer->app->getTitle())) {
      title = (new GString(xpdfAppName))->append(": ")->append(fileName);
    }
    XtVaSetValues(viewer->win, XmNtitle, title->getCString(),
		  XmNiconName, title->getCString(), NULL);
    if (!viewer->app->getTitle()) {
      delete title;
    }
#ifndef DISABLE_OUTLINE
    viewer->setupOutline();
#endif
    viewer->setupPrintDialog();
  }

  if (viewer->toolBar != None) {
    if (pageNum >= 0) {
      s = XmStringCreateLocalized("");
      XtVaSetValues(viewer->linkLabel, XmNlabelString, s, NULL);
      XmStringFree(s);
      sprintf(buf, "%d", pageNum);
      XmTextFieldSetString(viewer->pageNumText, buf);
      XtVaSetValues(viewer->prevTenPageBtn, XmNsensitive,
		    pageNum > 1, NULL);
      XtVaSetValues(viewer->prevPageBtn, XmNsensitive,
		    pageNum > 1, NULL);
      XtVaSetValues(viewer->nextTenPageBtn, XmNsensitive,
		    pageNum < viewer->core->getDoc()->getNumPages(), NULL);
      XtVaSetValues(viewer->nextPageBtn, XmNsensitive,
		    pageNum < viewer->core->getDoc()->getNumPages(), NULL);
      XtVaSetValues(viewer->backBtn, XmNsensitive,
		    viewer->core->canGoBack(), NULL);
      XtVaSetValues(viewer->forwardBtn, XmNsensitive,
		    viewer->core->canGoForward(), NULL);
    }

    if (numPages >= 0) {
      sprintf(buf, " of %d", numPages);
      s = XmStringCreateLocalized(buf);
      XtVaSetValues(viewer->pageCountLabel, XmNlabelString, s, NULL);
      XmStringFree(s);
    }

    if (linkString) {
      s = XmStringCreateLocalized(linkString);
      XtVaSetValues(viewer->linkLabel, XmNlabelString, s, NULL);
      XmStringFree(s);
    }
  }
}


//------------------------------------------------------------------------
// GUI code: outline
//------------------------------------------------------------------------

#ifndef DISABLE_OUTLINE

void XPDFViewer::setupOutline() {
  GList *items;
  UnicodeMap *uMap;
  GString *enc;
  int i;

  if (outlineScroll == None) {
    return;
  }

  // unmanage and destroy the old labels
  if (outlineLabels) {
    XtUnmanageChildren(outlineLabels, outlineLabelsLength);
    for (i = 0; i < outlineLabelsLength; ++i) {
      XtDestroyWidget(outlineLabels[i]);
    }
    gfree(outlineLabels);
    outlineLabels = NULL;
    outlineLabelsLength = outlineLabelsSize = 0;
  }

  if (core->getDoc()) {

    // create the new labels
    items = core->getDoc()->getOutline()->getItems();
    if (items && items->getLength() > 0) {
      enc = new GString("Latin1");
      uMap = globalParams->getUnicodeMap(enc);
      delete enc;
      setupOutlineItems(items, NULL, uMap);
      uMap->decRefCnt();
    }

    // manage the new labels
    XtManageChildren(outlineLabels, outlineLabelsLength);
  }
}

void XPDFViewer::setupOutlineItems(GList *items, Widget parent,
				   UnicodeMap *uMap) {
  OutlineItem *item;
  GList *kids;
  Widget label;
  Arg args[20];
  GString *title;
  char buf[8];
  XmString s;
  int i, j, n;

  for (i = 0; i < items->getLength(); ++i) {
    item = (OutlineItem *)items->get(i);
    title = new GString();
    for (j = 0; j < item->getTitleLength(); ++j) {
      n = uMap->mapUnicode(item->getTitle()[j], buf, sizeof(buf));
      title->append(buf, n);
    }
    n = 0;
    XtSetArg(args[n], XPDFNentryPosition, i); ++n;
    if (parent) {
      XtSetArg(args[n], XPDFNentryParent, parent); ++n;
    }
    XtSetArg(args[n], XPDFNentryExpanded, item->isOpen()); ++n;
    s = XmStringCreateLocalized(title->getCString());
    delete title;
    XtSetArg(args[n], XmNlabelString, s); ++n;
    XtSetArg(args[n], XmNuserData, item); ++n;
    XtSetArg(args[n], XmNmarginWidth, 0); ++n;
    XtSetArg(args[n], XmNmarginHeight, 2); ++n;
    XtSetArg(args[n], XmNshadowThickness, 0); ++n;
    XtSetArg(args[n], XmNforeground,
	     app->getReverseVideo() ? WhitePixel(display, screenNum)
	                            : BlackPixel(display, screenNum)); ++n;
    XtSetArg(args[n], XmNbackground, app->getPaperPixel()); ++n;
    label = XmCreateLabelGadget(outlineTree, "label", args, n);
    XmStringFree(s);
    if (outlineLabelsLength == outlineLabelsSize) {
      outlineLabelsSize += 64;
      outlineLabels = (Widget *)greallocn(outlineLabels,
					  outlineLabelsSize, sizeof(Widget *));
    }
    outlineLabels[outlineLabelsLength++] = label;
    item->open();
    if ((kids = item->getKids())) {
      setupOutlineItems(kids, label, uMap);
    }
  }
}

void XPDFViewer::outlineSelectCbk(Widget widget, XtPointer ptr,
				  XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  XPDFTreeSelectCallbackStruct *data =
      (XPDFTreeSelectCallbackStruct *)callData;
  OutlineItem *item;

  XtVaGetValues(data->selectedItem, XmNuserData, &item, NULL);
  if (item) {
    if (item->getAction()) {
      viewer->core->doAction(item->getAction());
    }
  }
  viewer->core->takeFocus();
}

#endif // !DISABLE_OUTLINE

//------------------------------------------------------------------------
// GUI code: "about" dialog
//------------------------------------------------------------------------

void XPDFViewer::initAboutDialog() {
  Widget scrolledWin, col, label, sep, closeBtn;
  Arg args[20];
  int n, i;
  XmString s;
  char buf[20];

  //----- dialog
  n = 0;
  s = XmStringCreateLocalized(xpdfAppName ": About");
  XtSetArg(args[n], XmNdialogTitle, s); ++n;
  XtSetArg(args[n], XmNwidth, 450); ++n;
  XtSetArg(args[n], XmNheight, 300); ++n;
  aboutDialog = XmCreateFormDialog(win, "aboutDialog", args, n);
  XmStringFree(s);

  //----- "close" button
  n = 0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightOffset, 4); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomOffset, 4); ++n;
  closeBtn = XmCreatePushButton(aboutDialog, "Close", args, n);
  XtManageChild(closeBtn);
  n = 0;
  XtSetArg(args[n], XmNdefaultButton, closeBtn); ++n;
  XtSetArg(args[n], XmNcancelButton, closeBtn); ++n;
  XtSetValues(aboutDialog, args, n);

  //----- scrolled window and RowColumn
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNbottomWidget, closeBtn); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNscrollingPolicy, XmAUTOMATIC); ++n;
  scrolledWin = XmCreateScrolledWindow(aboutDialog, "scrolledWin", args, n);
  XtManageChild(scrolledWin);
  n = 0;
  XtSetArg(args[n], XmNorientation, XmVERTICAL); ++n;
  XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); ++n;
  col = XmCreateRowColumn(scrolledWin, "col", args, n);
  XtManageChild(col);

  //----- fonts
  aboutBigFont =
    createFontList("-*-times-bold-i-normal--20-*-*-*-*-*-iso8859-1");
  aboutVersionFont =
    createFontList("-*-times-medium-r-normal--16-*-*-*-*-*-iso8859-1");
  aboutFixedFont =
    createFontList("-*-courier-medium-r-normal--12-*-*-*-*-*-iso8859-1");

  //----- heading
  n = 0;
  s = XmStringCreateLocalized("Xpdf");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  XtSetArg(args[n], XmNfontList, aboutBigFont); ++n;
  label = XmCreateLabel(col, "h0", args, n);
  XmStringFree(s);
  XtManageChild(label);
  n = 0;
  s = XmStringCreateLocalized("Version " xpdfVersion);
  XtSetArg(args[n], XmNlabelString, s); ++n;
  XtSetArg(args[n], XmNfontList, aboutVersionFont); ++n;
  label = XmCreateLabel(col, "h1", args, n);
  XmStringFree(s);
  XtManageChild(label);
  n = 0;
  s = XmStringCreateLocalized(xpdfCopyright);
  XtSetArg(args[n], XmNlabelString, s); ++n;
  XtSetArg(args[n], XmNfontList, aboutVersionFont); ++n;
  label = XmCreateLabel(col, "h2", args, n);
  XmStringFree(s);
  XtManageChild(label);
  n = 0;
  s = XmStringCreateLocalized(" ");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  XtSetArg(args[n], XmNfontList, aboutVersionFont); ++n;
  label = XmCreateLabel(col, "h3", args, n);
  XmStringFree(s);
  XtManageChild(label);
  n = 0;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
  sep = XmCreateSeparator(col, "sep", args, n);
  XtManageChild(sep);
  n = 0;
  s = XmStringCreateLocalized(" ");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  XtSetArg(args[n], XmNfontList, aboutVersionFont); ++n;
  label = XmCreateLabel(col, "h4", args, n);
  XmStringFree(s);
  XtManageChild(label);
  n = 0;

  //----- text
  for (i = 0; aboutWinText[i]; ++i) {
    n = 0;
    s = XmStringCreateLocalized(aboutWinText[i]);
    XtSetArg(args[n], XmNlabelString, s); ++n;
    XtSetArg(args[n], XmNfontList, aboutFixedFont); ++n;
    sprintf(buf, "t%d", i);
    label = XmCreateLabel(col, buf, args, n);
    XtManageChild(label);
    XmStringFree(s);
  }
}

//------------------------------------------------------------------------
// GUI code: "open" dialog
//------------------------------------------------------------------------

void XPDFViewer::initOpenDialog() {
  Arg args[20];
  int n;
  XmString s1, s2, s3;
  GString *dir;

  n = 0;
  s1 = XmStringCreateLocalized("Open");
  XtSetArg(args[n], XmNokLabelString, s1); ++n;
  s2 = XmStringCreateLocalized("*.[Pp][Dd][Ff]");
  XtSetArg(args[n], XmNpattern, s2); ++n;
  s3 = XmStringCreateLocalized(xpdfAppName ": Open");
  XtSetArg(args[n], XmNdialogTitle, s3); ++n;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL); ++n;
  XtSetArg(args[n], XmNautoUnmanage, True); ++n;
  openDialog = XmCreateFileSelectionDialog(win, "openDialog", args, n);
  XmStringFree(s1);
  XmStringFree(s2);
  XmStringFree(s3);
  XtUnmanageChild(XmFileSelectionBoxGetChild(openDialog,
					     XmDIALOG_HELP_BUTTON));
  XtAddCallback(openDialog, XmNokCallback,
		&openOkCbk, (XtPointer)this);

  if (core->getDoc() && core->getDoc()->getFileName()) {
    dir = makePathAbsolute(grabPath(
	      core->getDoc()->getFileName()->getCString()));
    s1 = XmStringCreateLocalized(dir->getCString());
    XtVaSetValues(openDialog, XmNdirectory, s1, NULL);
    XmStringFree(s1);
    delete dir;
  }
}

void XPDFViewer::mapOpenDialog(GBool openInNewWindowA) {
  if (!openDialog) {
    initOpenDialog();
  }
  openInNewWindow = openInNewWindowA;
  XmFileSelectionDoSearch(openDialog, NULL);
  XtManageChild(openDialog);
}

void XPDFViewer::openOkCbk(Widget widget, XtPointer ptr,
			   XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  XmFileSelectionBoxCallbackStruct *data =
    (XmFileSelectionBoxCallbackStruct *)callData;
  char *fileName;
  XmStringContext context;
  XmStringCharSet charSet;
  XmStringDirection dir;
  Boolean sep;
  GString *fileNameStr;

  XmStringInitContext(&context, data->value);
  if (XmStringGetNextSegment(context, &fileName, &charSet, &dir, &sep)) {
    fileNameStr = new GString(fileName);
    if (viewer->openInNewWindow) {
      viewer->app->open(fileNameStr);
    } else {
      if (viewer->loadFile(fileNameStr)) {
	viewer->displayPage(1, viewer->core->getZoom(),
			    viewer->core->getRotate(), gTrue, gTrue);
      }
    }
    delete fileNameStr;
    XtFree(charSet);
    XtFree(fileName);
  }
  XmStringFreeContext(context);
}

//------------------------------------------------------------------------
// GUI code: "find" dialog
//------------------------------------------------------------------------

void XPDFViewer::initFindDialog() {
  Widget form1, label, okBtn, closeBtn;
  Arg args[20];
  int n;
  XmString s;

  //----- dialog
  n = 0;
  s = XmStringCreateLocalized(xpdfAppName ": Find");
  XtSetArg(args[n], XmNdialogTitle, s); ++n;
  XtSetArg(args[n], XmNnavigationType, XmNONE); ++n;
  XtSetArg(args[n], XmNautoUnmanage, False); ++n;
  findDialog = XmCreateFormDialog(win, "findDialog", args, n);
  XmStringFree(s);

  //----- "find" and "close" buttons
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftOffset, 4); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomOffset, 4); ++n;
  XtSetArg(args[n], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ++n;
  okBtn = XmCreatePushButton(findDialog, "Find", args, n);
  XtManageChild(okBtn);
  XtAddCallback(okBtn, XmNactivateCallback,
		&findFindCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightOffset, 4); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomOffset, 4); ++n;
  XtSetArg(args[n], XmNnavigationType, XmEXCLUSIVE_TAB_GROUP); ++n;
  closeBtn = XmCreatePushButton(findDialog, "Close", args, n);
  XtManageChild(closeBtn);
  XtAddCallback(closeBtn, XmNactivateCallback,
		&findCloseCbk, (XtPointer)this);

  //----- checkboxes
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftOffset, 4); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNbottomWidget, okBtn); ++n;
  XtSetArg(args[n], XmNindicatorType, XmN_OF_MANY); ++n;
#if XmVERSION <= 1
  XtSetArg(args[n], XmNindicatorOn, True); ++n;
#else
  XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_FILL); ++n;
#endif
  XtSetArg(args[n], XmNset, XmUNSET); ++n;
  s = XmStringCreateLocalized("Search backward");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  findBackwardToggle = XmCreateToggleButton(findDialog, "backward", args, n);
  XmStringFree(s);
  XtManageChild(findBackwardToggle);
  n = 0;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, findBackwardToggle); ++n;
  XtSetArg(args[n], XmNleftOffset, 16); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightOffset, 4); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNbottomWidget, okBtn); ++n;
  XtSetArg(args[n], XmNindicatorType, XmN_OF_MANY); ++n;
#if XmVERSION <= 1
  XtSetArg(args[n], XmNindicatorOn, True); ++n;
#else
  XtSetArg(args[n], XmNindicatorOn, XmINDICATOR_FILL); ++n;
#endif
  XtSetArg(args[n], XmNset, XmUNSET); ++n;
  s = XmStringCreateLocalized("Match case");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  findCaseSensitiveToggle =
      XmCreateToggleButton(findDialog, "matchCase", args, n);
  XmStringFree(s);
  XtManageChild(findCaseSensitiveToggle);

  //----- search string entry
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNtopOffset, 4); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNbottomWidget, findBackwardToggle); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftOffset, 2); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightOffset, 2); ++n;
  form1 = XmCreateForm(findDialog, "form", args, n);
  XtManageChild(form1);
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  s = XmStringCreateLocalized("Find text: ");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  label = XmCreateLabel(form1, "label", args, n);
  XmStringFree(s);
  XtManageChild(label);
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNleftWidget, label); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  findText = XmCreateTextField(form1, "text", args, n);
  XtManageChild(findText);
#ifdef LESSTIF_VERSION
  XtAddCallback(findText, XmNactivateCallback,
		&findFindCbk, (XtPointer)this);
#endif

  //----- dialog parameters
  n = 0;
  XtSetArg(args[n], XmNdefaultButton, okBtn); ++n;
  XtSetArg(args[n], XmNcancelButton, closeBtn); ++n;
#if XmVersion > 1001
  XtSetArg(args[n], XmNinitialFocus, findText); ++n;
#endif
  XtSetValues(findDialog, args, n);
}

void XPDFViewer::findFindCbk(Widget widget, XtPointer ptr,
			     XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  viewer->doFind(gFalse);
}

void XPDFViewer::mapFindDialog() {
  XmTextFieldSetSelection(findText, 0, XmTextFieldGetLastPosition(findText),
			  XtLastTimestampProcessed(display));
  XmTextFieldSetInsertionPosition(findText, 0);
  XtManageChild(findDialog);
}

void XPDFViewer::doFind(GBool next) {
  if (XtWindow(findDialog)) {
    XDefineCursor(display, XtWindow(findDialog), core->getBusyCursor());
  }
  core->find(XmTextFieldGetString(findText),
	     XmToggleButtonGetState(findCaseSensitiveToggle),
	     next,
	     XmToggleButtonGetState(findBackwardToggle),
	     gFalse);
  if (XtWindow(findDialog)) {
    XUndefineCursor(display, XtWindow(findDialog));
  }
}

void XPDFViewer::findCloseCbk(Widget widget, XtPointer ptr,
			      XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;

  XtUnmanageChild(viewer->findDialog);
}

//------------------------------------------------------------------------
// GUI code: "save as" dialog
//------------------------------------------------------------------------

void XPDFViewer::initSaveAsDialog() {
  Arg args[20];
  int n;
  XmString s1, s2, s3;
  GString *dir;

  n = 0;
  s1 = XmStringCreateLocalized("Save");
  XtSetArg(args[n], XmNokLabelString, s1); ++n;
  s2 = XmStringCreateLocalized("*.[Pp][Dd][Ff]");
  XtSetArg(args[n], XmNpattern, s2); ++n;
  s3 = XmStringCreateLocalized(xpdfAppName ": Save as");
  XtSetArg(args[n], XmNdialogTitle, s3); ++n;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL); ++n;
  XtSetArg(args[n], XmNautoUnmanage, True); ++n;
  saveAsDialog = XmCreateFileSelectionDialog(win, "saveAsDialog", args, n);
  XmStringFree(s1);
  XmStringFree(s2);
  XmStringFree(s3);
  XtUnmanageChild(XmFileSelectionBoxGetChild(saveAsDialog,
					     XmDIALOG_HELP_BUTTON));
  XtAddCallback(saveAsDialog, XmNokCallback,
		&saveAsOkCbk, (XtPointer)this);

  if (core->getDoc() && core->getDoc()->getFileName()) {
    dir = makePathAbsolute(grabPath(
	      core->getDoc()->getFileName()->getCString()));
    s1 = XmStringCreateLocalized(dir->getCString());
    XtVaSetValues(saveAsDialog, XmNdirectory, s1, NULL);
    XmStringFree(s1);
    delete dir;
  }
}

void XPDFViewer::mapSaveAsDialog() {
  if (!saveAsDialog) {
    initSaveAsDialog();
  }
  XmFileSelectionDoSearch(saveAsDialog, NULL);
  XtManageChild(saveAsDialog);
}

void XPDFViewer::saveAsOkCbk(Widget widget, XtPointer ptr,
			     XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  XmFileSelectionBoxCallbackStruct *data =
    (XmFileSelectionBoxCallbackStruct *)callData;
  char *fileName;
  GString *fileNameStr;
  XmStringContext context;
  XmStringCharSet charSet;
  XmStringDirection dir;
  Boolean sep;

  XmStringInitContext(&context, data->value);
  if (XmStringGetNextSegment(context, &fileName, &charSet, &dir, &sep)) {
    fileNameStr = new GString(fileName);
    viewer->core->getDoc()->saveAs(fileNameStr);
    delete fileNameStr;
    XtFree(charSet);
    XtFree(fileName);
  }
  XmStringFreeContext(context);
}

//------------------------------------------------------------------------
// GUI code: "print" dialog
//------------------------------------------------------------------------

void XPDFViewer::initPrintDialog() {
  Widget sep1, sep2, row, label1, label2, okBtn, cancelBtn;
  Arg args[20];
  int n;
  XmString s;
  GString *psFileName;

  //----- dialog
  n = 0;
  s = XmStringCreateLocalized(xpdfAppName ": Print");
  XtSetArg(args[n], XmNdialogTitle, s); ++n;
  XtSetArg(args[n], XmNdialogStyle, XmDIALOG_PRIMARY_APPLICATION_MODAL); ++n;
  printDialog = XmCreateFormDialog(win, "printDialog", args, n);
  XmStringFree(s);

  //----- "print with command"
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNtopOffset, 4); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNindicatorType, XmONE_OF_MANY); ++n;
  XtSetArg(args[n], XmNset, XmSET); ++n;
  s = XmStringCreateLocalized("Print with command:");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  printWithCmdBtn = XmCreateToggleButton(printDialog, "printWithCmd", args, n);
  XmStringFree(s);
  XtManageChild(printWithCmdBtn);
  XtAddCallback(printWithCmdBtn, XmNvalueChangedCallback,
		&printWithCmdBtnCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNtopWidget, printWithCmdBtn); ++n;
  XtSetArg(args[n], XmNtopOffset, 2); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftOffset, 16); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightOffset, 4); ++n;
  XtSetArg(args[n], XmNcolumns, 40); ++n;
  printCmdText = XmCreateTextField(printDialog, "printCmd", args, n);
  XtManageChild(printCmdText);

  //----- "print to file"
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNtopWidget, printCmdText); ++n;
  XtSetArg(args[n], XmNtopOffset, 4); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNindicatorType, XmONE_OF_MANY); ++n;
  XtSetArg(args[n], XmNset, XmUNSET); ++n;
  s = XmStringCreateLocalized("Print to file:");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  printToFileBtn = XmCreateToggleButton(printDialog, "printToFile", args, n);
  XmStringFree(s);
  XtManageChild(printToFileBtn);
  XtAddCallback(printToFileBtn, XmNvalueChangedCallback,
		&printToFileBtnCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNtopWidget, printToFileBtn); ++n;
  XtSetArg(args[n], XmNtopOffset, 2); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftOffset, 16); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightOffset, 4); ++n;
  XtSetArg(args[n], XmNcolumns, 40); ++n;
  XtSetArg(args[n], XmNsensitive, False); ++n;
  printFileText = XmCreateTextField(printDialog, "printFile", args, n);
  XtManageChild(printFileText);

  //----- separator
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNtopWidget, printFileText); ++n;
  XtSetArg(args[n], XmNtopOffset, 8); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftOffset, 8); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightOffset, 8); ++n;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
  sep1 = XmCreateSeparator(printDialog, "sep1", args, n);
  XtManageChild(sep1);

  //----- page range
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNtopWidget, sep1); ++n;
  XtSetArg(args[n], XmNtopOffset, 8); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftOffset, 4); ++n;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
  XtSetArg(args[n], XmNpacking, XmPACK_TIGHT); ++n;
  row = XmCreateRowColumn(printDialog, "row", args, n);
  XtManageChild(row);
  n = 0;
  s = XmStringCreateLocalized("Pages:");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  label1 = XmCreateLabel(row, "label1", args, n);
  XmStringFree(s);
  XtManageChild(label1);
  n = 0;
  XtSetArg(args[n], XmNcolumns, 5); ++n;
  printFirstPage = XmCreateTextField(row, "printFirstPage", args, n);
  XtManageChild(printFirstPage);
  n = 0;
  s = XmStringCreateLocalized("to");
  XtSetArg(args[n], XmNlabelString, s); ++n;
  label2 = XmCreateLabel(row, "label2", args, n);
  XmStringFree(s);
  XtManageChild(label2);
  n = 0;
  XtSetArg(args[n], XmNcolumns, 5); ++n;
  printLastPage = XmCreateTextField(row, "printLastPage", args, n);
  XtManageChild(printLastPage);

  //----- separator
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNtopWidget, row); ++n;
  XtSetArg(args[n], XmNtopOffset, 8); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftOffset, 8); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightOffset, 8); ++n;
  XtSetArg(args[n], XmNorientation, XmHORIZONTAL); ++n;
  sep2 = XmCreateSeparator(printDialog, "sep2", args, n);
  XtManageChild(sep2);

  //----- "print" and "cancel" buttons
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNtopWidget, sep2); ++n;
  XtSetArg(args[n], XmNtopOffset, 8); ++n;
  XtSetArg(args[n], XmNleftAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNleftOffset, 4); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomOffset, 4); ++n;
  okBtn = XmCreatePushButton(printDialog, "Print", args, n);
  XtManageChild(okBtn);
  XtAddCallback(okBtn, XmNactivateCallback,
		&printPrintCbk, (XtPointer)this);
  n = 0;
  XtSetArg(args[n], XmNtopAttachment, XmATTACH_WIDGET); ++n;
  XtSetArg(args[n], XmNtopWidget, sep2); ++n;
  XtSetArg(args[n], XmNtopOffset, 8); ++n;
  XtSetArg(args[n], XmNrightAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNrightOffset, 4); ++n;
  XtSetArg(args[n], XmNbottomAttachment, XmATTACH_FORM); ++n;
  XtSetArg(args[n], XmNbottomOffset, 4); ++n;
  cancelBtn = XmCreatePushButton(printDialog, "Cancel", args, n);
  XtManageChild(cancelBtn);
  n = 0;
  XtSetArg(args[n], XmNdefaultButton, okBtn); ++n;
  XtSetArg(args[n], XmNcancelButton, cancelBtn); ++n;
  XtSetValues(printDialog, args, n);

  //----- initial values
  if ((psFileName = globalParams->getPSFile())) {
    if (psFileName->getChar(0) == '|') {
      XmTextFieldSetString(printCmdText,
			   psFileName->getCString() + 1);
    } else {
      XmTextFieldSetString(printFileText, psFileName->getCString());
    }
    delete psFileName;
  }
}

void XPDFViewer::setupPrintDialog() {
  PDFDoc *doc;
  char buf[20];
  GString *pdfFileName, *psFileName, *psFileName2;
  char *p;

  doc = core->getDoc();
  psFileName = globalParams->getPSFile();
  if (!psFileName || psFileName->getChar(0) == '|') {
    pdfFileName = doc->getFileName();
    p = pdfFileName->getCString() + pdfFileName->getLength() - 4;
    if (!strcmp(p, ".pdf") || !strcmp(p, ".PDF")) {
      psFileName2 = new GString(pdfFileName->getCString(),
				pdfFileName->getLength() - 4);
    } else {
      psFileName2 = pdfFileName->copy();
    }
    psFileName2->append(".ps");
    XmTextFieldSetString(printFileText, psFileName2->getCString());
    delete psFileName2;
  }
  if (psFileName && psFileName->getChar(0) == '|') {
    XmToggleButtonSetState(printWithCmdBtn, True, False);
    XmToggleButtonSetState(printToFileBtn, False, False);
    XtVaSetValues(printCmdText, XmNsensitive, True, NULL);
    XtVaSetValues(printFileText, XmNsensitive, False, NULL);
  } else {
    XmToggleButtonSetState(printWithCmdBtn, False, False);
    XmToggleButtonSetState(printToFileBtn, True, False);
    XtVaSetValues(printCmdText, XmNsensitive, False, NULL);
    XtVaSetValues(printFileText, XmNsensitive, True, NULL);
  }
  if (psFileName) {
    delete psFileName;
  }

  sprintf(buf, "%d", doc->getNumPages());
  XmTextFieldSetString(printFirstPage, "1");
  XmTextFieldSetString(printLastPage, buf);
}

void XPDFViewer::printWithCmdBtnCbk(Widget widget, XtPointer ptr,
				    XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  XmToggleButtonCallbackStruct *data =
      (XmToggleButtonCallbackStruct *)callData;

  if (data->set != XmSET) {
    XmToggleButtonSetState(viewer->printWithCmdBtn, True, False);
  }
  XmToggleButtonSetState(viewer->printToFileBtn, False, False);
  XtVaSetValues(viewer->printCmdText, XmNsensitive, True, NULL);
  XtVaSetValues(viewer->printFileText, XmNsensitive, False, NULL);
}

void XPDFViewer::printToFileBtnCbk(Widget widget, XtPointer ptr,
				   XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  XmToggleButtonCallbackStruct *data =
      (XmToggleButtonCallbackStruct *)callData;

  if (data->set != XmSET) {
    XmToggleButtonSetState(viewer->printToFileBtn, True, False);
  }
  XmToggleButtonSetState(viewer->printWithCmdBtn, False, False);
  XtVaSetValues(viewer->printFileText, XmNsensitive, True, NULL);
  XtVaSetValues(viewer->printCmdText, XmNsensitive, False, NULL);
}

void XPDFViewer::printPrintCbk(Widget widget, XtPointer ptr,
			       XtPointer callData) {
  XPDFViewer *viewer = (XPDFViewer *)ptr;
  unsigned char withCmd;
  GString *psFileName;
  int firstPage, lastPage;
  PDFDoc *doc;
  PSOutputDev *psOut;

  doc = viewer->core->getDoc();
  if (!doc->okToPrint()) {
    error(-1, "Printing this document is not allowed.");
    return;
  }

  viewer->core->setBusyCursor(gTrue);

  XtVaGetValues(viewer->printWithCmdBtn, XmNset, &withCmd, NULL);
  if (withCmd) {
    psFileName = new GString(XmTextFieldGetString(viewer->printCmdText));
    psFileName->insert(0, '|');
  } else {
    psFileName = new GString(XmTextFieldGetString(viewer->printFileText));
  }

  firstPage = atoi(XmTextFieldGetString(viewer->printFirstPage));
  lastPage = atoi(XmTextFieldGetString(viewer->printLastPage));
  if (firstPage < 1) {
    firstPage = 1;
  } else if (firstPage > doc->getNumPages()) {
    firstPage = doc->getNumPages();
  }
  if (lastPage < firstPage) {
    lastPage = firstPage;
  } else if (lastPage > doc->getNumPages()) {
    lastPage = doc->getNumPages();
  }

  psOut = new PSOutputDev(psFileName->getCString(), doc->getXRef(),
			  doc->getCatalog(), firstPage, lastPage,
			  psModePS);
  if (psOut->isOk()) {
    doc->displayPages(psOut, firstPage, lastPage, 72, 72,
		      0, gTrue, globalParams->getPSCrop(), gTrue);
  }
  delete psOut;
  delete psFileName;

  viewer->core->setBusyCursor(gFalse);
}

//------------------------------------------------------------------------
// Motif support
//------------------------------------------------------------------------

XmFontList XPDFViewer::createFontList(char *xlfd) {
  XmFontList fontList;

#if XmVersion <= 1001

  XFontStruct *font;
  String params;
  Cardinal nParams;

  font = XLoadQueryFont(display, xlfd);
  if (font) {
    fontList = XmFontListCreate(font, XmSTRING_DEFAULT_CHARSET);
  } else {
    params = (String)xlfd;
    nParams = 1;
    XtAppWarningMsg(app->getAppContext(),
		    "noSuchFont", "CvtStringToXmFontList",
		    "XtToolkitError", "No such font: %s",
		    &params, &nParams);
    fontList = NULL;
  }

#else

  XmFontListEntry entry;

  entry = XmFontListEntryLoad(display, xlfd,
			      XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG);
  fontList = XmFontListAppendEntry(NULL, entry);
  XmFontListEntryFree(&entry);

#endif

  return fontList;
}
