//========================================================================
//
// CoreOutputDev.cc
//
// Copyright 2004 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "Object.h"
#include "TextOutputDev.h"
#include "CoreOutputDev.h"

//------------------------------------------------------------------------
// CoreOutputDev
//------------------------------------------------------------------------

CoreOutputDev::CoreOutputDev(SplashColorMode colorModeA, int bitmapRowPadA,
			     GBool reverseVideoA, SplashColorPtr paperColorA,
			     GBool incrementalUpdateA,
			     CoreOutRedrawCbk redrawCbkA,
			     void *redrawCbkDataA):
  SplashOutputDev(colorModeA, bitmapRowPadA, reverseVideoA, paperColorA)
{
  incrementalUpdate = incrementalUpdateA;
  redrawCbk = redrawCbkA;
  redrawCbkData = redrawCbkDataA;
}

CoreOutputDev::~CoreOutputDev() {
}

void CoreOutputDev::endPage() {
  SplashOutputDev::endPage();
  if (!incrementalUpdate) {
    (*redrawCbk)(redrawCbkData, 0, 0, getBitmapWidth(), getBitmapHeight(),
		 gTrue);
  }
}

void CoreOutputDev::dump() {
  int x0, y0, x1, y1;

  if (incrementalUpdate) {
    getModRegion(&x0, &y0, &x1, &y1);
    clearModRegion();
    if (x1 >= x0 && y1 >= y0) {
      (*redrawCbk)(redrawCbkData, x0, y0, x1, y1, gFalse);
    }
  }
}

void CoreOutputDev::clear() {
  startDoc(NULL);
  startPage(0, NULL);
}
