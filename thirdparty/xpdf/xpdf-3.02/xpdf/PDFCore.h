//========================================================================
//
// PDFCore.h
//
// Copyright 2004 Glyph & Cog, LLC
//
//========================================================================

#ifndef PDFCORE_H
#define PDFCORE_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdlib.h>
#include "SplashTypes.h"
#include "CharTypes.h"

class GString;
class GList;
class SplashBitmap;
class SplashPattern;
class BaseStream;
class PDFDoc;
class Links;
class LinkDest;
class LinkAction;
class TextPage;
class HighlightFile;
class CoreOutputDev;
class PDFCore;

//------------------------------------------------------------------------
// zoom factor
//------------------------------------------------------------------------

#define zoomPage  -1
#define zoomWidth -2
#define defZoom   125

//------------------------------------------------------------------------


//------------------------------------------------------------------------

// Number of pixels of matte color between pages in continuous mode.
#define continuousModePageSpacing 3

//------------------------------------------------------------------------
// PDFCorePage
//------------------------------------------------------------------------

class PDFCorePage {
public:

  PDFCorePage(int pageA, int wA, int hA, int tileWA, int tileHA);
  ~PDFCorePage();

  int page;
  GList *tiles;			// cached tiles [PDFCoreTile]
  int xDest, yDest;		// position of upper-left corner
				//   in the drawing area
  int w, h;			// size of whole page bitmap
  int tileW, tileH;		// size of tiles
  Links *links;			// hyperlinks for this page
  TextPage *text;		// extracted text
};

//------------------------------------------------------------------------
// PDFCoreTile
//------------------------------------------------------------------------

class PDFCoreTile {
public:

  PDFCoreTile(int xDestA, int yDestA);
  virtual ~PDFCoreTile();

  int xMin, yMin, xMax, yMax;
  int xDest, yDest;
  Guint edges;
  SplashBitmap *bitmap;
  double ctm[6];		// coordinate transform matrix:
				//   default user space -> device space
  double ictm[6];		// inverse CTM
};

#define pdfCoreTileTopEdge      0x01
#define pdfCoreTileBottomEdge   0x02
#define pdfCoreTileLeftEdge     0x04
#define pdfCoreTileRightEdge    0x08
#define pdfCoreTileTopSpace     0x10
#define pdfCoreTileBottomSpace  0x20

//------------------------------------------------------------------------
// PDFHistory
//------------------------------------------------------------------------

struct PDFHistory {
  GString *fileName;
  int page;
};

#define pdfHistorySize 50


//------------------------------------------------------------------------
// PDFCore
//------------------------------------------------------------------------

class PDFCore {
public:

  PDFCore(SplashColorMode colorModeA, int bitmapRowPadA,
	  GBool reverseVideoA, SplashColorPtr paperColorA,
	  GBool incrementalUpdate);
  virtual ~PDFCore();

  //----- loadFile / displayPage / displayDest

  // Load a new file.  Returns pdfOk or error code.
  virtual int loadFile(GString *fileName, GString *ownerPassword = NULL,
		       GString *userPassword = NULL);

#ifdef WIN32
  // Load a new file.  Returns pdfOk or error code.
  virtual int loadFile(wchar_t *fileName, int fileNameLen,
		       GString *ownerPassword = NULL,
		       GString *userPassword = NULL);
#endif

  // Load a new file, via a Stream instead of a file name.  Returns
  // pdfOk or error code.
  virtual int loadFile(BaseStream *stream, GString *ownerPassword = NULL,
		       GString *userPassword = NULL);

  // Load an already-created PDFDoc object.
  virtual void loadDoc(PDFDoc *docA);

  // Clear out the current document, if any.
  virtual void clear();

  // Same as clear(), but returns the PDFDoc object instead of
  // deleting it.
  virtual PDFDoc *takeDoc(GBool redraw);

  // Display (or redisplay) the specified page.  If <scrollToTop> is
  // set, the window is vertically scrolled to the top; otherwise, no
  // scrolling is done.  If <addToHist> is set, this page change is
  // added to the history list.
  virtual void displayPage(int topPageA, double zoomA, int rotateA,
			   GBool scrollToTop, GBool addToHist);

  // Display a link destination.
  virtual void displayDest(LinkDest *dest, double zoomA, int rotateA,
			   GBool addToHist);

  // Update the display, given the specified parameters.
  virtual void update(int topPageA, int scrollXA, int scrollYA,
		      double zoomA, int rotateA, GBool force, GBool addToHist);

  //----- page/position changes

  virtual GBool gotoNextPage(int inc, GBool top);
  virtual GBool gotoPrevPage(int dec, GBool top, GBool bottom);
  virtual GBool gotoNamedDestination(GString *dest);
  virtual GBool goForward();
  virtual GBool goBackward();
  virtual void scrollLeft(int nCols = 16);
  virtual void scrollRight(int nCols = 16);
  virtual void scrollUp(int nLines = 16);
  virtual void scrollUpPrevPage(int nLines = 16);
  virtual void scrollDown(int nLines = 16);
  virtual void scrollDownNextPage(int nLines = 16);
  virtual void scrollPageUp();
  virtual void scrollPageDown();
  virtual void scrollTo(int x, int y);
  virtual void scrollToLeftEdge();
  virtual void scrollToRightEdge();
  virtual void scrollToTopEdge();
  virtual void scrollToBottomEdge();
  virtual void scrollToTopLeft();
  virtual void scrollToBottomRight();
  virtual void zoomToRect(int pg, double ulx, double uly,
			  double lrx, double lry);
  virtual void zoomCentered(double zoomA);
  virtual void zoomToCurrentWidth();
  virtual void setContinuousMode(GBool cm);

