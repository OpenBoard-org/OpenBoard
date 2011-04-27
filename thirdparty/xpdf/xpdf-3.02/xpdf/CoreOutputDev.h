//========================================================================
//
// CoreOutputDev.h
//
// Copyright 2004 Glyph & Cog, LLC
//
//========================================================================

#ifndef COREOUTPUTDEV_H
#define COREOUTPUTDEV_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "SplashTypes.h"
#include "SplashOutputDev.h"

class TextPage;

//------------------------------------------------------------------------

typedef void (*CoreOutRedrawCbk)(void *data, int x0, int y0, int x1, int y1,
				 GBool composited);

//------------------------------------------------------------------------
// CoreOutputDev
//------------------------------------------------------------------------

class CoreOutputDev: public SplashOutputDev {
public:

  CoreOutputDev(SplashColorMode colorModeA, int bitmapRowPadA,
		GBool reverseVideoA, SplashColorPtr paperColorA,
		GBool incrementalUpdateA,
		CoreOutRedrawCbk redrawCbkA,
		void *redrawCbkDataA);

  virtual ~CoreOutputDev();

  //----- initialization and control

  // End a page.
  virtual void endPage();

  // Dump page contents to display.
  virtual void dump();

  //----- special access

  // Clear out the document (used when displaying an empty window).
  void clear();

private:

  GBool incrementalUpdate;      // incrementally update the display?
  CoreOutRedrawCbk redrawCbk;
  void *redrawCbkData;
};

#endif
