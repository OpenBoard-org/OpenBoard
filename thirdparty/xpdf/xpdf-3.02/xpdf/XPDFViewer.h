//========================================================================
//
// XPDFViewer.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef XPDFVIEWER_H
#define XPDFVIEWER_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#define Object XtObject
#include <Xm/XmAll.h>
#undef Object
#include "gtypes.h"
#include "XPDFCore.h"

#if (XmVERSION <= 1) && !defined(__sgi)
#define DISABLE_OUTLINE
#endif

#if (XmVERSION >= 2 && !defined(LESSTIF_VERSION))
#  define USE_COMBO_BOX 1
#else
#  undef USE_COMBO_BOX
#endif

class GString;
class GList;
class UnicodeMap;
class LinkDest;
class XPDFApp;
class XPDFViewer;

//------------------------------------------------------------------------

// NB: this must match the defn of zoomMenuBtnInfo in XPDFViewer.cc
#define nZoomMenuItems 10

//------------------------------------------------------------------------

struct XPDFViewerCmd {
  char *name;
  int nArgs;
  GBool requiresDoc;
  GBool requiresEvent;
  void (XPDFViewer::*func)(GString *args[], int nArgs, XEvent *event);
};

//------------------------------------------------------------------------
// XPDFViewer
//------------------------------------------------------------------------

class XPDFViewer {
public:

  XPDFViewer(XPDFApp *appA, GString *fileName,
	     int pageA, GString *destName, GBool fullScreen,
	     GString *ownerPassword, GString *userPassword);
  XPDFViewer(XPDFApp *appA, PDFDoc *doc, int pageA,
	     GString *destName, GBool fullScreen);
  GBool isOk() { return ok; }
  ~XPDFViewer();

  void open(GString *fileName, int pageA, GString *destName);
  void clear();
  void reloadFile();

  void execCmd(GString *cmd, XEvent *event);

  Widget getWindow() { return win; }

private:

  //----- load / display
  GBool loadFile(GString *fileName, GString *ownerPassword = NULL,
		 GString *userPassword = NULL);
  void displayPage(int pageA, double zoomA, int rotateA,
                   GBool scrollToTop, GBool addToHist);
  void displayDest(LinkDest *dest, double zoomA, int rotateA,
		   GBool addToHist);
  void getPageAndDest(int pageA, GString *destName,
		      int *pageOut, LinkDest **destOut);

  //----- hyperlinks / actions
  void doLink(int wx, int wy, GBool onlyIfNoSelection, GBool newWin);
  static void actionCbk(void *data, char *action);

  //----- keyboard/mouse input
  static void keyPressCbk(void *data, KeySym key, Guint modifiers,
			  XEvent *event);
  static void mouseCbk(void *data, XEvent *event);
  int getModifiers(Guint modifiers);
  int getContext(Guint modifiers);