  //----- selection

  // Current selected region.
  void setSelection(int newSelectPage,
		    int newSelectULX, int newSelectULY,
		    int newSelectLRX, int newSelectLRY);
  void moveSelection(int pg, int x, int y);
  GBool getSelection(int *pg, double *ulx, double *uly,
		     double *lrx, double *lry);

  // Text extraction.
  GString *extractText(int pg, double xMin, double yMin,
		       double xMax, double yMax);

  //----- find

  virtual GBool find(char *s, GBool caseSensitive, GBool next, GBool backward,
		     GBool onePageOnly);
  virtual GBool findU(Unicode *u, int len, GBool caseSensitive,
		      GBool next, GBool backward, GBool onePageOnly);


  //----- coordinate conversion

  // user space: per-pace, as defined by PDF file; unit = point
  // device space: (0,0) is upper-left corner of a page; unit = pixel
  // window space: (0,0) is upper-left corner of drawing area; unit = pixel

  GBool cvtWindowToUser(int xw, int yw, int *pg, double *xu, double *yu);
  GBool cvtWindowToDev(int xw, int yw, int *pg, int *xd, int *yd);
  void cvtUserToWindow(int pg, double xy, double yu, int *xw, int *yw);
  void cvtUserToDev(int pg, double xu, double yu, int *xd, int *yd);
  void cvtDevToWindow(int pg, int xd, int yd, int *xw, int *yw);
  void cvtDevToUser(int pg, int xd, int yd, double *xu, double *yu);

  //----- misc access

  PDFDoc *getDoc() { return doc; }
  int getPageNum() { return topPage; }
  double getZoom() { return zoom; }
  double getZoomDPI() { return dpi; }
  int getRotate() { return rotate; }
  GBool getContinuousMode() { return continuousMode; }
  virtual void setReverseVideo(GBool reverseVideoA);
  GBool canGoBack() { return historyBLen > 1; }
  GBool canGoForward() { return historyFLen > 0; }
  int getScrollX() { return scrollX; }
  int getScrollY() { return scrollY; }
  int getDrawAreaWidth() { return drawAreaWidth; }
  int getDrawAreaHeight() { return drawAreaHeight; }
  virtual void setBusyCursor(GBool busy) = 0;
  LinkAction *findLink(int pg, double x, double y);

protected:

  int loadFile2(PDFDoc *newDoc);
  void addPage(int pg, int rot);
  void needTile(PDFCorePage *page, int x, int y);
  void xorRectangle(int pg, int x0, int y0, int x1, int y1,
		    SplashPattern *pattern, PDFCoreTile *oneTile = NULL);
  int loadHighlightFile(HighlightFile *hf, SplashColorPtr color,
			SplashColorPtr selectColor, GBool selectable);
  PDFCorePage *findPage(int pg);
  static void redrawCbk(void *data, int x0, int y0, int x1, int y1,
			GBool composited);
  void redrawWindow(int x, int y, int width, int height,
		    GBool needUpdate);
  virtual PDFCoreTile *newTile(int xDestA, int yDestA);
  virtual void updateTileData(PDFCoreTile *tileA, int xSrc, int ySrc,
			      int width, int height, GBool composited);
  virtual void redrawRect(PDFCoreTile *tileA, int xSrc, int ySrc,
			  int xDest, int yDest, int width, int height,
			  GBool composited) = 0;
  void clippedRedrawRect(PDFCoreTile *tile, int xSrc, int ySrc,
			 int xDest, int yDest, int width, int height,
			 int xClip, int yClip, int wClip, int hClip,
			 GBool needUpdate, GBool composited = gTrue);
  virtual void updateScrollbars() = 0;
  virtual GBool checkForNewFile() { return gFalse; }

  PDFDoc *doc;			// current PDF file
  GBool continuousMode;		// false for single-page mode, true for
				//   continuous mode
  int drawAreaWidth,		// size of the PDF display area
      drawAreaHeight;
  double maxUnscaledPageW,	// maximum unscaled page size
         maxUnscaledPageH;
  int maxPageW;			// maximum page width (only used in
				//   continuous mode)
  int totalDocH;		// total document height (only used in
				//   continuous mode)
  int *pageY;			// top coordinates for each page (only used
				//   in continuous mode)
  int topPage;			// page at top of window
  int scrollX, scrollY;		// offset from top left corner of topPage
				//   to top left corner of window
  double zoom;			// current zoom level, in percent of 72 dpi
  double dpi;			// current zoom level, in DPI
  int rotate;			// current page rotation

  int selectPage;		// page number of current selection
  int selectULX,		// coordinates of current selection,
      selectULY,		//   in device space -- (ULX==LRX || ULY==LRY)
      selectLRX,		//   means there is no selection
      selectLRY;
  GBool dragging;		// set while selection is being dragged
  GBool lastDragLeft;		// last dragged selection edge was left/right
  GBool lastDragTop;		// last dragged selection edge was top/bottom

  PDFHistory			// page history queue
    history[pdfHistorySize];
  int historyCur;               // currently displayed page
  int historyBLen;              // number of valid entries backward from
                                //   current entry
  int historyFLen;              // number of valid entries forward from
                                //   current entry


  GList *pages;			// cached pages [PDFCorePage]
  PDFCoreTile *curTile;		// tile currently being rasterized
  PDFCorePage *curPage;		// page to which curTile belongs

  SplashColor paperColor;
  CoreOutputDev *out;

  friend class PDFCoreTile;
};

#endif
