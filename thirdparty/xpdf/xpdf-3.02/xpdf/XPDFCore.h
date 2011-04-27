//========================================================================
//
// XPDFCore.h
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef XPDFCORE_H
#define XPDFCORE_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#define Object XtObject
#include <Xm/XmAll.h>
#undef Object
#include "gtypes.h"
#include "gfile.h" // for time_t
#include "SplashTypes.h"
#include "PDFCore.h"

class GString;
class BaseStream;
class PDFDoc;
class LinkAction;

//------------------------------------------------------------------------

#define xMaxRGBCube 6		// max size of RGB color cube

//------------------------------------------------------------------------
// callbacks
//------------------------------------------------------------------------

typedef void (*XPDFUpdateCbk)(void *data, GString *fileName,
			      int pageNum, int numPages, char *linkLabel);

typedef void (*XPDFActionCbk)(void *data, char *action);

typedef void (*XPDFKeyPressCbk)(void *data, KeySym key, Guint modifiers,
				XEvent *event);

typedef void (*XPDFMouseCbk)(void *data, XEvent *event);

//------------------------------------------------------------------------
// XPDFCore
//------------------------------------------------------------------------

class XPDFCore: public PDFCore {
public:

  // Create viewer core inside <parentWidgetA>.
  XPDFCore(Widget shellA, Widget parentWidgetA,
	   SplashColorPtr paperColorA, Gulong paperPixelA,
	   Gulong mattePixelA, GBool fullScreenA, GBool reverseVideoA,
	   GBool installCmap, int rgbCubeSizeA);

  ~XPDFCore();

  //----- loadFile / displayPage / displayDest

  // Load a new file.  Returns pdfOk or error code.
  virtual int loadFile(GString *fileName, GString *ownerPassword = NULL,
		       GString *userPassword = NULL);

  // Load a new file, via a Stream instead of a file name.  Returns
  // pdfOk or error code.
  virtual int loadFile(BaseStream *stream, GString *ownerPassword = NULL,
		       GString *userPassword = NULL);

  // Load an already-created PDFDoc object.
  virtual void loadDoc(PDFDoc *docA);

  // Resize the window to fit page <pg> of the current document.
  void resizeToPage(int pg);

  // Update the display, given the specified parameters.
  virtual void update(int topPageA, int scrollXA, int scrollYA,
		      double zoomA, int rotateA,
		      GBool force, GBool addToHist);

  //----- page/position changes

  virtual GBool gotoNextPage(int inc, GBool top);
  virtual GBool gotoPrevPage(int dec, GBool top, GBool bottom);
  virtual GBool goForward();
  virtual GBool goBackward();

  //----- selection

  void startSelection(int wx, int wy);
  void endSelection(int wx, int wy);
  void copySelection();
  void startPan(int wx, int wy);
  void endPan(int wx, int wy);

  //----- hyperlinks

  void doAction(LinkAction *action);
  LinkAction *getLinkAction() { return linkAction; }
  GString *mungeURL(GString *url);

  //----- find

  virtual GBool find(char *s, GBool caseSensitive,
		     GBool next, GBool backward, GBool onePageOnly);
  virtual GBool findU(Unicode *u, int len, GBool caseSensitive,
		      GBool next, GBool backward, GBool onePageOnly);

  //----- simple modal dialogs

  GBool doQuestionDialog(char *title, GString *msg);
  void doInfoDialog(char *title, GString *msg);
  void doErrorDialog(char *title, GString *msg);

  //----- password dialog

  GString *getPassword();

  //----- misc access

  Widget getWidget() { return scrolledWin; }
  Widget getDrawAreaWidget() { return drawArea; }
  virtual void setBusyCursor(GBool busy);
  Cursor getBusyCursor() { return busyCursor; }
  void takeFocus();
  void enableHyperlinks(GBool on) { hyperlinksEnabled = on; }
  GBool getHyperlinksEnabled() { return hyperlinksEnabled; }
  void enableSelect(GBool on) { selectEnabled = on; }
  void setUpdateCbk(XPDFUpdateCbk cbk, void *data)
    { updateCbk = cbk; updateCbkData = data; }
  void setActionCbk(XPDFActionCbk cbk, void *data)
    { actionCbk = cbk; actionCbkData = data; }
  void setKeyPressCbk(XPDFKeyPressCbk cbk, void *data)
    { keyPressCbk = cbk; keyPressCbkData = data; }
  void setMouseCbk(XPDFMouseCbk cbk, void *data)
    { mouseCbk = cbk; mouseCbkData = data; }
  GBool getFullScreen() { return fullScreen; }

private:

  virtual GBool checkForNewFile();

  //----- hyperlinks
  void runCommand(GString *cmdFmt, GString *arg);

  //----- selection
  static Boolean convertSelectionCbk(Widget widget, Atom *selection,
				     Atom *target, Atom *type,
				     XtPointer *value, unsigned long *length,
				     int *format);

  //----- GUI code
  void setupX(GBool installCmap, int rgbCubeSizeA);
  void initWindow();
  static void hScrollChangeCbk(Widget widget, XtPointer ptr,
			       XtPointer callData);
  static void hScrollDragCbk(Widget widget, XtPointer ptr,
			     XtPointer callData);
  static void vScrollChangeCbk(Widget widget, XtPointer ptr,
			       XtPointer callData);
  static void vScrollDragCbk(Widget widget, XtPointer ptr,
			     XtPointer callData);
  static void resizeCbk(Widget widget, XtPointer ptr, XtPointer callData);
  static void redrawCbk(Widget widget, XtPointer ptr, XtPointer callData);
  static void inputCbk(Widget widget, XtPointer ptr, XtPointer callData);
  virtual PDFCoreTile *newTile(int xDestA, int yDestA);
  virtual void updateTileData(PDFCoreTile *tileA, int xSrc, int ySrc,
			      int width, int height, GBool composited);
  virtual void redrawRect(PDFCoreTile *tileA, int xSrc, int ySrc,
			  int xDest, int yDest, int width, int height,
			  GBool composited);
  virtual void updateScrollbars();
  void setCursor(Cursor cursor);
  GBool doDialog(int type, GBool hasCancel,
		 char *title, GString *msg);
  static void dialogOkCbk(Widget widget, XtPointer ptr,
			  XtPointer callData);
  static void dialogCancelCbk(Widget widget, XtPointer ptr,
			      XtPointer callData);
  void initPasswordDialog();
  static void passwordTextVerifyCbk(Widget widget, XtPointer ptr,
				    XtPointer callData);
  static void passwordOkCbk(Widget widget, XtPointer ptr,
			    XtPointer callData);
  static void passwordCancelCbk(Widget widget, XtPointer ptr,
				XtPointer callData);

  Gulong paperPixel;
  Gulong mattePixel;
  //~unimp: move fullScreen into PDFCore?
  GBool fullScreen;

  Display *display;
  int screenNum;
  Visual *visual;
  Colormap colormap;
  Guint depth;                  // visual depth
  GBool trueColor;              // set if using a TrueColor visual
  int rDiv, gDiv, bDiv;         // RGB right shifts (for TrueColor)
  int rShift, gShift, bShift;   // RGB left shifts (for TrueColor)
  int rgbCubeSize;              // size of color cube (for non-TrueColor)
  Gulong                        // color cube (for non-TrueColor)
    colors[xMaxRGBCube * xMaxRGBCube * xMaxRGBCube];

  Widget shell;			// top-level shell containing the widget
  Widget parentWidget;		// parent widget (not created by XPDFCore)
  Widget scrolledWin;
  Widget hScrollBar;
  Widget vScrollBar;
  Widget drawAreaFrame;
  Widget drawArea;
  Cursor busyCursor, linkCursor, selectCursor;
  Cursor currentCursor;
  GC drawAreaGC;		// GC for blitting into drawArea

  static GString *currentSelection;  // selected text
  static XPDFCore *currentSelectionOwner;
  static Atom targetsAtom;

  GBool panning;
  int panMX, panMY;

  time_t modTime;		// last modification time of PDF file

  LinkAction *linkAction;	// mouse cursor is over this link

  XPDFUpdateCbk updateCbk;
  void *updateCbkData;
  XPDFActionCbk actionCbk;
  void *actionCbkData;
  XPDFKeyPressCbk keyPressCbk;
  void *keyPressCbkData;
  XPDFMouseCbk mouseCbk;
  void *mouseCbkData;

  GBool hyperlinksEnabled;
  GBool selectEnabled;

  int dialogDone;

  Widget passwordDialog;
  Widget passwordText;
  GString *password;
};

#endif