  //----- command functions
  void cmdAbout(GString *args[], int nArgs, XEvent *event);
  void cmdCloseOutline(GString *args[], int nArgs, XEvent *event);
  void cmdCloseWindow(GString *args[], int nArgs, XEvent *event);
  void cmdContinuousMode(GString *args[], int nArgs, XEvent *event);
  void cmdEndPan(GString *args[], int nArgs, XEvent *event);
  void cmdEndSelection(GString *args[], int nArgs, XEvent *event);
  void cmdFind(GString *args[], int nArgs, XEvent *event);
  void cmdFindNext(GString *args[], int nArgs, XEvent *event);
  void cmdFocusToDocWin(GString *args[], int nArgs, XEvent *event);
  void cmdFocusToPageNum(GString *args[], int nArgs, XEvent *event);
  void cmdFollowLink(GString *args[], int nArgs, XEvent *event);
  void cmdFollowLinkInNewWin(GString *args[], int nArgs, XEvent *event);
  void cmdFollowLinkInNewWinNoSel(GString *args[], int nArgs, XEvent *event);
  void cmdFollowLinkNoSel(GString *args[], int nArgs, XEvent *event);
  void cmdFullScreenMode(GString *args[], int nArgs, XEvent *event);
  void cmdGoBackward(GString *args[], int nArgs, XEvent *event);
  void cmdGoForward(GString *args[], int nArgs, XEvent *event);
  void cmdGotoDest(GString *args[], int nArgs, XEvent *event);
  void cmdGotoLastPage(GString *args[], int nArgs, XEvent *event);
  void cmdGotoLastPageNoScroll(GString *args[], int nArgs, XEvent *event);
  void cmdGotoPage(GString *args[], int nArgs, XEvent *event);
  void cmdGotoPageNoScroll(GString *args[], int nArgs, XEvent *event);
  void cmdNextPage(GString *args[], int nArgs, XEvent *event);
  void cmdNextPageNoScroll(GString *args[], int nArgs, XEvent *event);
  void cmdOpen(GString *args[], int nArgs, XEvent *event);
  void cmdOpenFile(GString *args[], int nArgs, XEvent *event);
  void cmdOpenFileAtDest(GString *args[], int nArgs, XEvent *event);
  void cmdOpenFileAtDestInNewWin(GString *args[], int nArgs, XEvent *event);
  void cmdOpenFileAtPage(GString *args[], int nArgs, XEvent *event);
  void cmdOpenFileAtPageInNewWin(GString *args[], int nArgs, XEvent *event);
  void cmdOpenFileInNewWin(GString *args[], int nArgs, XEvent *event);
  void cmdOpenInNewWin(GString *args[], int nArgs, XEvent *event);
  void cmdOpenOutline(GString *args[], int nArgs, XEvent *event);
  void cmdPageDown(GString *args[], int nArgs, XEvent *event);
  void cmdPageUp(GString *args[], int nArgs, XEvent *event);
  void cmdPostPopupMenu(GString *args[], int nArgs, XEvent *event);
  void cmdPrevPage(GString *args[], int nArgs, XEvent *event);
  void cmdPrevPageNoScroll(GString *args[], int nArgs, XEvent *event);
  void cmdPrint(GString *args[], int nArgs, XEvent *event);
  void cmdQuit(GString *args[], int nArgs, XEvent *event);
  void cmdRaise(GString *args[], int nArgs, XEvent *event);
  void cmdRedraw(GString *args[], int nArgs, XEvent *event);
  void cmdReload(GString *args[], int nArgs, XEvent *event);
  void cmdRun(GString *args[], int nArgs, XEvent *event);
  void cmdScrollDown(GString *args[], int nArgs, XEvent *event);
  void cmdScrollDownNextPage(GString *args[], int nArgs, XEvent *event);
  void cmdScrollLeft(GString *args[], int nArgs, XEvent *event);
  void cmdScrollOutlineDown(GString *args[], int nArgs, XEvent *event);
  void cmdScrollOutlineUp(GString *args[], int nArgs, XEvent *event);
  void cmdScrollRight(GString *args[], int nArgs, XEvent *event);
  void cmdScrollToBottomEdge(GString *args[], int nArgs, XEvent *event);
  void cmdScrollToBottomRight(GString *args[], int nArgs, XEvent *event);
  void cmdScrollToLeftEdge(GString *args[], int nArgs, XEvent *event);
  void cmdScrollToRightEdge(GString *args[], int nArgs, XEvent *event);
  void cmdScrollToTopEdge(GString *args[], int nArgs, XEvent *event);
  void cmdScrollToTopLeft(GString *args[], int nArgs, XEvent *event);
  void cmdScrollUp(GString *args[], int nArgs, XEvent *event);
  void cmdScrollUpPrevPage(GString *args[], int nArgs, XEvent *event);
  void cmdSinglePageMode(GString *args[], int nArgs, XEvent *event);
  void cmdStartPan(GString *args[], int nArgs, XEvent *event);
  void cmdStartSelection(GString *args[], int nArgs, XEvent *event);
  void cmdToggleContinuousMode(GString *args[], int nArgs, XEvent *event);
  void cmdToggleFullScreenMode(GString *args[], int nArgs, XEvent *event);
  void cmdToggleOutline(GString *args[], int nArgs, XEvent *event);
  void cmdWindowMode(GString *args[], int nArgs, XEvent *event);
  void cmdZoomFitPage(GString *args[], int nArgs, XEvent *event);
  void cmdZoomFitWidth(GString *args[], int nArgs, XEvent *event);
  void cmdZoomIn(GString *args[], int nArgs, XEvent *event);
  void cmdZoomOut(GString *args[], int nArgs, XEvent *event);
  void cmdZoomPercent(GString *args[], int nArgs, XEvent *event);
  void cmdZoomToSelection(GString *args[], int nArgs, XEvent *event);

  //----- GUI code: main window
  void initWindow(GBool fullScreen);
  void initToolbar(Widget parent);
#ifndef DISABLE_OUTLINE
  void initPanedWin(Widget parent);
#endif
  void initCore(Widget parent, GBool fullScreen);
  void initPopupMenu();
  void addToolTip(Widget widget, char *text);
  void mapWindow();
  void closeWindow();
  int getZoomIdx();
  void setZoomIdx(int idx);
  void setZoomVal(double z);
  static void prevPageCbk(Widget widget, XtPointer ptr,
			  XtPointer callData);
  static void prevTenPageCbk(Widget widget, XtPointer ptr,
			     XtPointer callData);
  static void nextPageCbk(Widget widget, XtPointer ptr,
			  XtPointer callData);
  static void nextTenPageCbk(Widget widget, XtPointer ptr,
			     XtPointer callData);
  static void backCbk(Widget widget, XtPointer ptr,
		      XtPointer callData);
  static void forwardCbk(Widget widget, XtPointer ptr,
			 XtPointer callData);
#if USE_COMBO_BOX
  static void zoomComboBoxCbk(Widget widget, XtPointer ptr,
			      XtPointer callData);
#else
  static void zoomMenuCbk(Widget widget, XtPointer ptr,
			  XtPointer callData);
#endif
  static void findCbk(Widget widget, XtPointer ptr,
		      XtPointer callData);
  static void printCbk(Widget widget, XtPointer ptr,
		       XtPointer callData);
  static void aboutCbk(Widget widget, XtPointer ptr,
		       XtPointer callData);
  static void quitCbk(Widget widget, XtPointer ptr,
		      XtPointer callData);
  static void openCbk(Widget widget, XtPointer ptr,
		      XtPointer callData);
  static void openInNewWindowCbk(Widget widget, XtPointer ptr,
				 XtPointer callData);
  static void reloadCbk(Widget widget, XtPointer ptr,
			XtPointer callData);
  static void saveAsCbk(Widget widget, XtPointer ptr,
			XtPointer callData);
  static void continuousModeToggleCbk(Widget widget, XtPointer ptr,
				      XtPointer callData);
  static void fullScreenToggleCbk(Widget widget, XtPointer ptr,
				  XtPointer callData);
  static void rotateCCWCbk(Widget widget, XtPointer ptr,
			   XtPointer callData);
  static void rotateCWCbk(Widget widget, XtPointer ptr,
			  XtPointer callData);
  static void zoomToSelectionCbk(Widget widget, XtPointer ptr,
				 XtPointer callData);
  static void closeCbk(Widget widget, XtPointer ptr,
		       XtPointer callData);
  static void closeMsgCbk(Widget widget, XtPointer ptr,
			  XtPointer callData);
  static void pageNumCbk(Widget widget, XtPointer ptr,
			 XtPointer callData);
  static void updateCbk(void *data, GString *fileName,
			int pageNum, int numPages, char *linkString);

  //----- GUI code: outline
#ifndef DISABLE_OUTLINE
  void setupOutline();
  void setupOutlineItems(GList *items, Widget parent, UnicodeMap *uMap);
  static void outlineSelectCbk(Widget widget, XtPointer ptr,
			       XtPointer callData);
#endif

  //----- GUI code: "about" dialog
  void initAboutDialog();

  //----- GUI code: "open" dialog
  void initOpenDialog();
  void mapOpenDialog(GBool openInNewWindowA);
  static void openOkCbk(Widget widget, XtPointer ptr,
			XtPointer callData);

  //----- GUI code: "find" dialog
  void initFindDialog();
  static void findFindCbk(Widget widget, XtPointer ptr,
			  XtPointer callData);
  void mapFindDialog();
  void doFind(GBool next);
  static void findCloseCbk(Widget widget, XtPointer ptr,
			   XtPointer callData);

  //----- GUI code: "save as" dialog
  void initSaveAsDialog();
  void mapSaveAsDialog();
  static void saveAsOkCbk(Widget widget, XtPointer ptr,
			  XtPointer callData);

  //----- GUI code: "print" dialog
  void initPrintDialog();
  void setupPrintDialog();
  static void printWithCmdBtnCbk(Widget widget, XtPointer ptr,
				 XtPointer callData);
  static void printToFileBtnCbk(Widget widget, XtPointer ptr,
				XtPointer callData);
  static void printPrintCbk(Widget widget, XtPointer ptr,
			    XtPointer callData);

  //----- Motif support
  XmFontList createFontList(char *xlfd);

  static XPDFViewerCmd cmdTab[];

  XPDFApp *app;
  GBool ok;

  Display *display;
  int screenNum;
  Widget win;			// top-level window
  Widget form;
  Widget panedWin;
#ifndef DISABLE_OUTLINE
  Widget outlineScroll;
  Widget outlineTree;
  Widget *outlineLabels;
  int outlineLabelsLength;
  int outlineLabelsSize;
  Dimension outlinePaneWidth;
#endif
  XPDFCore *core;
  Widget toolBar;
  Widget backBtn;
  Widget prevTenPageBtn;
  Widget prevPageBtn;
  Widget nextPageBtn;
  Widget nextTenPageBtn;
  Widget forwardBtn;
  Widget pageNumText;
  Widget pageCountLabel;
#if USE_COMBO_BOX
  Widget zoomComboBox;
#else
  Widget zoomMenu;
  Widget zoomMenuBtns[nZoomMenuItems];
#endif
  Widget zoomWidget;
  Widget findBtn;
  Widget printBtn;
  Widget aboutBtn;
  Widget linkLabel;
  Widget quitBtn;
  Widget popupMenu;

  Widget aboutDialog;
  XmFontList aboutBigFont, aboutVersionFont, aboutFixedFont;

  Widget openDialog;
  GBool openInNewWindow;

  Widget findDialog;
  Widget findText;
  Widget findBackwardToggle;
  Widget findCaseSensitiveToggle;

  Widget saveAsDialog;

  Widget printDialog;
  Widget printWithCmdBtn;
  Widget printToFileBtn;
  Widget printCmdText;
  Widget printFileText;
  Widget printFirstPage;
  Widget printLastPage;
};

#endif
