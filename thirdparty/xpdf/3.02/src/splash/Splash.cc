//========================================================================
//
// Splash.cc
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include "gmem.h"
#include "SplashErrorCodes.h"
#include "SplashMath.h"
#include "SplashBitmap.h"
#include "SplashState.h"
#include "SplashPath.h"
#include "SplashXPath.h"
#include "SplashXPathScanner.h"
#include "SplashPattern.h"
#include "SplashScreen.h"
#include "SplashFont.h"
#include "SplashGlyphBitmap.h"
#include "Splash.h"

//------------------------------------------------------------------------

#define splashAAGamma 1.5

// distance of Bezier control point from center for circle approximation
// = (4 * (sqrt(2) - 1) / 3) * r
#define bezierCircle ((SplashCoord)0.55228475)
#define bezierCircle2 ((SplashCoord)(0.5 * 0.55228475))

// Divide a 16-bit value (in [0, 255*255]) by 255, returning an 8-bit result.
static inline Guchar div255(int x) {
  return (Guchar)((x + (x >> 8) + 0x80) >> 8);
}

// The PDF spec says that all pixels whose *centers* lie within the
// image target region get painted, so we want to round n+0.5 down to
// n.  But this causes problems, e.g., with PDF files that fill a
// rectangle with black and then draw an image to the exact same
// rectangle, so we instead use the fill scan conversion rule.
// However, the correct rule works better for glyphs, so we also
// provide that option in fillImageMask.
#if 0
static inline int imgCoordMungeLower(SplashCoord x) {
  return splashCeil(x + 0.5) - 1;
}
static inline int imgCoordMungeUpper(SplashCoord x) {
  return splashCeil(x + 0.5) - 1;
}
#else
static inline int imgCoordMungeLower(SplashCoord x) {
  return splashFloor(x);
}
static inline int imgCoordMungeUpper(SplashCoord x) {
  return splashFloor(x) + 1;
}
static inline int imgCoordMungeLowerC(SplashCoord x, GBool glyphMode) {
  return glyphMode ? (splashCeil(x + 0.5) - 1) : splashFloor(x);
}
static inline int imgCoordMungeUpperC(SplashCoord x, GBool glyphMode) {
  return glyphMode ? (splashCeil(x + 0.5) - 1) : (splashFloor(x) + 1);
}
#endif

// Used by drawImage and fillImageMask to divide the target
// quadrilateral into sections.
struct ImageSection {
  int y0, y1;				// actual y range
  int ia0, ia1;				// vertex indices for edge A
  int ib0, ib1;				// vertex indices for edge A
  SplashCoord xa0, ya0, xa1, ya1;	// edge A
  SplashCoord dxdya;			// slope of edge A
  SplashCoord xb0, yb0, xb1, yb1;	// edge B
  SplashCoord dxdyb;			// slope of edge B
};

//------------------------------------------------------------------------
// SplashPipe
//------------------------------------------------------------------------

#define splashPipeMaxStages 9

struct SplashPipe {
  // pixel coordinates
  int x, y;

  // source pattern
  SplashPattern *pattern;

  // source alpha and color
  Guchar aInput;
  GBool usesShape;
  SplashColorPtr cSrc;
  SplashColor cSrcVal;

  // non-isolated group alpha0
  Guchar *alpha0Ptr;

  // soft mask
  SplashColorPtr softMaskPtr;

  // destination alpha and color
  SplashColorPtr destColorPtr;
  int destColorMask;
  Guchar *destAlphaPtr;

  // shape
  Guchar shape;

  // result alpha and color
  GBool noTransparency;
  SplashPipeResultColorCtrl resultColorCtrl;

  // non-isolated group correction
  int nonIsolatedGroup;

  // the "run" function
  void (Splash::*run)(SplashPipe *pipe);
};

SplashPipeResultColorCtrl Splash::pipeResultColorNoAlphaBlend[] = {
  splashPipeResultColorNoAlphaBlendMono,
  splashPipeResultColorNoAlphaBlendMono,
  splashPipeResultColorNoAlphaBlendRGB,
  splashPipeResultColorNoAlphaBlendRGB
#if SPLASH_CMYK
  ,
  splashPipeResultColorNoAlphaBlendCMYK
#endif
};

SplashPipeResultColorCtrl Splash::pipeResultColorAlphaNoBlend[] = {
  splashPipeResultColorAlphaNoBlendMono,
  splashPipeResultColorAlphaNoBlendMono,
  splashPipeResultColorAlphaNoBlendRGB,
  splashPipeResultColorAlphaNoBlendRGB
#if SPLASH_CMYK
  ,
  splashPipeResultColorAlphaNoBlendCMYK
#endif
};

SplashPipeResultColorCtrl Splash::pipeResultColorAlphaBlend[] = {
  splashPipeResultColorAlphaBlendMono,
  splashPipeResultColorAlphaBlendMono,
  splashPipeResultColorAlphaBlendRGB,
  splashPipeResultColorAlphaBlendRGB
#if SPLASH_CMYK
  ,
  splashPipeResultColorAlphaBlendCMYK
#endif
};

//------------------------------------------------------------------------

static void blendXor(SplashColorPtr src, SplashColorPtr dest,
		     SplashColorPtr blend, SplashColorMode cm) {
  int i;

  for (i = 0; i < splashColorModeNComps[cm]; ++i) {
    blend[i] = src[i] ^ dest[i];
  }
}

//------------------------------------------------------------------------
// modified region
//------------------------------------------------------------------------

void Splash::clearModRegion() {
  modXMin = bitmap->getWidth();
  modYMin = bitmap->getHeight();
  modXMax = -1;
  modYMax = -1;
}

inline void Splash::updateModX(int x) {
  if (x < modXMin) {
    modXMin = x;
  }
  if (x > modXMax) {
    modXMax = x;
  }
}

inline void Splash::updateModY(int y) {
  if (y < modYMin) {
    modYMin = y;
  }
  if (y > modYMax) {
    modYMax = y;
  }
}

//------------------------------------------------------------------------
// pipeline
//------------------------------------------------------------------------

inline void Splash::pipeInit(SplashPipe *pipe, int x, int y,
			     SplashPattern *pattern, SplashColorPtr cSrc,
			     Guchar aInput, GBool usesShape,
			     GBool nonIsolatedGroup) {
  pipeSetXY(pipe, x, y);
  pipe->pattern = NULL;

  // source color
  if (pattern) {
    if (pattern->isStatic()) {
      pattern->getColor(x, y, pipe->cSrcVal);
    } else {
      pipe->pattern = pattern;
    }
    pipe->cSrc = pipe->cSrcVal;
  } else {
    pipe->cSrc = cSrc;
  }

  // source alpha
  pipe->aInput = aInput;
  pipe->usesShape = usesShape;

  // result alpha
  if (aInput == 255 && !state->softMask && !usesShape &&
      !state->inNonIsolatedGroup) {
    pipe->noTransparency = gTrue;
  } else {
    pipe->noTransparency = gFalse;
  }

  // result color
  if (pipe->noTransparency) {
    // the !state->blendFunc case is handled separately in pipeRun
    pipe->resultColorCtrl = pipeResultColorNoAlphaBlend[bitmap->mode];
  } else if (!state->blendFunc) {
    pipe->resultColorCtrl = pipeResultColorAlphaNoBlend[bitmap->mode];
  } else {
    pipe->resultColorCtrl = pipeResultColorAlphaBlend[bitmap->mode];
  }

  // non-isolated group correction
  if (nonIsolatedGroup) {
    pipe->nonIsolatedGroup = splashColorModeNComps[bitmap->mode];
  } else {
    pipe->nonIsolatedGroup = 0;
  }

  // select the 'run' function
  pipe->run = &Splash::pipeRun;
  if (!pipe->pattern && pipe->noTransparency && !state->blendFunc) {
    if (bitmap->mode == splashModeMono1 && !pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunSimpleMono1;
    } else if (bitmap->mode == splashModeMono8 && pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunSimpleMono8;
    } else if (bitmap->mode == splashModeRGB8 && pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunSimpleRGB8;
    } else if (bitmap->mode == splashModeBGR8 && pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunSimpleBGR8;
#if SPLASH_CMYK
    } else if (bitmap->mode == splashModeCMYK8 && pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunSimpleCMYK8;
#endif
    }
  } else if (!pipe->pattern && !pipe->noTransparency && !state->softMask &&
	     pipe->usesShape &&
	     !(state->inNonIsolatedGroup && alpha0Bitmap->alpha) &&
	     !state->blendFunc && pipe->nonIsolatedGroup == 0) {
    if (bitmap->mode == splashModeMono1 && !pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunAAMono1;
    } else if (bitmap->mode == splashModeMono8 && pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunAAMono8;
    } else if (bitmap->mode == splashModeRGB8 && pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunAARGB8;
    } else if (bitmap->mode == splashModeBGR8 && pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunAABGR8;
#if SPLASH_CMYK
    } else if (bitmap->mode == splashModeCMYK8 && pipe->destAlphaPtr) {
      pipe->run = &Splash::pipeRunAACMYK8;
#endif
    }
  }
}

// general case
void Splash::pipeRun(SplashPipe *pipe) {
  Guchar aSrc, aDest, alpha2, alpha0, aResult;
  SplashColor cSrc2, cDest, cDest2, cBlend;
  Guchar cResult0, cResult1, cResult2, cResult3;

  //----- source color

  // static pattern: handled in pipeInit
  // fixed color: handled in pipeInit

  // dynamic pattern
  if (pipe->pattern) {
    pipe->pattern->getColor(pipe->x, pipe->y, pipe->cSrcVal);
  }

  if (pipe->noTransparency && !state->blendFunc) {

    //----- write destination pixel

    switch (bitmap->mode) {
    case splashModeMono1:
      cResult0 = pipe->cSrc[0];
      if (state->screen->test(pipe->x, pipe->y, cResult0)) {
	*pipe->destColorPtr |= pipe->destColorMask;
      } else {
	*pipe->destColorPtr &= ~pipe->destColorMask;
      }
      if (!(pipe->destColorMask >>= 1)) {
	pipe->destColorMask = 0x80;
	++pipe->destColorPtr;
      }
      break;
    case splashModeMono8:
      *pipe->destColorPtr++ = pipe->cSrc[0];
      break;
    case splashModeRGB8:
      *pipe->destColorPtr++ = pipe->cSrc[0];
      *pipe->destColorPtr++ = pipe->cSrc[1];
      *pipe->destColorPtr++ = pipe->cSrc[2];
      break;
    case splashModeBGR8:
      *pipe->destColorPtr++ = pipe->cSrc[2];
      *pipe->destColorPtr++ = pipe->cSrc[1];
      *pipe->destColorPtr++ = pipe->cSrc[0];
      break;
#if SPLASH_CMYK
    case splashModeCMYK8:
      if (state->overprintMask & 1) {
	pipe->destColorPtr[0] = pipe->cSrc[0];
      }
      if (state->overprintMask & 2) {
	pipe->destColorPtr[1] = pipe->cSrc[1];
      }
      if (state->overprintMask & 4) {
	pipe->destColorPtr[2] = pipe->cSrc[2];
      }
      if (state->overprintMask & 8) {
	pipe->destColorPtr[3] = pipe->cSrc[3];
      }
      pipe->destColorPtr += 4;
      break;
#endif
    }
    if (pipe->destAlphaPtr) {
      *pipe->destAlphaPtr++ = 255;
    }

  } else {

    //----- read destination pixel

    switch (bitmap->mode) {
    case splashModeMono1:
      cDest[0] = (*pipe->destColorPtr & pipe->destColorMask) ? 0xff : 0x00;
      break;
    case splashModeMono8:
      cDest[0] = *pipe->destColorPtr;
      break;
    case splashModeRGB8:
      cDest[0] = pipe->destColorPtr[0];
      cDest[1] = pipe->destColorPtr[1];
      cDest[2] = pipe->destColorPtr[2];
      break;
    case splashModeBGR8:
      cDest[0] = pipe->destColorPtr[2];
      cDest[1] = pipe->destColorPtr[1];
      cDest[2] = pipe->destColorPtr[0];
      break;
#if SPLASH_CMYK
    case splashModeCMYK8:
      cDest[0] = pipe->destColorPtr[0];
      cDest[1] = pipe->destColorPtr[1];
      cDest[2] = pipe->destColorPtr[2];
      cDest[3] = pipe->destColorPtr[3];
      break;
#endif
    }
    if (pipe->destAlphaPtr) {
      aDest = *pipe->destAlphaPtr;
    } else {
      aDest = 0xff;
    }

    //----- blend function

    if (state->blendFunc) {
#if SPLASH_CMYK
      if (bitmap->mode == splashModeCMYK8) {
	// convert colors to additive
	cSrc2[0] = 0xff - pipe->cSrc[0];
	cSrc2[1] = 0xff - pipe->cSrc[1];
	cSrc2[2] = 0xff - pipe->cSrc[2];
	cSrc2[3] = 0xff - pipe->cSrc[3];
	cDest2[0] = 0xff - cDest[0];
	cDest2[1] = 0xff - cDest[1];
	cDest2[2] = 0xff - cDest[2];
	cDest2[3] = 0xff - cDest[3];
	(*state->blendFunc)(cSrc2, cDest2, cBlend, bitmap->mode);
	// convert result back to subtractive
	cBlend[0] = 0xff - cBlend[0];
	cBlend[1] = 0xff - cBlend[1];
	cBlend[2] = 0xff - cBlend[2];
	cBlend[3] = 0xff - cBlend[3];
      } else
#endif
      (*state->blendFunc)(pipe->cSrc, cDest, cBlend, bitmap->mode);
    }

    //----- source alpha

    if (state->softMask) {
      if (pipe->usesShape) {
	aSrc = div255(div255(pipe->aInput * *pipe->softMaskPtr++) *
		      pipe->shape);
      } else {
	aSrc = div255(pipe->aInput * *pipe->softMaskPtr++);
      }
    } else if (pipe->usesShape) {
      aSrc = div255(pipe->aInput * pipe->shape);
    } else {
      aSrc = pipe->aInput;
    }

    //----- result alpha and non-isolated group element correction

    if (pipe->noTransparency) {
      alpha2 = aResult = 255;
    } else {
      aResult = aSrc + aDest - div255(aSrc * aDest);

      if (pipe->alpha0Ptr) {
	alpha0 = *pipe->alpha0Ptr++;
	alpha2 = aResult + alpha0 - div255(aResult * alpha0);
      } else {
	alpha2 = aResult;
      }
    }

    //----- result color

    cResult0 = cResult1 = cResult2 = cResult3 = 0; // make gcc happy

    switch (pipe->resultColorCtrl) {

#if SPLASH_CMYK
    case splashPipeResultColorNoAlphaBlendCMYK:
      cResult3 = div255((255 - aDest) * pipe->cSrc[3] + aDest * cBlend[3]);
#endif
    case splashPipeResultColorNoAlphaBlendRGB:
      cResult2 = div255((255 - aDest) * pipe->cSrc[2] + aDest * cBlend[2]);
      cResult1 = div255((255 - aDest) * pipe->cSrc[1] + aDest * cBlend[1]);
    case splashPipeResultColorNoAlphaBlendMono:
      cResult0 = div255((255 - aDest) * pipe->cSrc[0] + aDest * cBlend[0]);
      break;

    case splashPipeResultColorAlphaNoBlendMono:
      if (alpha2 == 0) {
	cResult0 = 0;
      } else {
	cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			     aSrc * pipe->cSrc[0]) / alpha2);
      }
      break;
    case splashPipeResultColorAlphaNoBlendRGB:
      if (alpha2 == 0) {
	cResult0 = 0;
	cResult1 = 0;
	cResult2 = 0;
      } else {
	cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			     aSrc * pipe->cSrc[0]) / alpha2);
	cResult1 = (Guchar)(((alpha2 - aSrc) * cDest[1] +
			     aSrc * pipe->cSrc[1]) / alpha2);
	cResult2 = (Guchar)(((alpha2 - aSrc) * cDest[2] +
			     aSrc * pipe->cSrc[2]) / alpha2);
      }
      break;
#if SPLASH_CMYK
    case splashPipeResultColorAlphaNoBlendCMYK:
      if (alpha2 == 0) {
	cResult0 = 0;
	cResult1 = 0;
	cResult2 = 0;
	cResult3 = 0;
      } else {
	cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			     aSrc * pipe->cSrc[0]) / alpha2);
	cResult1 = (Guchar)(((alpha2 - aSrc) * cDest[1] +
			     aSrc * pipe->cSrc[1]) / alpha2);
	cResult2 = (Guchar)(((alpha2 - aSrc) * cDest[2] +
			     aSrc * pipe->cSrc[2]) / alpha2);
	cResult3 = (Guchar)(((alpha2 - aSrc) * cDest[3] +
			     aSrc * pipe->cSrc[3]) / alpha2);
      }
      break;
#endif

    case splashPipeResultColorAlphaBlendMono:
      if (alpha2 == 0) {
	cResult0 = 0;
      } else {
	cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			     aSrc * ((255 - aDest) * pipe->cSrc[0] +
				     aDest * cBlend[0]) / 255) /
			    alpha2);
      }
      break;
    case splashPipeResultColorAlphaBlendRGB:
      if (alpha2 == 0) {
	cResult0 = 0;
	cResult1 = 0;
	cResult2 = 0;
      } else {
	cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			     aSrc * ((255 - aDest) * pipe->cSrc[0] +
				     aDest * cBlend[0]) / 255) /
			    alpha2);
	cResult1 = (Guchar)(((alpha2 - aSrc) * cDest[1] +
			     aSrc * ((255 - aDest) * pipe->cSrc[1] +
				     aDest * cBlend[1]) / 255) /
			    alpha2);
	cResult2 = (Guchar)(((alpha2 - aSrc) * cDest[2] +
			     aSrc * ((255 - aDest) * pipe->cSrc[2] +
				     aDest * cBlend[2]) / 255) /
			    alpha2);
      }
      break;
#if SPLASH_CMYK
    case splashPipeResultColorAlphaBlendCMYK:
      if (alpha2 == 0) {
	cResult0 = 0;
	cResult1 = 0;
	cResult2 = 0;
	cResult3 = 0;
      } else {
	cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			     aSrc * ((255 - aDest) * pipe->cSrc[0] +
				     aDest * cBlend[0]) / 255) /
			    alpha2);
	cResult1 = (Guchar)(((alpha2 - aSrc) * cDest[1] +
			     aSrc * ((255 - aDest) * pipe->cSrc[1] +
				     aDest * cBlend[1]) / 255) /
			    alpha2);
	cResult2 = (Guchar)(((alpha2 - aSrc) * cDest[2] +
			     aSrc * ((255 - aDest) * pipe->cSrc[2] +
				     aDest * cBlend[2]) / 255) /
			    alpha2);
	cResult3 = (Guchar)(((alpha2 - aSrc) * cDest[3] +
			     aSrc * ((255 - aDest) * pipe->cSrc[3] +
				     aDest * cBlend[3]) / 255) /
			    alpha2);
      }
      break;
#endif
    }

    //----- non-isolated group correction

    if (aResult != 0) {
      switch (pipe->nonIsolatedGroup) {
#if SPLASH_CMYK
      case 4:
	cResult3 += (cResult3 - cDest[3]) * aDest *
	            (255 - aResult) / (255 * aResult);
#endif
      case 3:
	cResult2 += (cResult2 - cDest[2]) * aDest *
	            (255 - aResult) / (255 * aResult);
	cResult1 += (cResult1 - cDest[1]) * aDest *
	            (255 - aResult) / (255 * aResult);
      case 1:
	cResult0 += (cResult0 - cDest[0]) * aDest *
	            (255 - aResult) / (255 * aResult);
      case 0:
	break;
      }
    }

    //----- write destination pixel

    switch (bitmap->mode) {
    case splashModeMono1:
      if (state->screen->test(pipe->x, pipe->y, cResult0)) {
	*pipe->destColorPtr |= pipe->destColorMask;
      } else {
	*pipe->destColorPtr &= ~pipe->destColorMask;
      }
      if (!(pipe->destColorMask >>= 1)) {
	pipe->destColorMask = 0x80;
	++pipe->destColorPtr;
      }
      break;
    case splashModeMono8:
      *pipe->destColorPtr++ = cResult0;
      break;
    case splashModeRGB8:
      *pipe->destColorPtr++ = cResult0;
      *pipe->destColorPtr++ = cResult1;
      *pipe->destColorPtr++ = cResult2;
      break;
    case splashModeBGR8:
      *pipe->destColorPtr++ = cResult2;
      *pipe->destColorPtr++ = cResult1;
      *pipe->destColorPtr++ = cResult0;
      break;
#if SPLASH_CMYK
    case splashModeCMYK8:
      if (state->overprintMask & 1) {
	pipe->destColorPtr[0] = cResult0;
      }
      if (state->overprintMask & 2) {
	pipe->destColorPtr[1] = cResult1;
      }
      if (state->overprintMask & 4) {
	pipe->destColorPtr[2] = cResult2;
      }
      if (state->overprintMask & 8) {
	pipe->destColorPtr[3] = cResult3;
      }
      pipe->destColorPtr += 4;
      break;
#endif
    }
    if (pipe->destAlphaPtr) {
      *pipe->destAlphaPtr++ = aResult;
    }

  }

  ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeMono1 && !pipe->destAlphaPtr) {
void Splash::pipeRunSimpleMono1(SplashPipe *pipe) {
  Guchar cResult0;

  //----- write destination pixel
  cResult0 = pipe->cSrc[0];
  if (state->screen->test(pipe->x, pipe->y, cResult0)) {
    *pipe->destColorPtr |= pipe->destColorMask;
  } else {
    *pipe->destColorPtr &= ~pipe->destColorMask;
  }
  if (!(pipe->destColorMask >>= 1)) {
    pipe->destColorMask = 0x80;
    ++pipe->destColorPtr;
  }

  ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeMono8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleMono8(SplashPipe *pipe) {
  //----- write destination pixel
  *pipe->destColorPtr++ = pipe->cSrc[0];
  *pipe->destAlphaPtr++ = 255;

  ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeRGB8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleRGB8(SplashPipe *pipe) {
  //----- write destination pixel
  *pipe->destColorPtr++ = pipe->cSrc[0];
  *pipe->destColorPtr++ = pipe->cSrc[1];
  *pipe->destColorPtr++ = pipe->cSrc[2];
  *pipe->destAlphaPtr++ = 255;

  ++pipe->x;
}

// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeBGR8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleBGR8(SplashPipe *pipe) {
  //----- write destination pixel
  *pipe->destColorPtr++ = pipe->cSrc[2];
  *pipe->destColorPtr++ = pipe->cSrc[1];
  *pipe->destColorPtr++ = pipe->cSrc[0];
  *pipe->destAlphaPtr++ = 255;

  ++pipe->x;
}

#if SPLASH_CMYK
// special case:
// !pipe->pattern && pipe->noTransparency && !state->blendFunc &&
// bitmap->mode == splashModeCMYK8 && pipe->destAlphaPtr) {
void Splash::pipeRunSimpleCMYK8(SplashPipe *pipe) {
  //----- write destination pixel
  if (state->overprintMask & 1) {
    pipe->destColorPtr[0] = pipe->cSrc[0];
  }
  if (state->overprintMask & 2) {
    pipe->destColorPtr[1] = pipe->cSrc[1];
  }
  if (state->overprintMask & 4) {
    pipe->destColorPtr[2] = pipe->cSrc[2];
  }
  if (state->overprintMask & 8) {
    pipe->destColorPtr[3] = pipe->cSrc[3];
  }
  pipe->destColorPtr += 4;
  *pipe->destAlphaPtr++ = 255;

  ++pipe->x;
}
#endif


// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// pipe->nonIsolatedGroup == 0 &&
// bitmap->mode == splashModeMono1 && !pipe->destAlphaPtr
void Splash::pipeRunAAMono1(SplashPipe *pipe) {
  Guchar aSrc;
  SplashColor cDest;
  Guchar cResult0;

  //----- read destination pixel
  cDest[0] = (*pipe->destColorPtr & pipe->destColorMask) ? 0xff : 0x00;

  //----- source alpha
  aSrc = div255(pipe->aInput * pipe->shape);

  //----- result color
  // note: aDest = alpha2 = aResult = 0xff
  cResult0 = (Guchar)div255((0xff - aSrc) * cDest[0] + aSrc * pipe->cSrc[0]);

  //----- write destination pixel
  if (state->screen->test(pipe->x, pipe->y, cResult0)) {
    *pipe->destColorPtr |= pipe->destColorMask;
  } else {
    *pipe->destColorPtr &= ~pipe->destColorMask;
  }
  if (!(pipe->destColorMask >>= 1)) {
    pipe->destColorMask = 0x80;
    ++pipe->destColorPtr;
  }

  ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// pipe->nonIsolatedGroup == 0 &&
// bitmap->mode == splashModeMono8 && pipe->destAlphaPtr
void Splash::pipeRunAAMono8(SplashPipe *pipe) {
  Guchar aSrc, aDest, alpha2, aResult;
  SplashColor cDest;
  Guchar cResult0;

  //----- read destination pixel
  cDest[0] = *pipe->destColorPtr;
  aDest = *pipe->destAlphaPtr;

  //----- source alpha
  aSrc = div255(pipe->aInput * pipe->shape);

  //----- result alpha and non-isolated group element correction
  aResult = aSrc + aDest - div255(aSrc * aDest);
  alpha2 = aResult;

  //----- result color
  if (alpha2 == 0) {
    cResult0 = 0;
  } else {
    cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			 aSrc * pipe->cSrc[0]) / alpha2);
  }

  //----- write destination pixel
  *pipe->destColorPtr++ = cResult0;
  *pipe->destAlphaPtr++ = aResult;

  ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// pipe->nonIsolatedGroup == 0 &&
// bitmap->mode == splashModeRGB8 && pipe->destAlphaPtr
void Splash::pipeRunAARGB8(SplashPipe *pipe) {
  Guchar aSrc, aDest, alpha2, aResult;
  SplashColor cDest;
  Guchar cResult0, cResult1, cResult2;

  //----- read destination pixel
  cDest[0] = pipe->destColorPtr[0];
  cDest[1] = pipe->destColorPtr[1];
  cDest[2] = pipe->destColorPtr[2];
  aDest = *pipe->destAlphaPtr;

  //----- source alpha
  aSrc = div255(pipe->aInput * pipe->shape);

  //----- result alpha and non-isolated group element correction
  aResult = aSrc + aDest - div255(aSrc * aDest);
  alpha2 = aResult;

  //----- result color
  if (alpha2 == 0) {
    cResult0 = 0;
    cResult1 = 0;
    cResult2 = 0;
  } else {
    cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			 aSrc * pipe->cSrc[0]) / alpha2);
    cResult1 = (Guchar)(((alpha2 - aSrc) * cDest[1] +
			 aSrc * pipe->cSrc[1]) / alpha2);
    cResult2 = (Guchar)(((alpha2 - aSrc) * cDest[2] +
			 aSrc * pipe->cSrc[2]) / alpha2);
  }

  //----- write destination pixel
  *pipe->destColorPtr++ = cResult0;
  *pipe->destColorPtr++ = cResult1;
  *pipe->destColorPtr++ = cResult2;
  *pipe->destAlphaPtr++ = aResult;

  ++pipe->x;
}

// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// pipe->nonIsolatedGroup == 0 &&
// bitmap->mode == splashModeBGR8 && pipe->destAlphaPtr
void Splash::pipeRunAABGR8(SplashPipe *pipe) {
  Guchar aSrc, aDest, alpha2, aResult;
  SplashColor cDest;
  Guchar cResult0, cResult1, cResult2;

  //----- read destination pixel
  cDest[0] = pipe->destColorPtr[2];
  cDest[1] = pipe->destColorPtr[1];
  cDest[2] = pipe->destColorPtr[0];
  aDest = *pipe->destAlphaPtr;

  //----- source alpha
  aSrc = div255(pipe->aInput * pipe->shape);

  //----- result alpha and non-isolated group element correction
  aResult = aSrc + aDest - div255(aSrc * aDest);
  alpha2 = aResult;

  //----- result color
  if (alpha2 == 0) {
    cResult0 = 0;
    cResult1 = 0;
    cResult2 = 0;
  } else {
    cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			 aSrc * pipe->cSrc[0]) / alpha2);
    cResult1 = (Guchar)(((alpha2 - aSrc) * cDest[1] +
			 aSrc * pipe->cSrc[1]) / alpha2);
    cResult2 = (Guchar)(((alpha2 - aSrc) * cDest[2] +
			 aSrc * pipe->cSrc[2]) / alpha2);
  }

  //----- write destination pixel
  *pipe->destColorPtr++ = cResult2;
  *pipe->destColorPtr++ = cResult1;
  *pipe->destColorPtr++ = cResult0;
  *pipe->destAlphaPtr++ = aResult;

  ++pipe->x;
}

#if SPLASH_CMYK
// special case:
// !pipe->pattern && !pipe->noTransparency && !state->softMask &&
// pipe->usesShape && !pipe->alpha0Ptr && !state->blendFunc &&
// pipe->nonIsolatedGroup == 0 &&
// bitmap->mode == splashModeCMYK8 && pipe->destAlphaPtr
void Splash::pipeRunAACMYK8(SplashPipe *pipe) {
  Guchar aSrc, aDest, alpha2, aResult;
  SplashColor cDest;
  Guchar cResult0, cResult1, cResult2, cResult3;

  //----- read destination pixel
  cDest[0] = pipe->destColorPtr[0];
  cDest[1] = pipe->destColorPtr[1];
  cDest[2] = pipe->destColorPtr[2];
  cDest[3] = pipe->destColorPtr[3];
  aDest = *pipe->destAlphaPtr;

  //----- source alpha
  aSrc = div255(pipe->aInput * pipe->shape);

  //----- result alpha and non-isolated group element correction
  aResult = aSrc + aDest - div255(aSrc * aDest);
  alpha2 = aResult;

  //----- result color
  if (alpha2 == 0) {
    cResult0 = 0;
    cResult1 = 0;
    cResult2 = 0;
    cResult3 = 0;
  } else {
    cResult0 = (Guchar)(((alpha2 - aSrc) * cDest[0] +
			 aSrc * pipe->cSrc[0]) / alpha2);
    cResult1 = (Guchar)(((alpha2 - aSrc) * cDest[1] +
			 aSrc * pipe->cSrc[1]) / alpha2);
    cResult2 = (Guchar)(((alpha2 - aSrc) * cDest[2] +
			 aSrc * pipe->cSrc[2]) / alpha2);
    cResult3 = (Guchar)(((alpha2 - aSrc) * cDest[3] +
			 aSrc * pipe->cSrc[3]) / alpha2);
  }

  //----- write destination pixel
  if (state->overprintMask & 1) {
    pipe->destColorPtr[0] = cResult0;
  }
  if (state->overprintMask & 2) {
    pipe->destColorPtr[1] = cResult1;
  }
  if (state->overprintMask & 4) {
    pipe->destColorPtr[2] = cResult2;
  }
  if (state->overprintMask & 8) {
    pipe->destColorPtr[3] = cResult3;
  }
  pipe->destColorPtr += 4;
  *pipe->destAlphaPtr++ = aResult;

  ++pipe->x;
}
#endif


inline void Splash::pipeSetXY(SplashPipe *pipe, int x, int y) {
  pipe->x = x;
  pipe->y = y;
  if (state->softMask) {
    pipe->softMaskPtr =
        &state->softMask->data[y * state->softMask->rowSize + x];
  }
  switch (bitmap->mode) {
  case splashModeMono1:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + (x >> 3)];
    pipe->destColorMask = 0x80 >> (x & 7);
    break;
  case splashModeMono8:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + x];
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + 3 * x];
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    pipe->destColorPtr = &bitmap->data[y * bitmap->rowSize + 4 * x];
    break;
#endif
  }
  if (bitmap->alpha) {
    pipe->destAlphaPtr = &bitmap->alpha[y * bitmap->width + x];
  } else {
    pipe->destAlphaPtr = NULL;
  }
  if (state->inNonIsolatedGroup && alpha0Bitmap->alpha) {
    pipe->alpha0Ptr =
        &alpha0Bitmap->alpha[(alpha0Y + y) * alpha0Bitmap->width +
			     (alpha0X + x)];
  } else {
    pipe->alpha0Ptr = NULL;
  }
}

inline void Splash::pipeIncX(SplashPipe *pipe) {
  ++pipe->x;
  if (state->softMask) {
    ++pipe->softMaskPtr;
  }
  switch (bitmap->mode) {
  case splashModeMono1:
    if (!(pipe->destColorMask >>= 1)) {
      pipe->destColorMask = 0x80;
      ++pipe->destColorPtr;
    }
    break;
  case splashModeMono8:
    ++pipe->destColorPtr;
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    pipe->destColorPtr += 3;
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    pipe->destColorPtr += 4;
    break;
#endif
  }
  if (pipe->destAlphaPtr) {
    ++pipe->destAlphaPtr;
  }
  if (pipe->alpha0Ptr) {
    ++pipe->alpha0Ptr;
  }
}

inline void Splash::drawPixel(SplashPipe *pipe, int x, int y, GBool noClip) {
  if (noClip || state->clip->test(x, y)) {
    pipeSetXY(pipe, x, y);
    (this->*pipe->run)(pipe);
    updateModX(x);
    updateModY(y);
  }
}

inline void Splash::drawAAPixelInit() {
  aaBufY = -1;
}

inline void Splash::drawAAPixel(SplashPipe *pipe, int x, int y) {
#if splashAASize == 4
  static int bitCount4[16] = { 0, 1, 1, 2, 1, 2, 2, 3,
			       1, 2, 2, 3, 2, 3, 3, 4 };
  int w;
#else
  int xx, yy;
#endif
  SplashColorPtr p;
  int x0, x1, t;

  if (x < 0 || x >= bitmap->width ||
      y < state->clip->getYMinI() || y > state->clip->getYMaxI()) {
    return;
  }

  // update aaBuf
  if (y != aaBufY) {
    memset(aaBuf->getDataPtr(), 0xff,
	   aaBuf->getRowSize() * aaBuf->getHeight());
    x0 = 0;
    x1 = bitmap->width - 1;
    state->clip->clipAALine(aaBuf, &x0, &x1, y);
    aaBufY = y;
  }

  // compute the shape value
#if splashAASize == 4
  p = aaBuf->getDataPtr() + (x >> 1);
  w = aaBuf->getRowSize();
  if (x & 1) {
    t = bitCount4[*p & 0x0f] + bitCount4[p[w] & 0x0f] +
        bitCount4[p[2*w] & 0x0f] + bitCount4[p[3*w] & 0x0f];
  } else {
    t = bitCount4[*p >> 4] + bitCount4[p[w] >> 4] +
        bitCount4[p[2*w] >> 4] + bitCount4[p[3*w] >> 4];
  }
#else
  t = 0;
  for (yy = 0; yy < splashAASize; ++yy) {
    for (xx = 0; xx < splashAASize; ++xx) {
      p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() +
	  ((x * splashAASize + xx) >> 3);
      t += (*p >> (7 - ((x * splashAASize + xx) & 7))) & 1;
    }
  }
#endif

  // draw the pixel
  if (t != 0) {
    pipeSetXY(pipe, x, y);
    pipe->shape = div255(aaGamma[t] * pipe->shape);
    (this->*pipe->run)(pipe);
    updateModX(x);
    updateModY(y);
  }
}

inline void Splash::drawSpan(SplashPipe *pipe, int x0, int x1, int y,
			     GBool noClip) {
  int x;

  pipeSetXY(pipe, x0, y);
  if (noClip) {
    for (x = x0; x <= x1; ++x) {
      (this->*pipe->run)(pipe);
    }
    updateModX(x0);
    updateModX(x1);
    updateModY(y);
  } else {
    for (x = x0; x <= x1; ++x) {
      if (state->clip->test(x, y)) {
	(this->*pipe->run)(pipe);
	updateModX(x);
	updateModY(y);
      } else {
	pipeIncX(pipe);
      }
    }
  }
}

inline void Splash::drawAALine(SplashPipe *pipe, int x0, int x1, int y) {
#if splashAASize == 4
  static int bitCount4[16] = { 0, 1, 1, 2, 1, 2, 2, 3,
			       1, 2, 2, 3, 2, 3, 3, 4 };
  SplashColorPtr p0, p1, p2, p3;
  int t;
#else
  SplashColorPtr p;
  int xx, yy, t;
#endif
  int x;

#if splashAASize == 4
  p0 = aaBuf->getDataPtr() + (x0 >> 1);
  p1 = p0 + aaBuf->getRowSize();
  p2 = p1 + aaBuf->getRowSize();
  p3 = p2 + aaBuf->getRowSize();
#endif
  pipeSetXY(pipe, x0, y);
  for (x = x0; x <= x1; ++x) {

    // compute the shape value
#if splashAASize == 4
    if (x & 1) {
      t = bitCount4[*p0 & 0x0f] + bitCount4[*p1 & 0x0f] +
	  bitCount4[*p2 & 0x0f] + bitCount4[*p3 & 0x0f];
      ++p0; ++p1; ++p2; ++p3;
    } else {
      t = bitCount4[*p0 >> 4] + bitCount4[*p1 >> 4] +
	  bitCount4[*p2 >> 4] + bitCount4[*p3 >> 4];
    }
#else
    t = 0;
    for (yy = 0; yy < splashAASize; ++yy) {
      for (xx = 0; xx < splashAASize; ++xx) {
	p = aaBuf->getDataPtr() + yy * aaBuf->getRowSize() +
	    ((x * splashAASize + xx) >> 3);
	t += (*p >> (7 - ((x * splashAASize + xx) & 7))) & 1;
      }
    }
#endif

    if (t != 0) {
      pipe->shape = aaGamma[t];
      (this->*pipe->run)(pipe);
      updateModX(x);
      updateModY(y);
    } else {
      pipeIncX(pipe);
    }
  }
}

//------------------------------------------------------------------------

// Transform a point from user space to device space.
inline void Splash::transform(SplashCoord *matrix,
			      SplashCoord xi, SplashCoord yi,
			      SplashCoord *xo, SplashCoord *yo) {
  //                          [ m[0] m[1] 0 ]
  // [xo yo 1] = [xi yi 1] *  [ m[2] m[3] 0 ]
  //                          [ m[4] m[5] 1 ]
  *xo = xi * matrix[0] + yi * matrix[2] + matrix[4];
  *yo = xi * matrix[1] + yi * matrix[3] + matrix[5];
}

//------------------------------------------------------------------------
// Splash
//------------------------------------------------------------------------

Splash::Splash(SplashBitmap *bitmapA, GBool vectorAntialiasA,
	       SplashScreenParams *screenParams) {
  int i;

  bitmap = bitmapA;
  vectorAntialias = vectorAntialiasA;
  inShading = gFalse;
  state = new SplashState(bitmap->width, bitmap->height, vectorAntialias,
			  screenParams);
  if (vectorAntialias) {
    aaBuf = new SplashBitmap(splashAASize * bitmap->width, splashAASize,
			     1, splashModeMono1, gFalse);
    for (i = 0; i <= splashAASize * splashAASize; ++i) {
      aaGamma[i] = (Guchar)splashRound(
		       splashPow((SplashCoord)i /
				 (SplashCoord)(splashAASize * splashAASize),
				 splashAAGamma) * 255);
    }
  } else {
    aaBuf = NULL;
  }
  clearModRegion();
  debugMode = gFalse;
}

Splash::Splash(SplashBitmap *bitmapA, GBool vectorAntialiasA,
	       SplashScreen *screenA) {
  int i;

  bitmap = bitmapA;
  vectorAntialias = vectorAntialiasA;
  inShading = gFalse;
  state = new SplashState(bitmap->width, bitmap->height, vectorAntialias,
			  screenA);
  if (vectorAntialias) {
    aaBuf = new SplashBitmap(splashAASize * bitmap->width, splashAASize,
			     1, splashModeMono1, gFalse);
    for (i = 0; i <= splashAASize * splashAASize; ++i) {
      aaGamma[i] = (Guchar)splashRound(
		       splashPow((SplashCoord)i /
				 (SplashCoord)(splashAASize * splashAASize),
				 splashAAGamma) * 255);
    }
  } else {
    aaBuf = NULL;
  }
  clearModRegion();
  debugMode = gFalse;
}

Splash::~Splash() {
  while (state->next) {
    restoreState();
  }
  delete state;
  if (vectorAntialias) {
    delete aaBuf;
  }
}

//------------------------------------------------------------------------
// state read
//------------------------------------------------------------------------

SplashCoord *Splash::getMatrix() {
  return state->matrix;
}

SplashPattern *Splash::getStrokePattern() {
  return state->strokePattern;
}

SplashPattern *Splash::getFillPattern() {
  return state->fillPattern;
}

SplashScreen *Splash::getScreen() {
  return state->screen;
}

SplashBlendFunc Splash::getBlendFunc() {
  return state->blendFunc;
}

SplashCoord Splash::getStrokeAlpha() {
  return state->strokeAlpha;
}

SplashCoord Splash::getFillAlpha() {
  return state->fillAlpha;
}

SplashCoord Splash::getLineWidth() {
  return state->lineWidth;
}

int Splash::getLineCap() {
  return state->lineCap;
}

int Splash::getLineJoin() {
  return state->lineJoin;
}

SplashCoord Splash::getMiterLimit() {
  return state->miterLimit;
}

SplashCoord Splash::getFlatness() {
  return state->flatness;
}

SplashCoord *Splash::getLineDash() {
  return state->lineDash;
}

int Splash::getLineDashLength() {
  return state->lineDashLength;
}

SplashCoord Splash::getLineDashPhase() {
  return state->lineDashPhase;
}

GBool Splash::getStrokeAdjust() {
  return state->strokeAdjust;
}

SplashClip *Splash::getClip() {
  return state->clip;
}

SplashBitmap *Splash::getSoftMask() {
  return state->softMask;
}

GBool Splash::getInNonIsolatedGroup() {
  return state->inNonIsolatedGroup;
}

//------------------------------------------------------------------------
// state write
//------------------------------------------------------------------------

void Splash::setMatrix(SplashCoord *matrix) {
  memcpy(state->matrix, matrix, 6 * sizeof(SplashCoord));
}

void Splash::setStrokePattern(SplashPattern *strokePattern) {
  state->setStrokePattern(strokePattern);
}

void Splash::setFillPattern(SplashPattern *fillPattern) {
  state->setFillPattern(fillPattern);
}

void Splash::setScreen(SplashScreen *screen) {
  state->setScreen(screen);
}

void Splash::setBlendFunc(SplashBlendFunc func) {
  state->blendFunc = func;
}

void Splash::setStrokeAlpha(SplashCoord alpha) {
  state->strokeAlpha = alpha;
}

void Splash::setFillAlpha(SplashCoord alpha) {
  state->fillAlpha = alpha;
}

void Splash::setLineWidth(SplashCoord lineWidth) {
  state->lineWidth = lineWidth;
}

void Splash::setLineCap(int lineCap) {
  state->lineCap = lineCap;
}

void Splash::setLineJoin(int lineJoin) {
  state->lineJoin = lineJoin;
}

void Splash::setMiterLimit(SplashCoord miterLimit) {
  state->miterLimit = miterLimit;
}

void Splash::setFlatness(SplashCoord flatness) {
  if (flatness < 1) {
    state->flatness = 1;
  } else {
    state->flatness = flatness;
  }
}

void Splash::setLineDash(SplashCoord *lineDash, int lineDashLength,
			 SplashCoord lineDashPhase) {
  state->setLineDash(lineDash, lineDashLength, lineDashPhase);
}

void Splash::setStrokeAdjust(GBool strokeAdjust) {
  state->strokeAdjust = strokeAdjust;
}

void Splash::clipResetToRect(SplashCoord x0, SplashCoord y0,
			     SplashCoord x1, SplashCoord y1) {
  state->clip->resetToRect(x0, y0, x1, y1);
}

SplashError Splash::clipToRect(SplashCoord x0, SplashCoord y0,
			       SplashCoord x1, SplashCoord y1) {
  return state->clip->clipToRect(x0, y0, x1, y1);
}

SplashError Splash::clipToPath(SplashPath *path, GBool eo) {
  return state->clip->clipToPath(path, state->matrix, state->flatness, eo);
}

void Splash::setSoftMask(SplashBitmap *softMask) {
  state->setSoftMask(softMask);
}

void Splash::setInNonIsolatedGroup(SplashBitmap *alpha0BitmapA,
				   int alpha0XA, int alpha0YA) {
  alpha0Bitmap = alpha0BitmapA;
  alpha0X = alpha0XA;
  alpha0Y = alpha0YA;
  state->inNonIsolatedGroup = gTrue;
}

void Splash::setOverprintMask(Guint overprintMask) {
  state->overprintMask = overprintMask;
}

//------------------------------------------------------------------------
// state save/restore
//------------------------------------------------------------------------

void Splash::saveState() {
  SplashState *newState;

  newState = state->copy();
  newState->next = state;
  state = newState;
}

SplashError Splash::restoreState() {
  SplashState *oldState;

  if (!state->next) {
    return splashErrNoSave;
  }
  oldState = state;
  state = state->next;
  delete oldState;
  return splashOk;
}

//------------------------------------------------------------------------
// drawing operations
//------------------------------------------------------------------------

void Splash::clear(SplashColorPtr color, Guchar alpha) {
  SplashColorPtr row, p;
  Guchar mono;
  int x, y;

  switch (bitmap->mode) {
  case splashModeMono1:
    mono = (color[0] & 0x80) ? 0xff : 0x00;
    if (bitmap->rowSize < 0) {
      memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	     mono, -bitmap->rowSize * bitmap->height);
    } else {
      memset(bitmap->data, mono, bitmap->rowSize * bitmap->height);
    }
    break;
  case splashModeMono8:
    if (bitmap->rowSize < 0) {
      memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	     color[0], -bitmap->rowSize * bitmap->height);
    } else {
      memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
    }
    break;
  case splashModeRGB8:
    if (color[0] == color[1] && color[1] == color[2]) {
      if (bitmap->rowSize < 0) {
	memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	       color[0], -bitmap->rowSize * bitmap->height);
      } else {
	memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
      }
    } else {
      row = bitmap->data;
      for (y = 0; y < bitmap->height; ++y) {
	p = row;
	for (x = 0; x < bitmap->width; ++x) {
	  *p++ = color[2];
	  *p++ = color[1];
	  *p++ = color[0];
	}
	row += bitmap->rowSize;
      }
    }
    break;
  case splashModeBGR8:
    if (color[0] == color[1] && color[1] == color[2]) {
      if (bitmap->rowSize < 0) {
	memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	       color[0], -bitmap->rowSize * bitmap->height);
      } else {
	memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
      }
    } else {
      row = bitmap->data;
      for (y = 0; y < bitmap->height; ++y) {
	p = row;
	for (x = 0; x < bitmap->width; ++x) {
	  *p++ = color[0];
	  *p++ = color[1];
	  *p++ = color[2];
	}
	row += bitmap->rowSize;
      }
    }
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    if (color[0] == color[1] && color[1] == color[2] && color[2] == color[3]) {
      if (bitmap->rowSize < 0) {
	memset(bitmap->data + bitmap->rowSize * (bitmap->height - 1),
	       color[0], -bitmap->rowSize * bitmap->height);
      } else {
	memset(bitmap->data, color[0], bitmap->rowSize * bitmap->height);
      }
    } else {
      row = bitmap->data;
      for (y = 0; y < bitmap->height; ++y) {
	p = row;
	for (x = 0; x < bitmap->width; ++x) {
	  *p++ = color[0];
	  *p++ = color[1];
	  *p++ = color[2];
	  *p++ = color[3];
	}
	row += bitmap->rowSize;
      }
    }
    break;
#endif
  }

  if (bitmap->alpha) {
    memset(bitmap->alpha, alpha, bitmap->width * bitmap->height);
  }

  updateModX(0);
  updateModY(0);
  updateModX(bitmap->width - 1);
  updateModY(bitmap->height - 1);
}

SplashError Splash::stroke(SplashPath *path) {
  SplashPath *path2, *dPath;

  if (debugMode) {
    printf("stroke [dash:%d] [width:%.2f]:\n",
	   state->lineDashLength, (double)state->lineWidth);
    dumpPath(path);
  }
  opClipRes = splashClipAllOutside;
  if (path->length == 0) {
    return splashErrEmptyPath;
  }
  path2 = flattenPath(path, state->matrix, state->flatness);
  if (state->lineDashLength > 0) {
    dPath = makeDashedPath(path2);
    delete path2;
    path2 = dPath;
  }
  if (state->lineWidth == 0) {
    strokeNarrow(path2);
  } else {
    strokeWide(path2);
  }
  delete path2;
  return splashOk;
}

void Splash::strokeNarrow(SplashPath *path) {
  SplashPipe pipe;
  SplashXPath *xPath;
  SplashXPathSeg *seg;
  int x0, x1, x2, x3, y0, y1, x, y, t;
  SplashCoord dx, dy, dxdy;
  SplashClipResult clipRes;
  int nClipRes[3];
  int i;

  nClipRes[0] = nClipRes[1] = nClipRes[2] = 0;

  xPath = new SplashXPath(path, state->matrix, state->flatness, gFalse);

  pipeInit(&pipe, 0, 0, state->strokePattern, NULL,
	   (Guchar)splashRound(state->strokeAlpha * 255),
	   gFalse, gFalse);

  for (i = 0, seg = xPath->segs; i < xPath->length; ++i, ++seg) {

    x0 = splashFloor(seg->x0);
    x1 = splashFloor(seg->x1);
    y0 = splashFloor(seg->y0);
    y1 = splashFloor(seg->y1);

    // horizontal segment
    if (y0 == y1) {
      if (x0 > x1) {
	t = x0; x0 = x1; x1 = t;
      }
      if ((clipRes = state->clip->testSpan(x0, x1, y0))
	  != splashClipAllOutside) {
	drawSpan(&pipe, x0, x1, y0, clipRes == splashClipAllInside);
      }

    // segment with |dx| > |dy|
    } else if (splashAbs(seg->dxdy) > 1) {
      dx = seg->x1 - seg->x0;
      dy = seg->y1 - seg->y0;
      dxdy = seg->dxdy;
      if (y0 > y1) {
	t = y0; y0 = y1; y1 = t;
	t = x0; x0 = x1; x1 = t;
	dx = -dx;
	dy = -dy;
      }
      if ((clipRes = state->clip->testRect(x0 <= x1 ? x0 : x1, y0,
					   x0 <= x1 ? x1 : x0, y1))
	  != splashClipAllOutside) {
	if (dx > 0) {
	  x2 = x0;
	  x3 = splashFloor(seg->x0 + ((SplashCoord)y0 + 1 - seg->y0) * dxdy);
	  drawSpan(&pipe, x2, (x2 <= x3 - 1) ? x3 - 1 : x2, y0,
		   clipRes == splashClipAllInside);
	  x2 = x3;
	  for (y = y0 + 1; y <= y1 - 1; ++y) {
	    x3 = splashFloor(seg->x0 + ((SplashCoord)y + 1 - seg->y0) * dxdy);
	    drawSpan(&pipe, x2, x3 - 1, y, clipRes == splashClipAllInside);
	    x2 = x3;
	  }
	  drawSpan(&pipe, x2, x2 <= x1 ? x1 : x2, y1,
		   clipRes == splashClipAllInside);
	} else {
	  x2 = x0;
	  x3 = splashFloor(seg->x0 + ((SplashCoord)y0 + 1 - seg->y0) * dxdy);
	  drawSpan(&pipe, (x3 + 1 <= x2) ? x3 + 1 : x2, x2, y0,
		   clipRes == splashClipAllInside);
	  x2 = x3;
	  for (y = y0 + 1; y <= y1 - 1; ++y) {
	    x3 = splashFloor(seg->x0 + ((SplashCoord)y + 1 - seg->y0) * dxdy);
	    drawSpan(&pipe, x3 + 1, x2, y, clipRes == splashClipAllInside);
	    x2 = x3;
	  }
	  drawSpan(&pipe, x1, (x1 <= x2) ? x2 : x1, y1,
		   clipRes == splashClipAllInside);
	}
      }

    // segment with |dy| > |dx|
    } else {
      dxdy = seg->dxdy;
      if (y0 > y1) {
	t = x0; x0 = x1; x1 = t;
	t = y0; y0 = y1; y1 = t;
      }
      if ((clipRes = state->clip->testRect(x0 <= x1 ? x0 : x1, y0,
					   x0 <= x1 ? x1 : x0, y1))
	  != splashClipAllOutside) {
	drawPixel(&pipe, x0, y0, clipRes == splashClipAllInside);
	for (y = y0 + 1; y <= y1 - 1; ++y) {
	  x = splashFloor(seg->x0 + ((SplashCoord)y - seg->y0) * dxdy);
	  drawPixel(&pipe, x, y, clipRes == splashClipAllInside);
	}
	drawPixel(&pipe, x1, y1, clipRes == splashClipAllInside);
      }
    }
    ++nClipRes[clipRes];
  }
  if (nClipRes[splashClipPartial] ||
      (nClipRes[splashClipAllInside] && nClipRes[splashClipAllOutside])) {
    opClipRes = splashClipPartial;
  } else if (nClipRes[splashClipAllInside]) {
    opClipRes = splashClipAllInside;
  } else {
    opClipRes = splashClipAllOutside;
  }

  delete xPath;
}

void Splash::strokeWide(SplashPath *path) {
  SplashPath *path2;

  path2 = makeStrokePath(path, gFalse);
  fillWithPattern(path2, gFalse, state->strokePattern, state->strokeAlpha);
  delete path2;
}

SplashPath *Splash::flattenPath(SplashPath *path, SplashCoord *matrix,
				SplashCoord flatness) {
  SplashPath *fPath;
  SplashCoord flatness2;
  Guchar flag;
  int i;

  fPath = new SplashPath();
  flatness2 = flatness * flatness;
  i = 0;
  while (i < path->length) {
    flag = path->flags[i];
    if (flag & splashPathFirst) {
      fPath->moveTo(path->pts[i].x, path->pts[i].y);
      ++i;
    } else {
      if (flag & splashPathCurve) {
	flattenCurve(path->pts[i-1].x, path->pts[i-1].y,
		     path->pts[i  ].x, path->pts[i  ].y,
		     path->pts[i+1].x, path->pts[i+1].y,
		     path->pts[i+2].x, path->pts[i+2].y,
		     matrix, flatness2, fPath);
	i += 3;
      } else {
	fPath->lineTo(path->pts[i].x, path->pts[i].y);
	++i;
      }
      if (path->flags[i-1] & splashPathClosed) {
	fPath->close();
      }
    }
  }
  return fPath;
}

void Splash::flattenCurve(SplashCoord x0, SplashCoord y0,
			  SplashCoord x1, SplashCoord y1,
			  SplashCoord x2, SplashCoord y2,
			  SplashCoord x3, SplashCoord y3,
			  SplashCoord *matrix, SplashCoord flatness2,
			  SplashPath *fPath) {
  SplashCoord cx[splashMaxCurveSplits + 1][3];
  SplashCoord cy[splashMaxCurveSplits + 1][3];
  int cNext[splashMaxCurveSplits + 1];
  SplashCoord xl0, xl1, xl2, xr0, xr1, xr2, xr3, xx1, xx2, xh;
  SplashCoord yl0, yl1, yl2, yr0, yr1, yr2, yr3, yy1, yy2, yh;
  SplashCoord dx, dy, mx, my, tx, ty, d1, d2;
  int p1, p2, p3;

  // initial segment
  p1 = 0;
  p2 = splashMaxCurveSplits;
  cx[p1][0] = x0;  cy[p1][0] = y0;
  cx[p1][1] = x1;  cy[p1][1] = y1;
  cx[p1][2] = x2;  cy[p1][2] = y2;
  cx[p2][0] = x3;  cy[p2][0] = y3;
  cNext[p1] = p2;

  while (p1 < splashMaxCurveSplits) {

    // get the next segment
    xl0 = cx[p1][0];  yl0 = cy[p1][0];
    xx1 = cx[p1][1];  yy1 = cy[p1][1];
    xx2 = cx[p1][2];  yy2 = cy[p1][2];
    p2 = cNext[p1];
    xr3 = cx[p2][0];  yr3 = cy[p2][0];

    // compute the distances (in device space) from the control points
    // to the midpoint of the straight line (this is a bit of a hack,
    // but it's much faster than computing the actual distances to the
    // line)
    transform(matrix, (xl0 + xr3) * 0.5, (yl0 + yr3) * 0.5, &mx, &my);
    transform(matrix, xx1, yy1, &tx, &ty);
    dx = tx - mx;
    dy = ty - my;
    d1 = dx*dx + dy*dy;
    transform(matrix, xx2, yy2, &tx, &ty);
    dx = tx - mx;
    dy = ty - my;
    d2 = dx*dx + dy*dy;

    // if the curve is flat enough, or no more subdivisions are
    // allowed, add the straight line segment
    if (p2 - p1 == 1 || (d1 <= flatness2 && d2 <= flatness2)) {
      fPath->lineTo(xr3, yr3);
      p1 = p2;

    // otherwise, subdivide the curve
    } else {
      xl1 = (xl0 + xx1) * 0.5;
      yl1 = (yl0 + yy1) * 0.5;
      xh = (xx1 + xx2) * 0.5;
      yh = (yy1 + yy2) * 0.5;
      xl2 = (xl1 + xh) * 0.5;
      yl2 = (yl1 + yh) * 0.5;
      xr2 = (xx2 + xr3) * 0.5;
      yr2 = (yy2 + yr3) * 0.5;
      xr1 = (xh + xr2) * 0.5;
      yr1 = (yh + yr2) * 0.5;
      xr0 = (xl2 + xr1) * 0.5;
      yr0 = (yl2 + yr1) * 0.5;
      // add the new subdivision points
      p3 = (p1 + p2) / 2;
      cx[p1][1] = xl1;  cy[p1][1] = yl1;
      cx[p1][2] = xl2;  cy[p1][2] = yl2;
      cNext[p1] = p3;
      cx[p3][0] = xr0;  cy[p3][0] = yr0;
      cx[p3][1] = xr1;  cy[p3][1] = yr1;
      cx[p3][2] = xr2;  cy[p3][2] = yr2;
      cNext[p3] = p2;
    }
  }
}

SplashPath *Splash::makeDashedPath(SplashPath *path) {
  SplashPath *dPath;
  SplashCoord lineDashTotal;
  SplashCoord lineDashStartPhase, lineDashDist, segLen;
  SplashCoord x0, y0, x1, y1, xa, ya;
  GBool lineDashStartOn, lineDashOn, newPath;
  int lineDashStartIdx, lineDashIdx;
  int i, j, k;

  lineDashTotal = 0;
  for (i = 0; i < state->lineDashLength; ++i) {
    lineDashTotal += state->lineDash[i];
  }
  lineDashStartPhase = state->lineDashPhase;
  i = splashFloor(lineDashStartPhase / lineDashTotal);
  lineDashStartPhase -= (SplashCoord)i * lineDashTotal;
  lineDashStartOn = gTrue;
  lineDashStartIdx = 0;
  if (lineDashStartPhase > 0) {
    while (lineDashStartPhase >= state->lineDash[lineDashStartIdx]) {
      lineDashStartOn = !lineDashStartOn;
      lineDashStartPhase -= state->lineDash[lineDashStartIdx];
      ++lineDashStartIdx;
    }
  }

  dPath = new SplashPath();

  // process each subpath
  i = 0;
  while (i < path->length) {

    // find the end of the subpath
    for (j = i;
	 j < path->length - 1 && !(path->flags[j] & splashPathLast);
	 ++j) ;

    // initialize the dash parameters
    lineDashOn = lineDashStartOn;
    lineDashIdx = lineDashStartIdx;
    lineDashDist = state->lineDash[lineDashIdx] - lineDashStartPhase;

    // process each segment of the subpath
    newPath = gTrue;
    for (k = i; k < j; ++k) {

      // grab the segment
      x0 = path->pts[k].x;
      y0 = path->pts[k].y;
      x1 = path->pts[k+1].x;
      y1 = path->pts[k+1].y;
      segLen = splashDist(x0, y0, x1, y1);

      // process the segment
      while (segLen > 0) {

	if (lineDashDist >= segLen) {
	  if (lineDashOn) {
	    if (newPath) {
	      dPath->moveTo(x0, y0);
	      newPath = gFalse;
	    }
	    dPath->lineTo(x1, y1);
	  }
	  lineDashDist -= segLen;
	  segLen = 0;

	} else {
	  xa = x0 + (lineDashDist / segLen) * (x1 - x0);
	  ya = y0 + (lineDashDist / segLen) * (y1 - y0);
	  if (lineDashOn) {
	    if (newPath) {
	      dPath->moveTo(x0, y0);
	      newPath = gFalse;
	    }
	    dPath->lineTo(xa, ya);
	  }
	  x0 = xa;
	  y0 = ya;
	  segLen -= lineDashDist;
	  lineDashDist = 0;
	}

	// get the next entry in the dash array
	if (lineDashDist <= 0) {
	  lineDashOn = !lineDashOn;
	  if (++lineDashIdx == state->lineDashLength) {
	    lineDashIdx = 0;
	  }
	  lineDashDist = state->lineDash[lineDashIdx];
	  newPath = gTrue;
	}
      }
    }
    i = j + 1;
  }

  return dPath;
}

SplashError Splash::fill(SplashPath *path, GBool eo) {
  if (debugMode) {
    printf("fill [eo:%d]:\n", eo);
    dumpPath(path);
  }
  return fillWithPattern(path, eo, state->fillPattern, state->fillAlpha);
}

SplashError Splash::fillWithPattern(SplashPath *path, GBool eo,
				    SplashPattern *pattern,
				    SplashCoord alpha) {
  SplashPipe pipe;
  SplashXPath *xPath;
  SplashXPathScanner *scanner;
  int xMinI, yMinI, xMaxI, yMaxI, x0, x1, y;
  SplashClipResult clipRes, clipRes2;

  if (path->length == 0) {
    return splashErrEmptyPath;
  }
  if (pathAllOutside(path)) {
    opClipRes = splashClipAllOutside;
    return splashOk;
  }
  xPath = new SplashXPath(path, state->matrix, state->flatness, gTrue);
  if (vectorAntialias && !inShading) {
    xPath->aaScale();
  }
  xPath->sort();
  yMinI = state->clip->getYMinI();
  yMaxI = state->clip->getYMaxI();
  if (vectorAntialias && !inShading) {
    yMinI = yMinI * splashAASize;
    yMaxI = (yMaxI + 1) * splashAASize - 1;
  }
  scanner = new SplashXPathScanner(xPath, eo, yMinI, yMaxI);

  // get the min and max x and y values
  if (vectorAntialias && !inShading) {
    scanner->getBBoxAA(&xMinI, &yMinI, &xMaxI, &yMaxI);
  } else {
    scanner->getBBox(&xMinI, &yMinI, &xMaxI, &yMaxI);
  }

  // check clipping
  if ((clipRes = state->clip->testRect(xMinI, yMinI, xMaxI, yMaxI))
      != splashClipAllOutside) {
    if (scanner->hasPartialClip()) {
      clipRes = splashClipPartial;
    }

    pipeInit(&pipe, 0, yMinI, pattern, NULL, (Guchar)splashRound(alpha * 255),
	     vectorAntialias && !inShading, gFalse);

    // draw the spans
    if (vectorAntialias && !inShading) {
      for (y = yMinI; y <= yMaxI; ++y) {
	scanner->renderAALine(aaBuf, &x0, &x1, y);
	if (clipRes != splashClipAllInside) {
	  state->clip->clipAALine(aaBuf, &x0, &x1, y);
	}
	drawAALine(&pipe, x0, x1, y);
      }
    } else {
      for (y = yMinI; y <= yMaxI; ++y) {
	while (scanner->getNextSpan(y, &x0, &x1)) {
	  if (clipRes == splashClipAllInside) {
	    drawSpan(&pipe, x0, x1, y, gTrue);
	  } else {
	    // limit the x range
	    if (x0 < state->clip->getXMinI()) {
	      x0 = state->clip->getXMinI();
	    }
	    if (x1 > state->clip->getXMaxI()) {
	      x1 = state->clip->getXMaxI();
	    }
	    clipRes2 = state->clip->testSpan(x0, x1, y);
	    drawSpan(&pipe, x0, x1, y, clipRes2 == splashClipAllInside);
	  }
	}
      }
    }
  }
  opClipRes = clipRes;

  delete scanner;
  delete xPath;
  return splashOk;
}

GBool Splash::pathAllOutside(SplashPath *path) {
  SplashCoord xMin1, yMin1, xMax1, yMax1;
  SplashCoord xMin2, yMin2, xMax2, yMax2;
  SplashCoord x, y;
  int xMinI, yMinI, xMaxI, yMaxI;
  int i;

  xMin1 = xMax1 = path->pts[0].x;
  yMin1 = yMax1 = path->pts[0].y;
  for (i = 1; i < path->length; ++i) {
    if (path->pts[i].x < xMin1) {
      xMin1 = path->pts[i].x;
    } else if (path->pts[i].x > xMax1) {
      xMax1 = path->pts[i].x;
    }
    if (path->pts[i].y < yMin1) {
      yMin1 = path->pts[i].y;
    } else if (path->pts[i].y > yMax1) {
      yMax1 = path->pts[i].y;
    }
  }

  transform(state->matrix, xMin1, yMin1, &x, &y);
  xMin2 = xMax2 = x;
  yMin2 = yMax2 = y;
  transform(state->matrix, xMin1, yMax1, &x, &y);
  if (x < xMin2) {
    xMin2 = x;
  } else if (x > xMax2) {
    xMax2 = x;
  }
  if (y < yMin2) {
    yMin2 = y;
  } else if (y > yMax2) {
    yMax2 = y;
  }
  transform(state->matrix, xMax1, yMin1, &x, &y);
  if (x < xMin2) {
    xMin2 = x;
  } else if (x > xMax2) {
    xMax2 = x;
  }
  if (y < yMin2) {
    yMin2 = y;
  } else if (y > yMax2) {
    yMax2 = y;
  }
  transform(state->matrix, xMax1, yMax1, &x, &y);
  if (x < xMin2) {
    xMin2 = x;
  } else if (x > xMax2) {
    xMax2 = x;
  }
  if (y < yMin2) {
    yMin2 = y;
  } else if (y > yMax2) {
    yMax2 = y;
  }
  xMinI = splashFloor(xMin2);
  yMinI = splashFloor(yMin2);
  xMaxI = splashFloor(xMax2);
  yMaxI = splashFloor(yMax2);

  return state->clip->testRect(xMinI, yMinI, xMaxI, yMaxI) ==
         splashClipAllOutside;
}

SplashError Splash::xorFill(SplashPath *path, GBool eo) {
  SplashPipe pipe;
  SplashXPath *xPath;
  SplashXPathScanner *scanner;
  int xMinI, yMinI, xMaxI, yMaxI, x0, x1, y;
  SplashClipResult clipRes, clipRes2;
  SplashBlendFunc origBlendFunc;

  if (path->length == 0) {
    return splashErrEmptyPath;
  }
  xPath = new SplashXPath(path, state->matrix, state->flatness, gTrue);
  xPath->sort();
  scanner = new SplashXPathScanner(xPath, eo, state->clip->getYMinI(),
				   state->clip->getYMaxI());

  // get the min and max x and y values
  scanner->getBBox(&xMinI, &yMinI, &xMaxI, &yMaxI);

  // check clipping
  if ((clipRes = state->clip->testRect(xMinI, yMinI, xMaxI, yMaxI))
      != splashClipAllOutside) {
    if (scanner->hasPartialClip()) {
      clipRes = splashClipPartial;
    }

    origBlendFunc = state->blendFunc;
    state->blendFunc = &blendXor;
    pipeInit(&pipe, 0, yMinI, state->fillPattern, NULL, 255, gFalse, gFalse);

    // draw the spans
    for (y = yMinI; y <= yMaxI; ++y) {
      while (scanner->getNextSpan(y, &x0, &x1)) {
	if (clipRes == splashClipAllInside) {
	  drawSpan(&pipe, x0, x1, y, gTrue);
	} else {
	  // limit the x range
	  if (x0 < state->clip->getXMinI()) {
	    x0 = state->clip->getXMinI();
	  }
	  if (x1 > state->clip->getXMaxI()) {
	    x1 = state->clip->getXMaxI();
	  }
	  clipRes2 = state->clip->testSpan(x0, x1, y);
	  drawSpan(&pipe, x0, x1, y, clipRes2 == splashClipAllInside);
	}
      }
    }
    state->blendFunc = origBlendFunc;
  }
  opClipRes = clipRes;

  delete scanner;
  delete xPath;
  return splashOk;
}

SplashError Splash::fillChar(SplashCoord x, SplashCoord y,
			     int c, SplashFont *font) {
  SplashGlyphBitmap glyph;
  SplashCoord xt, yt;
  int x0, y0, xFrac, yFrac;
  SplashError err;

  if (debugMode) {
    printf("fillChar: x=%.2f y=%.2f c=%3d=0x%02x='%c'\n",
	   (double)x, (double)y, c, c, c);
  }
  transform(state->matrix, x, y, &xt, &yt);
  x0 = splashFloor(xt);
  xFrac = splashFloor((xt - x0) * splashFontFraction);
  y0 = splashFloor(yt);
  yFrac = splashFloor((yt - y0) * splashFontFraction);
  if (!font->getGlyph(c, xFrac, yFrac, &glyph)) {
    return splashErrNoGlyph;
  }
  err = fillGlyph2(x0, y0, &glyph);
  if (glyph.freeData) {
    gfree(glyph.data);
  }
  return err;
}

SplashError Splash::fillGlyph(SplashCoord x, SplashCoord y,
			      SplashGlyphBitmap *glyph) {
  SplashCoord xt, yt;
  int x0, y0;

  transform(state->matrix, x, y, &xt, &yt);
  x0 = splashFloor(xt);
  y0 = splashFloor(yt);
  return fillGlyph2(x0, y0, glyph);
}

SplashError Splash::fillGlyph2(int x0, int y0, SplashGlyphBitmap *glyph) {
  SplashPipe pipe;
  SplashClipResult clipRes;
  GBool noClip;
  int alpha0;
  Guchar alpha;
  Guchar *p;
  int x1, y1, xx, xx1, yy;

  if ((clipRes = state->clip->testRect(x0 - glyph->x,
				       y0 - glyph->y,
				       x0 - glyph->x + glyph->w - 1,
				       y0 - glyph->y + glyph->h - 1))
      != splashClipAllOutside) {
    noClip = clipRes == splashClipAllInside;

    if (noClip) {
      if (glyph->aa) {
	pipeInit(&pipe, x0 - glyph->x, y0 - glyph->y, state->fillPattern, NULL,
		 (Guchar)splashRound(state->fillAlpha * 255), gTrue, gFalse);
	p = glyph->data;
	for (yy = 0, y1 = y0 - glyph->y; yy < glyph->h; ++yy, ++y1) {
	  pipeSetXY(&pipe, x0 - glyph->x, y1);
	  for (xx = 0, x1 = x0 - glyph->x; xx < glyph->w; ++xx, ++x1) {
	    alpha = *p++;
	    if (alpha != 0) {
	      pipe.shape = alpha;
	      (this->*pipe.run)(&pipe);
	      updateModX(x1);
	      updateModY(y1);
	    } else {
	      pipeIncX(&pipe);
	    }
	  }
	}
      } else {
	pipeInit(&pipe, x0 - glyph->x, y0 - glyph->y, state->fillPattern, NULL,
		 (Guchar)splashRound(state->fillAlpha * 255), gFalse, gFalse);
	p = glyph->data;
	for (yy = 0, y1 = y0 - glyph->y; yy < glyph->h; ++yy, ++y1) {
	  pipeSetXY(&pipe, x0 - glyph->x, y1);
	  for (xx = 0, x1 = x0 - glyph->x; xx < glyph->w; xx += 8) {
	    alpha0 = *p++;
	    for (xx1 = 0; xx1 < 8 && xx + xx1 < glyph->w; ++xx1, ++x1) {
	      if (alpha0 & 0x80) {
		(this->*pipe.run)(&pipe);
		updateModX(x1);
		updateModY(y1);
	      } else {
		pipeIncX(&pipe);
	      }
	      alpha0 <<= 1;
	    }
	  }
	}
      }
    } else {
      if (glyph->aa) {
	pipeInit(&pipe, x0 - glyph->x, y0 - glyph->y, state->fillPattern, NULL,
		 (Guchar)splashRound(state->fillAlpha * 255), gTrue, gFalse);
	p = glyph->data;
	for (yy = 0, y1 = y0 - glyph->y; yy < glyph->h; ++yy, ++y1) {
	  pipeSetXY(&pipe, x0 - glyph->x, y1);
	  for (xx = 0, x1 = x0 - glyph->x; xx < glyph->w; ++xx, ++x1) {
	    if (state->clip->test(x1, y1)) {
	      alpha = *p++;
	      if (alpha != 0) {
		pipe.shape = alpha;
		(this->*pipe.run)(&pipe);
		updateModX(x1);
		updateModY(y1);
	      } else {
		pipeIncX(&pipe);
	      }
	    } else {
	      pipeIncX(&pipe);
	      ++p;
	    }
	  }
	}
      } else {
	pipeInit(&pipe, x0 - glyph->x, y0 - glyph->y, state->fillPattern, NULL,
		 (Guchar)splashRound(state->fillAlpha * 255), gFalse, gFalse);
	p = glyph->data;
	for (yy = 0, y1 = y0 - glyph->y; yy < glyph->h; ++yy, ++y1) {
	  pipeSetXY(&pipe, x0 - glyph->x, y1);
	  for (xx = 0, x1 = x0 - glyph->x; xx < glyph->w; xx += 8) {
	    alpha0 = *p++;
	    for (xx1 = 0; xx1 < 8 && xx + xx1 < glyph->w; ++xx1, ++x1) {
	      if (state->clip->test(x1, y1)) {
		if (alpha0 & 0x80) {
		  (this->*pipe.run)(&pipe);
		  updateModX(x1);
		  updateModY(y1);
		} else {
		  pipeIncX(&pipe);
		}
	      } else {
		pipeIncX(&pipe);
	      }
	      alpha0 <<= 1;
	    }
	  }
	}
      }
    }
  }
  opClipRes = clipRes;

  return splashOk;
}

SplashError Splash::fillImageMask(SplashImageMaskSource src, void *srcData,
				  int w, int h, SplashCoord *mat,
				  GBool glyphMode) {
  SplashBitmap *scaledMask;
  SplashClipResult clipRes;
  GBool minorAxisZero;
  int x0, y0, x1, y1, scaledWidth, scaledHeight;

  if (debugMode) {
    printf("fillImageMask: w=%d h=%d mat=[%.2f %.2f %.2f %.2f %.2f %.2f]\n",
	   w, h, (double)mat[0], (double)mat[1], (double)mat[2],
	   (double)mat[3], (double)mat[4], (double)mat[5]);
  }

  // check for singular matrix
  if (splashAbs(mat[0] * mat[3] - mat[1] * mat[2]) < 0.000001) {
    return splashErrSingularMatrix;
  }

  minorAxisZero = mat[1] == 0 && mat[2] == 0;

  // scaling only
  if (mat[0] > 0 && minorAxisZero && mat[3] > 0) {
    x0 = imgCoordMungeLowerC(mat[4], glyphMode);
    y0 = imgCoordMungeLowerC(mat[5], glyphMode);
    x1 = imgCoordMungeUpperC(mat[0] + mat[4], glyphMode);
    y1 = imgCoordMungeUpperC(mat[3] + mat[5], glyphMode);
    // make sure narrow images cover at least one pixel
    if (x0 == x1) {
      ++x1;
    }
    if (y0 == y1) {
      ++y1;
    }
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledMask = scaleMask(src, srcData, w, h, scaledWidth, scaledHeight);
      blitMask(scaledMask, x0, y0, clipRes);
      delete scaledMask;
    }
    
  // scaling plus vertical flip
  } else if (mat[0] > 0 && minorAxisZero && mat[3] < 0) {
    x0 = imgCoordMungeLowerC(mat[4], glyphMode);
    y0 = imgCoordMungeLowerC(mat[3] + mat[5], glyphMode);
    x1 = imgCoordMungeUpperC(mat[0] + mat[4], glyphMode);
    y1 = imgCoordMungeUpperC(mat[5], glyphMode);
    // make sure narrow images cover at least one pixel
    if (x0 == x1) {
      ++x1;
    }
    if (y0 == y1) {
      ++y1;
    }
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledMask = scaleMask(src, srcData, w, h, scaledWidth, scaledHeight);
      vertFlipImage(scaledMask, scaledWidth, scaledHeight, 1);
      blitMask(scaledMask, x0, y0, clipRes);
      delete scaledMask;
    }

  // all other cases
  } else {
    arbitraryTransformMask(src, srcData, w, h, mat, glyphMode);
  }

  return splashOk;
}

void Splash::arbitraryTransformMask(SplashImageMaskSource src, void *srcData,
				    int srcWidth, int srcHeight,
				    SplashCoord *mat, GBool glyphMode) {
  SplashBitmap *scaledMask;
  SplashClipResult clipRes, clipRes2;
  SplashPipe pipe;
  int scaledWidth, scaledHeight, t0, t1;
  SplashCoord r00, r01, r10, r11, det, ir00, ir01, ir10, ir11;
  SplashCoord vx[4], vy[4];
  int xMin, yMin, xMax, yMax;
  ImageSection section[3];
  int nSections;
  int y, xa, xb, x, i, xx, yy;

  // compute the four vertices of the target quadrilateral
  vx[0] = mat[4];                    vy[0] = mat[5];
  vx[1] = mat[2] + mat[4];           vy[1] = mat[3] + mat[5];
  vx[2] = mat[0] + mat[2] + mat[4];  vy[2] = mat[1] + mat[3] + mat[5];
  vx[3] = mat[0] + mat[4];           vy[3] = mat[1] + mat[5];

  // clipping
  xMin = imgCoordMungeLowerC(vx[0], glyphMode);
  xMax = imgCoordMungeUpperC(vx[0], glyphMode);
  yMin = imgCoordMungeLowerC(vy[0], glyphMode);
  yMax = imgCoordMungeUpperC(vy[0], glyphMode);
  for (i = 1; i < 4; ++i) {
    t0 = imgCoordMungeLowerC(vx[i], glyphMode);
    if (t0 < xMin) {
      xMin = t0;
    }
    t0 = imgCoordMungeUpperC(vx[i], glyphMode);
    if (t0 > xMax) {
      xMax = t0;
    }
    t1 = imgCoordMungeLowerC(vy[i], glyphMode);
    if (t1 < yMin) {
      yMin = t1;
    }
    t1 = imgCoordMungeUpperC(vy[i], glyphMode);
    if (t1 > yMax) {
      yMax = t1;
    }
  }
  clipRes = state->clip->testRect(xMin, yMin, xMax - 1, yMax - 1);
  opClipRes = clipRes;
  if (clipRes == splashClipAllOutside) {
    return;
  }

  // compute the scale factors
  if (mat[0] >= 0) {
    t0 = imgCoordMungeUpperC(mat[0] + mat[4], glyphMode) -
         imgCoordMungeLowerC(mat[4], glyphMode);
  } else {
    t0 = imgCoordMungeUpperC(mat[4], glyphMode) -
         imgCoordMungeLowerC(mat[0] + mat[4], glyphMode);
  }
  if (mat[1] >= 0) {
    t1 = imgCoordMungeUpperC(mat[1] + mat[5], glyphMode) -
         imgCoordMungeLowerC(mat[5], glyphMode);
  } else {
    t1 = imgCoordMungeUpperC(mat[5], glyphMode) -
         imgCoordMungeLowerC(mat[1] + mat[5], glyphMode);
  }
  scaledWidth = t0 > t1 ? t0 : t1;
  if (mat[2] >= 0) {
    t0 = imgCoordMungeUpperC(mat[2] + mat[4], glyphMode) -
         imgCoordMungeLowerC(mat[4], glyphMode);
  } else {
    t0 = imgCoordMungeUpperC(mat[4], glyphMode) -
         imgCoordMungeLowerC(mat[2] + mat[4], glyphMode);
  }
  if (mat[3] >= 0) {
    t1 = imgCoordMungeUpperC(mat[3] + mat[5], glyphMode) -
         imgCoordMungeLowerC(mat[5], glyphMode);
  } else {
    t1 = imgCoordMungeUpperC(mat[5], glyphMode) -
         imgCoordMungeLowerC(mat[3] + mat[5], glyphMode);
  }
  scaledHeight = t0 > t1 ? t0 : t1;
  if (scaledWidth == 0) {
    scaledWidth = 1;
  }
  if (scaledHeight == 0) {
    scaledHeight = 1;
  }

  // compute the inverse transform (after scaling) matrix
  r00 = mat[0] / scaledWidth;
  r01 = mat[1] / scaledWidth;
  r10 = mat[2] / scaledHeight;
  r11 = mat[3] / scaledHeight;
  det = r00 * r11 - r01 * r10;
  if (splashAbs(det) < 1e-6) {
    // this should be caught by the singular matrix check in fillImageMask
    return;
  }
  ir00 = r11 / det;
  ir01 = -r01 / det;
  ir10 = -r10 / det;
  ir11 = r00 / det;

  // scale the input image
  scaledMask = scaleMask(src, srcData, srcWidth, srcHeight,
			 scaledWidth, scaledHeight);

  // construct the three sections
  i = (vy[2] <= vy[3]) ? 2 : 3;
  if (vy[1] <= vy[i]) {
    i = 1;
  }
  if (vy[0] < vy[i] || (i != 3 && vy[0] == vy[i])) {
    i = 0;
  }
  if (vy[i] == vy[(i+1) & 3]) {
    section[0].y0 = imgCoordMungeLowerC(vy[i], glyphMode);
    section[0].y1 = imgCoordMungeUpperC(vy[(i+2) & 3], glyphMode) - 1;
    if (vx[i] < vx[(i+1) & 3]) {
      section[0].ia0 = i;
      section[0].ia1 = (i+3) & 3;
      section[0].ib0 = (i+1) & 3;
      section[0].ib1 = (i+2) & 3;
    } else {
      section[0].ia0 = (i+1) & 3;
      section[0].ia1 = (i+2) & 3;
      section[0].ib0 = i;
      section[0].ib1 = (i+3) & 3;
    }
    nSections = 1;
  } else {
    section[0].y0 = imgCoordMungeLowerC(vy[i], glyphMode);
    section[2].y1 = imgCoordMungeUpperC(vy[(i+2) & 3], glyphMode) - 1;
    section[0].ia0 = section[0].ib0 = i;
    section[2].ia1 = section[2].ib1 = (i+2) & 3;
    if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
      section[0].ia1 = section[2].ia0 = (i+1) & 3;
      section[0].ib1 = section[2].ib0 = (i+3) & 3;
    } else {
      section[0].ia1 = section[2].ia0 = (i+3) & 3;
      section[0].ib1 = section[2].ib0 = (i+1) & 3;
    }
    if (vy[(i+1) & 3] < vy[(i+3) & 3]) {
      section[1].y0 = imgCoordMungeLowerC(vy[(i+1) & 3], glyphMode);
      section[2].y0 = imgCoordMungeUpperC(vy[(i+3) & 3], glyphMode);
      if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
	section[1].ia0 = (i+1) & 3;
	section[1].ia1 = (i+2) & 3;
	section[1].ib0 = i;
	section[1].ib1 = (i+3) & 3;
      } else {
	section[1].ia0 = i;
	section[1].ia1 = (i+3) & 3;
	section[1].ib0 = (i+1) & 3;
	section[1].ib1 = (i+2) & 3;
      }
    } else {
      section[1].y0 = imgCoordMungeLowerC(vy[(i+3) & 3], glyphMode);
      section[2].y0 = imgCoordMungeUpperC(vy[(i+1) & 3], glyphMode);
      if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
	section[1].ia0 = i;
	section[1].ia1 = (i+1) & 3;
	section[1].ib0 = (i+3) & 3;
	section[1].ib1 = (i+2) & 3;
      } else {
	section[1].ia0 = (i+3) & 3;
	section[1].ia1 = (i+2) & 3;
	section[1].ib0 = i;
	section[1].ib1 = (i+1) & 3;
      }
    }
    section[0].y1 = section[1].y0 - 1;
    section[1].y1 = section[2].y0 - 1;
    nSections = 3;
  }
  for (i = 0; i < nSections; ++i) {
    section[i].xa0 = vx[section[i].ia0];
    section[i].ya0 = vy[section[i].ia0];
    section[i].xa1 = vx[section[i].ia1];
    section[i].ya1 = vy[section[i].ia1];
    section[i].xb0 = vx[section[i].ib0];
    section[i].yb0 = vy[section[i].ib0];
    section[i].xb1 = vx[section[i].ib1];
    section[i].yb1 = vy[section[i].ib1];
    section[i].dxdya = (section[i].xa1 - section[i].xa0) /
                       (section[i].ya1 - section[i].ya0);
    section[i].dxdyb = (section[i].xb1 - section[i].xb0) /
                       (section[i].yb1 - section[i].yb0);
  }

  // initialize the pixel pipe
  pipeInit(&pipe, 0, 0, state->fillPattern, NULL,
	   (Guchar)splashRound(state->fillAlpha * 255), gTrue, gFalse);
  if (vectorAntialias) {
    drawAAPixelInit();
  }

  // make sure narrow images cover at least one pixel
  if (nSections == 1) {
    if (section[0].y0 == section[0].y1) {
      ++section[0].y1;
      clipRes = opClipRes = splashClipPartial;
    }
  } else {
    if (section[0].y0 == section[2].y1) {
      ++section[1].y1;
      clipRes = opClipRes = splashClipPartial;
    }
  }

  // scan all pixels inside the target region
  for (i = 0; i < nSections; ++i) {
    for (y = section[i].y0; y <= section[i].y1; ++y) {
      xa = imgCoordMungeLowerC(section[i].xa0 +
			         (y + 0.5 - section[i].ya0) * section[i].dxdya,
			       glyphMode);
      xb = imgCoordMungeUpperC(section[i].xb0 +
			         (y + 0.5 - section[i].yb0) * section[i].dxdyb,
			       glyphMode);
      // make sure narrow images cover at least one pixel
      if (xa == xb) {
	++xb;
      }
      if (clipRes != splashClipAllInside) {
	clipRes2 = state->clip->testSpan(xa, xb - 1, y);
      } else {
	clipRes2 = clipRes;
      }
      for (x = xa; x < xb; ++x) {
	// map (x+0.5, y+0.5) back to the scaled image
	xx = splashFloor((x + 0.5 - mat[4]) * ir00 +
			 (y + 0.5 - mat[5]) * ir10);
	yy = splashFloor((x + 0.5 - mat[4]) * ir01 +
			 (y + 0.5 - mat[5]) * ir11);
	// xx should always be within bounds, but floating point
	// inaccuracy can cause problems
	if (xx < 0) {
	  xx = 0;
	} else if (xx >= scaledWidth) {
	  xx = scaledWidth - 1;
	}
	if (yy < 0) {
	  yy = 0;
	} else if (yy >= scaledHeight) {
	  yy = scaledHeight - 1;
	}
	pipe.shape = scaledMask->data[yy * scaledWidth + xx];
	if (vectorAntialias && clipRes2 != splashClipAllInside) {
	  drawAAPixel(&pipe, x, y);
	} else {
	  drawPixel(&pipe, x, y, clipRes2 == splashClipAllInside);
	}
      }
    }
  }

  delete scaledMask;
}

// Scale an image mask into a SplashBitmap.
SplashBitmap *Splash::scaleMask(SplashImageMaskSource src, void *srcData,
				int srcWidth, int srcHeight,
				int scaledWidth, int scaledHeight) {
  SplashBitmap *dest;

  dest = new SplashBitmap(scaledWidth, scaledHeight, 1, splashModeMono8,
			  gFalse);
  if (scaledHeight < srcHeight) {
    if (scaledWidth < srcWidth) {
      scaleMaskYdXd(src, srcData, srcWidth, srcHeight,
		    scaledWidth, scaledHeight, dest);
    } else {
      scaleMaskYdXu(src, srcData, srcWidth, srcHeight,
		    scaledWidth, scaledHeight, dest);
    }
  } else {
    if (scaledWidth < srcWidth) {
      scaleMaskYuXd(src, srcData, srcWidth, srcHeight,
		    scaledWidth, scaledHeight, dest);
    } else {
      scaleMaskYuXu(src, srcData, srcWidth, srcHeight,
		    scaledWidth, scaledHeight, dest);
    }
  }
  return dest;
}

void Splash::scaleMaskYdXd(SplashImageMaskSource src, void *srcData,
			   int srcWidth, int srcHeight,
			   int scaledWidth, int scaledHeight,
			   SplashBitmap *dest) {
  Guchar *lineBuf;
  Guint *pixBuf;
  Guint pix;
  Guchar *destPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, d, d0, d1;
  int i, j;

  // Bresenham parameters for y scale
  yp = srcHeight / scaledHeight;
  yq = srcHeight % scaledHeight;

  // Bresenham parameters for x scale
  xp = srcWidth / scaledWidth;
  xq = srcWidth % scaledWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmalloc(srcWidth);
  pixBuf = (Guint *)gmallocn(srcWidth, sizeof(int));

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  for (y = 0; y < scaledHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= scaledHeight) {
      yt -= scaledHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read rows from image
    memset(pixBuf, 0, srcWidth * sizeof(int));
    for (i = 0; i < yStep; ++i) {
      (*src)(srcData, lineBuf);
      for (j = 0; j < srcWidth; ++j) {
	pixBuf[j] += lineBuf[j];
      }
    }

    // init x scale Bresenham
    xt = 0;
    d0 = (255 << 23) / (yStep * xp);
    d1 = (255 << 23) / (yStep * (xp + 1));

    xx = 0;
    for (x = 0; x < scaledWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= scaledWidth) {
	xt -= scaledWidth;
	xStep = xp + 1;
	d = d1;
      } else {
	xStep = xp;
	d = d0;
      }

      // compute the final pixel
      pix = 0;
      for (i = 0; i < xStep; ++i) {
	pix += pixBuf[xx++];
      }
      // (255 * pix) / xStep * yStep
      pix = (pix * d) >> 23;

      // store the pixel
      *destPtr++ = (Guchar)pix;
    }
  }

  gfree(pixBuf);
  gfree(lineBuf);
}

void Splash::scaleMaskYdXu(SplashImageMaskSource src, void *srcData,
			   int srcWidth, int srcHeight,
			   int scaledWidth, int scaledHeight,
			   SplashBitmap *dest) {
  Guchar *lineBuf;
  Guint *pixBuf;
  Guint pix;
  Guchar *destPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, d;
  int i, j;

  // Bresenham parameters for y scale
  yp = srcHeight / scaledHeight;
  yq = srcHeight % scaledHeight;

  // Bresenham parameters for x scale
  xp = scaledWidth / srcWidth;
  xq = scaledWidth % srcWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmalloc(srcWidth);
  pixBuf = (Guint *)gmallocn(srcWidth, sizeof(int));

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  for (y = 0; y < scaledHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= scaledHeight) {
      yt -= scaledHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read rows from image
    memset(pixBuf, 0, srcWidth * sizeof(int));
    for (i = 0; i < yStep; ++i) {
      (*src)(srcData, lineBuf);
      for (j = 0; j < srcWidth; ++j) {
	pixBuf[j] += lineBuf[j];
      }
    }

    // init x scale Bresenham
    xt = 0;
    d = (255 << 23) / yStep;

    for (x = 0; x < srcWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= srcWidth) {
	xt -= srcWidth;
	xStep = xp + 1;
      } else {
	xStep = xp;
      }

      // compute the final pixel
      pix = pixBuf[x];
      // (255 * pix) / yStep
      pix = (pix * d) >> 23;

      // store the pixel
      for (i = 0; i < xStep; ++i) {
	*destPtr++ = (Guchar)pix;
      }
    }
  }

  gfree(pixBuf);
  gfree(lineBuf);
}

void Splash::scaleMaskYuXd(SplashImageMaskSource src, void *srcData,
			   int srcWidth, int srcHeight,
			   int scaledWidth, int scaledHeight,
			   SplashBitmap *dest) {
  Guchar *lineBuf;
  Guint pix;
  Guchar *destPtr0, *destPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, d, d0, d1;
  int i;

  // Bresenham parameters for y scale
  yp = scaledHeight / srcHeight;
  yq = scaledHeight % srcHeight;

  // Bresenham parameters for x scale
  xp = srcWidth / scaledWidth;
  xq = srcWidth % scaledWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmalloc(srcWidth);

  // init y scale Bresenham
  yt = 0;

  destPtr0 = dest->data;
  for (y = 0; y < srcHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= srcHeight) {
      yt -= srcHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read row from image
    (*src)(srcData, lineBuf);

    // init x scale Bresenham
    xt = 0;
    d0 = (255 << 23) / xp;
    d1 = (255 << 23) / (xp + 1);

    xx = 0;
    for (x = 0; x < scaledWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= scaledWidth) {
	xt -= scaledWidth;
	xStep = xp + 1;
	d = d1;
      } else {
	xStep = xp;
	d = d0;
      }

      // compute the final pixel
      pix = 0;
      for (i = 0; i < xStep; ++i) {
	pix += lineBuf[xx++];
      }
      // (255 * pix) / xStep
      pix = (pix * d) >> 23;

      // store the pixel
      for (i = 0; i < yStep; ++i) {
	destPtr = destPtr0 + i * scaledWidth + x;
	*destPtr = (Guchar)pix;
      }
    }

    destPtr0 += yStep * scaledWidth;
  }

  gfree(lineBuf);
}

void Splash::scaleMaskYuXu(SplashImageMaskSource src, void *srcData,
			   int srcWidth, int srcHeight,
			   int scaledWidth, int scaledHeight,
			   SplashBitmap *dest) {
  Guchar *lineBuf;
  Guint pix;
  Guchar *destPtr0, *destPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx;
  int i, j;

  // Bresenham parameters for y scale
  yp = scaledHeight / srcHeight;
  yq = scaledHeight % srcHeight;

  // Bresenham parameters for x scale
  xp = scaledWidth / srcWidth;
  xq = scaledWidth % srcWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmalloc(srcWidth);

  // init y scale Bresenham
  yt = 0;

  destPtr0 = dest->data;
  for (y = 0; y < srcHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= srcHeight) {
      yt -= srcHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read row from image
    (*src)(srcData, lineBuf);

    // init x scale Bresenham
    xt = 0;

    xx = 0;
    for (x = 0; x < srcWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= srcWidth) {
	xt -= srcWidth;
	xStep = xp + 1;
      } else {
	xStep = xp;
      }

      // compute the final pixel
      pix = lineBuf[x] ? 255 : 0;

      // store the pixel
      for (i = 0; i < yStep; ++i) {
	for (j = 0; j < xStep; ++j) {
	  destPtr = destPtr0 + i * scaledWidth + xx + j;
	  *destPtr++ = (Guchar)pix;
	}
      }

      xx += xStep;
    }

    destPtr0 += yStep * scaledWidth;
  }

  gfree(lineBuf);
}

void Splash::blitMask(SplashBitmap *src, int xDest, int yDest,
		      SplashClipResult clipRes) {
  SplashPipe pipe;
  Guchar *p;
  int w, h, x, y;

  w = src->getWidth();
  h = src->getHeight();
  if (vectorAntialias && clipRes != splashClipAllInside) {
    pipeInit(&pipe, xDest, yDest, state->fillPattern, NULL,
	     (Guchar)splashRound(state->fillAlpha * 255), gTrue, gFalse);
    drawAAPixelInit();
    p = src->getDataPtr();
    for (y = 0; y < h; ++y) {
      for (x = 0; x < w; ++x) {
	pipe.shape = *p++;
	drawAAPixel(&pipe, xDest + x, yDest + y);
      }
    }
  } else {
    pipeInit(&pipe, xDest, yDest, state->fillPattern, NULL,
	     (Guchar)splashRound(state->fillAlpha * 255), gTrue, gFalse);
    p = src->getDataPtr();
    if (clipRes == splashClipAllInside) {
      for (y = 0; y < h; ++y) {
	pipeSetXY(&pipe, xDest, yDest + y);
	for (x = 0; x < w; ++x) {
	  if (*p) {
	    pipe.shape = *p;
	    (this->*pipe.run)(&pipe);
	  } else {
	    pipeIncX(&pipe);
	  }
	  ++p;
	}
      }
      updateModX(xDest);
      updateModX(xDest + w - 1);
      updateModY(yDest);
      updateModY(yDest + h - 1);
    } else {
      for (y = 0; y < h; ++y) {
	pipeSetXY(&pipe, xDest, yDest + y);
	for (x = 0; x < w; ++x) {
	  if (*p && state->clip->test(xDest + x, yDest + y)) {
	    pipe.shape = *p;
	    (this->*pipe.run)(&pipe);
	    updateModX(xDest + x);
	    updateModY(yDest + y);
	  } else {
	    pipeIncX(&pipe);
	  }
	  ++p;
	}
      }
    }
  }
}

SplashError Splash::drawImage(SplashImageSource src, void *srcData,
			      SplashColorMode srcMode, GBool srcAlpha,
			      int w, int h, SplashCoord *mat) {
  GBool ok;
  SplashBitmap *scaledImg;
  SplashClipResult clipRes;
  GBool minorAxisZero;
  int x0, y0, x1, y1, scaledWidth, scaledHeight;
  int nComps;

  if (debugMode) {
    printf("drawImage: srcMode=%d srcAlpha=%d w=%d h=%d mat=[%.2f %.2f %.2f %.2f %.2f %.2f]\n",
	   srcMode, srcAlpha, w, h, (double)mat[0], (double)mat[1], (double)mat[2],
	   (double)mat[3], (double)mat[4], (double)mat[5]);
  }

  // check color modes
  ok = gFalse; // make gcc happy
  nComps = 0; // make gcc happy
  switch (bitmap->mode) {
  case splashModeMono1:
  case splashModeMono8:
    ok = srcMode == splashModeMono8;
    nComps = 1;
    break;
  case splashModeRGB8:
    ok = srcMode == splashModeRGB8;
    nComps = 3;
    break;
  case splashModeBGR8:
    ok = srcMode == splashModeBGR8;
    nComps = 3;
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    ok = srcMode == splashModeCMYK8;
    nComps = 4;
    break;
#endif
  default:
    ok = gFalse;
    break;
  }
  if (!ok) {
    return splashErrModeMismatch;
  }

  // check for singular matrix
  if (splashAbs(mat[0] * mat[3] - mat[1] * mat[2]) < 0.000001) {
    return splashErrSingularMatrix;
  }

  minorAxisZero = mat[1] == 0 && mat[2] == 0;

  // scaling only
  if (mat[0] > 0 && minorAxisZero && mat[3] > 0) {
    x0 = imgCoordMungeLower(mat[4]);
    y0 = imgCoordMungeLower(mat[5]);
    x1 = imgCoordMungeUpper(mat[0] + mat[4]);
    y1 = imgCoordMungeUpper(mat[3] + mat[5]);
    // make sure narrow images cover at least one pixel
    if (x0 == x1) {
      ++x1;
    }
    if (y0 == y1) {
      ++y1;
    }
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha, w, h,
			     scaledWidth, scaledHeight);
      blitImage(scaledImg, srcAlpha, x0, y0, clipRes);
      delete scaledImg;
    }
    
  // scaling plus vertical flip
  } else if (mat[0] > 0 && minorAxisZero && mat[3] < 0) {
    x0 = imgCoordMungeLower(mat[4]);
    y0 = imgCoordMungeLower(mat[3] + mat[5]);
    x1 = imgCoordMungeUpper(mat[0] + mat[4]);
    y1 = imgCoordMungeUpper(mat[5]);
    if (x0 == x1) {
      if (mat[4] + 0.5 * mat[0] < x0) {
	--x0;
      } else {
	++x1;
      }
    }
    if (y0 == y1) {
      if (mat[5] + 0.5 * mat[1] < y0) {
	--y0;
      } else {
	++y1;
      }
    }
    clipRes = state->clip->testRect(x0, y0, x1 - 1, y1 - 1);
    opClipRes = clipRes;
    if (clipRes != splashClipAllOutside) {
      scaledWidth = x1 - x0;
      scaledHeight = y1 - y0;
      scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha, w, h,
			     scaledWidth, scaledHeight);
      vertFlipImage(scaledImg, scaledWidth, scaledHeight, nComps);
      blitImage(scaledImg, srcAlpha, x0, y0, clipRes);
      delete scaledImg;
    }

  // all other cases
  } else {
    arbitraryTransformImage(src, srcData, srcMode, nComps, srcAlpha,
			    w, h, mat);
  }

  return splashOk;
}

void Splash::arbitraryTransformImage(SplashImageSource src, void *srcData,
				     SplashColorMode srcMode, int nComps,
				     GBool srcAlpha,
				     int srcWidth, int srcHeight,
				     SplashCoord *mat) {
  SplashBitmap *scaledImg;
  SplashClipResult clipRes, clipRes2;
  SplashPipe pipe;
  SplashColor pixel;
  int scaledWidth, scaledHeight, t0, t1;
  SplashCoord r00, r01, r10, r11, det, ir00, ir01, ir10, ir11;
  SplashCoord vx[4], vy[4];
  int xMin, yMin, xMax, yMax;
  ImageSection section[3];
  int nSections;
  int y, xa, xb, x, i, xx, yy;

  // compute the four vertices of the target quadrilateral
  vx[0] = mat[4];                    vy[0] = mat[5];
  vx[1] = mat[2] + mat[4];           vy[1] = mat[3] + mat[5];
  vx[2] = mat[0] + mat[2] + mat[4];  vy[2] = mat[1] + mat[3] + mat[5];
  vx[3] = mat[0] + mat[4];           vy[3] = mat[1] + mat[5];

  // clipping
  xMin = imgCoordMungeLower(vx[0]);
  xMax = imgCoordMungeUpper(vx[0]);
  yMin = imgCoordMungeLower(vy[0]);
  yMax = imgCoordMungeUpper(vy[0]);
  for (i = 1; i < 4; ++i) {
    t0 = imgCoordMungeLower(vx[i]);
    if (t0 < xMin) {
      xMin = t0;
    }
    t0 = imgCoordMungeUpper(vx[i]);
    if (t0 > xMax) {
      xMax = t0;
    }
    t1 = imgCoordMungeLower(vy[i]);
    if (t1 < yMin) {
      yMin = t1;
    }
    t1 = imgCoordMungeUpper(vy[i]);
    if (t1 > yMax) {
      yMax = t1;
    }
  }
  clipRes = state->clip->testRect(xMin, yMin, xMax - 1, yMax - 1);
  opClipRes = clipRes;
  if (clipRes == splashClipAllOutside) {
    return;
  }

  // compute the scale factors
  if (mat[0] >= 0) {
    t0 = imgCoordMungeUpper(mat[0] + mat[4]) - imgCoordMungeLower(mat[4]);
  } else {
    t0 = imgCoordMungeUpper(mat[4]) - imgCoordMungeLower(mat[0] + mat[4]);
  }
  if (mat[1] >= 0) {
    t1 = imgCoordMungeUpper(mat[1] + mat[5]) - imgCoordMungeLower(mat[5]);
  } else {
    t1 = imgCoordMungeUpper(mat[5]) - imgCoordMungeLower(mat[1] + mat[5]);
  }
  scaledWidth = t0 > t1 ? t0 : t1;
  if (mat[2] >= 0) {
    t0 = imgCoordMungeUpper(mat[2] + mat[4]) - imgCoordMungeLower(mat[4]);
  } else {
    t0 = imgCoordMungeUpper(mat[4]) - imgCoordMungeLower(mat[2] + mat[4]);
  }
  if (mat[3] >= 0) {
    t1 = imgCoordMungeUpper(mat[3] + mat[5]) - imgCoordMungeLower(mat[5]);
  } else {
    t1 = imgCoordMungeUpper(mat[5]) - imgCoordMungeLower(mat[3] + mat[5]);
  }
  scaledHeight = t0 > t1 ? t0 : t1;
  if (scaledWidth == 0) {
    scaledWidth = 1;
  }
  if (scaledHeight == 0) {
    scaledHeight = 1;
  }

  // compute the inverse transform (after scaling) matrix
  r00 = mat[0] / scaledWidth;
  r01 = mat[1] / scaledWidth;
  r10 = mat[2] / scaledHeight;
  r11 = mat[3] / scaledHeight;
  det = r00 * r11 - r01 * r10;
  if (splashAbs(det) < 1e-6) {
    // this should be caught by the singular matrix check in drawImage
    return;
  }
  ir00 = r11 / det;
  ir01 = -r01 / det;
  ir10 = -r10 / det;
  ir11 = r00 / det;

  // scale the input image
  scaledImg = scaleImage(src, srcData, srcMode, nComps, srcAlpha,
			 srcWidth, srcHeight, scaledWidth, scaledHeight);

  // construct the three sections
  i = (vy[2] <= vy[3]) ? 2 : 3;
  if (vy[1] <= vy[i]) {
    i = 1;
  }
  if (vy[0] < vy[i] || (i != 3 && vy[0] == vy[i])) {
    i = 0;
  }
  if (vy[i] == vy[(i+1) & 3]) {
    section[0].y0 = imgCoordMungeLower(vy[i]);
    section[0].y1 = imgCoordMungeUpper(vy[(i+2) & 3]) - 1;
    if (vx[i] < vx[(i+1) & 3]) {
      section[0].ia0 = i;
      section[0].ia1 = (i+3) & 3;
      section[0].ib0 = (i+1) & 3;
      section[0].ib1 = (i+2) & 3;
    } else {
      section[0].ia0 = (i+1) & 3;
      section[0].ia1 = (i+2) & 3;
      section[0].ib0 = i;
      section[0].ib1 = (i+3) & 3;
    }
    nSections = 1;
  } else {
    section[0].y0 = imgCoordMungeLower(vy[i]);
    section[2].y1 = imgCoordMungeUpper(vy[(i+2) & 3]) - 1;
    section[0].ia0 = section[0].ib0 = i;
    section[2].ia1 = section[2].ib1 = (i+2) & 3;
    if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
      section[0].ia1 = section[2].ia0 = (i+1) & 3;
      section[0].ib1 = section[2].ib0 = (i+3) & 3;
    } else {
      section[0].ia1 = section[2].ia0 = (i+3) & 3;
      section[0].ib1 = section[2].ib0 = (i+1) & 3;
    }
    if (vy[(i+1) & 3] < vy[(i+3) & 3]) {
      section[1].y0 = imgCoordMungeLower(vy[(i+1) & 3]);
      section[2].y0 = imgCoordMungeUpper(vy[(i+3) & 3]);
      if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
	section[1].ia0 = (i+1) & 3;
	section[1].ia1 = (i+2) & 3;
	section[1].ib0 = i;
	section[1].ib1 = (i+3) & 3;
      } else {
	section[1].ia0 = i;
	section[1].ia1 = (i+3) & 3;
	section[1].ib0 = (i+1) & 3;
	section[1].ib1 = (i+2) & 3;
      }
    } else {
      section[1].y0 = imgCoordMungeLower(vy[(i+3) & 3]);
      section[2].y0 = imgCoordMungeUpper(vy[(i+1) & 3]);
      if (vx[(i+1) & 3] < vx[(i+3) & 3]) {
	section[1].ia0 = i;
	section[1].ia1 = (i+1) & 3;
	section[1].ib0 = (i+3) & 3;
	section[1].ib1 = (i+2) & 3;
      } else {
	section[1].ia0 = (i+3) & 3;
	section[1].ia1 = (i+2) & 3;
	section[1].ib0 = i;
	section[1].ib1 = (i+1) & 3;
      }
    }
    section[0].y1 = section[1].y0 - 1;
    section[1].y1 = section[2].y0 - 1;
    nSections = 3;
  }
  for (i = 0; i < nSections; ++i) {
    section[i].xa0 = vx[section[i].ia0];
    section[i].ya0 = vy[section[i].ia0];
    section[i].xa1 = vx[section[i].ia1];
    section[i].ya1 = vy[section[i].ia1];
    section[i].xb0 = vx[section[i].ib0];
    section[i].yb0 = vy[section[i].ib0];
    section[i].xb1 = vx[section[i].ib1];
    section[i].yb1 = vy[section[i].ib1];
    section[i].dxdya = (section[i].xa1 - section[i].xa0) /
                       (section[i].ya1 - section[i].ya0);
    section[i].dxdyb = (section[i].xb1 - section[i].xb0) /
                       (section[i].yb1 - section[i].yb0);
  }

  // initialize the pixel pipe
  pipeInit(&pipe, 0, 0, NULL, pixel,
	   (Guchar)splashRound(state->fillAlpha * 255),
	   srcAlpha || (vectorAntialias && clipRes != splashClipAllInside),
	   gFalse);
  if (vectorAntialias) {
    drawAAPixelInit();
  }

  // make sure narrow images cover at least one pixel
  if (nSections == 1) {
    if (section[0].y0 == section[0].y1) {
      ++section[0].y1;
      clipRes = opClipRes = splashClipPartial;
    }
  } else {
    if (section[0].y0 == section[2].y1) {
      ++section[1].y1;
      clipRes = opClipRes = splashClipPartial;
    }
  }

  // scan all pixels inside the target region
  for (i = 0; i < nSections; ++i) {
    for (y = section[i].y0; y <= section[i].y1; ++y) {
      xa = imgCoordMungeLower(section[i].xa0 +
			      (y + 0.5 - section[i].ya0) * section[i].dxdya);
      xb = imgCoordMungeUpper(section[i].xb0 +
			      (y + 0.5 - section[i].yb0) * section[i].dxdyb);
      // make sure narrow images cover at least one pixel
      if (xa == xb) {
	++xb;
      }
      if (clipRes != splashClipAllInside) {
	clipRes2 = state->clip->testSpan(xa, xb - 1, y);
      } else {
	clipRes2 = clipRes;
      }
      for (x = xa; x < xb; ++x) {
	// map (x+0.5, y+0.5) back to the scaled image
	xx = splashFloor((x + 0.5 - mat[4]) * ir00 +
			 (y + 0.5 - mat[5]) * ir10);
	yy = splashFloor((x + 0.5 - mat[4]) * ir01 +
			 (y + 0.5 - mat[5]) * ir11);
	// xx should always be within bounds, but floating point
	// inaccuracy can cause problems
	if (xx < 0) {
	  xx = 0;
	} else if (xx >= scaledWidth) {
	  xx = scaledWidth - 1;
	}
	if (yy < 0) {
	  yy = 0;
	} else if (yy >= scaledHeight) {
	  yy = scaledHeight - 1;
	}
	scaledImg->getPixel(xx, yy, pixel);
	if (srcAlpha) {
	  pipe.shape = scaledImg->alpha[yy * scaledWidth + xx];
	} else {
	  pipe.shape = 255;
	}
	if (vectorAntialias && clipRes2 != splashClipAllInside) {
	  drawAAPixel(&pipe, x, y);
	} else {
	  drawPixel(&pipe, x, y, clipRes2 == splashClipAllInside);
	}
      }
    }
  }

  delete scaledImg;
}

// Scale an image into a SplashBitmap.
SplashBitmap *Splash::scaleImage(SplashImageSource src, void *srcData,
				 SplashColorMode srcMode, int nComps,
				 GBool srcAlpha, int srcWidth, int srcHeight,
				 int scaledWidth, int scaledHeight) {
  SplashBitmap *dest;

  dest = new SplashBitmap(scaledWidth, scaledHeight, 1, srcMode, srcAlpha);
  if (scaledHeight < srcHeight) {
    if (scaledWidth < srcWidth) {
      scaleImageYdXd(src, srcData, srcMode, nComps, srcAlpha,
		     srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
    } else {
      scaleImageYdXu(src, srcData, srcMode, nComps, srcAlpha,
		     srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
    }
  } else {
    if (scaledWidth < srcWidth) {
      scaleImageYuXd(src, srcData, srcMode, nComps, srcAlpha,
		     srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
    } else {
      scaleImageYuXu(src, srcData, srcMode, nComps, srcAlpha,
		     srcWidth, srcHeight, scaledWidth, scaledHeight, dest);
    }
  }
  return dest;
}

void Splash::scaleImageYdXd(SplashImageSource src, void *srcData,
			    SplashColorMode srcMode, int nComps,
			    GBool srcAlpha, int srcWidth, int srcHeight,
			    int scaledWidth, int scaledHeight,
			    SplashBitmap *dest) {
  Guchar *lineBuf, *alphaLineBuf;
  Guint *pixBuf, *alphaPixBuf;
  Guint pix0, pix1, pix2, pix3;
  Guint alpha;
  Guchar *destPtr, *destAlphaPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, xxa, d, d0, d1;
  int i, j;

  // Bresenham parameters for y scale
  yp = srcHeight / scaledHeight;
  yq = srcHeight % scaledHeight;

  // Bresenham parameters for x scale
  xp = srcWidth / scaledWidth;
  xq = srcWidth % scaledWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmallocn(srcWidth, nComps);
  pixBuf = (Guint *)gmallocn(srcWidth, nComps * sizeof(int));
  if (srcAlpha) {
    alphaLineBuf = (Guchar *)gmalloc(srcWidth);
    alphaPixBuf = (Guint *)gmallocn(srcWidth, sizeof(int));
  } else {
    alphaLineBuf = NULL;
    alphaPixBuf = NULL;
  }

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  destAlphaPtr = dest->alpha;
  for (y = 0; y < scaledHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= scaledHeight) {
      yt -= scaledHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read rows from image
    memset(pixBuf, 0, srcWidth * nComps * sizeof(int));
    if (srcAlpha) {
      memset(alphaPixBuf, 0, srcWidth * sizeof(int));
    }
    for (i = 0; i < yStep; ++i) {
      (*src)(srcData, lineBuf, alphaLineBuf);
      for (j = 0; j < srcWidth * nComps; ++j) {
	pixBuf[j] += lineBuf[j];
      }
      if (srcAlpha) {
	for (j = 0; j < srcWidth; ++j) {
	  alphaPixBuf[j] += alphaLineBuf[j];
	}
      }
    }

    // init x scale Bresenham
    xt = 0;
    d0 = (1 << 23) / (yStep * xp);
    d1 = (1 << 23) / (yStep * (xp + 1));

    xx = xxa = 0;
    for (x = 0; x < scaledWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= scaledWidth) {
	xt -= scaledWidth;
	xStep = xp + 1;
	d = d1;
      } else {
	xStep = xp;
	d = d0;
      }

      switch (srcMode) {

      case splashModeMono8:

	// compute the final pixel
	pix0 = 0;
	for (i = 0; i < xStep; ++i) {
	  pix0 += pixBuf[xx++];
	}
	// pix / xStep * yStep
	pix0 = (pix0 * d) >> 23;

	// store the pixel
	*destPtr++ = (Guchar)pix0;
	break;

      case splashModeRGB8:

	// compute the final pixel
	pix0 = pix1 = pix2 = 0;
	for (i = 0; i < xStep; ++i) {
	  pix0 += pixBuf[xx];
	  pix1 += pixBuf[xx+1];
	  pix2 += pixBuf[xx+2];
	  xx += 3;
	}
	// pix / xStep * yStep
	pix0 = (pix0 * d) >> 23;
	pix1 = (pix1 * d) >> 23;
	pix2 = (pix2 * d) >> 23;

	// store the pixel
	*destPtr++ = (Guchar)pix0;
	*destPtr++ = (Guchar)pix1;
	*destPtr++ = (Guchar)pix2;
	break;

      case splashModeBGR8:

	// compute the final pixel
	pix0 = pix1 = pix2 = 0;
	for (i = 0; i < xStep; ++i) {
	  pix0 += pixBuf[xx];
	  pix1 += pixBuf[xx+1];
	  pix2 += pixBuf[xx+2];
	  xx += 3;
	}
	// pix / xStep * yStep
	pix0 = (pix0 * d) >> 23;
	pix1 = (pix1 * d) >> 23;
	pix2 = (pix2 * d) >> 23;

	// store the pixel
	*destPtr++ = (Guchar)pix2;
	*destPtr++ = (Guchar)pix1;
	*destPtr++ = (Guchar)pix0;
	break;

#if SPLASH_CMYK
      case splashModeCMYK8:

	// compute the final pixel
	pix0 = pix1 = pix2 = pix3 = 0;
	for (i = 0; i < xStep; ++i) {
	  pix0 += pixBuf[xx];
	  pix1 += pixBuf[xx+1];
	  pix2 += pixBuf[xx+2];
	  pix3 += pixBuf[xx+3];
	  xx += 4;
	}
	// pix / xStep * yStep
	pix0 = (pix0 * d) >> 23;
	pix1 = (pix1 * d) >> 23;
	pix2 = (pix2 * d) >> 23;
	pix3 = (pix3 * d) >> 23;

	// store the pixel
	*destPtr++ = (Guchar)pix0;
	*destPtr++ = (Guchar)pix1;
	*destPtr++ = (Guchar)pix2;
	*destPtr++ = (Guchar)pix3;
	break;
#endif


      case splashModeMono1: // mono1 is not allowed
      default:
	break;
      }

      // process alpha
      if (srcAlpha) {
	alpha = 0;
	for (i = 0; i < xStep; ++i, ++xxa) {
	  alpha += alphaPixBuf[xxa];
	}
	// alpha / xStep * yStep
	alpha = (alpha * d) >> 23;
	*destAlphaPtr++ = (Guchar)alpha;
      }
    }
  }

  gfree(alphaPixBuf);
  gfree(alphaLineBuf);
  gfree(pixBuf);
  gfree(lineBuf);
}

void Splash::scaleImageYdXu(SplashImageSource src, void *srcData,
			    SplashColorMode srcMode, int nComps,
			    GBool srcAlpha, int srcWidth, int srcHeight,
			    int scaledWidth, int scaledHeight,
			    SplashBitmap *dest) {
  Guchar *lineBuf, *alphaLineBuf;
  Guint *pixBuf, *alphaPixBuf;
  Guint pix[splashMaxColorComps];
  Guint alpha;
  Guchar *destPtr, *destAlphaPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, d;
  int i, j;

  // Bresenham parameters for y scale
  yp = srcHeight / scaledHeight;
  yq = srcHeight % scaledHeight;

  // Bresenham parameters for x scale
  xp = scaledWidth / srcWidth;
  xq = scaledWidth % srcWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmallocn(srcWidth, nComps);
  pixBuf = (Guint *)gmallocn(srcWidth, nComps * sizeof(int));
  if (srcAlpha) {
    alphaLineBuf = (Guchar *)gmalloc(srcWidth);
    alphaPixBuf = (Guint *)gmallocn(srcWidth, sizeof(int));
  } else {
    alphaLineBuf = NULL;
    alphaPixBuf = NULL;
  }

  // init y scale Bresenham
  yt = 0;

  destPtr = dest->data;
  destAlphaPtr = dest->alpha;
  for (y = 0; y < scaledHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= scaledHeight) {
      yt -= scaledHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read rows from image
    memset(pixBuf, 0, srcWidth * nComps * sizeof(int));
    if (srcAlpha) {
      memset(alphaPixBuf, 0, srcWidth * sizeof(int));
    }
    for (i = 0; i < yStep; ++i) {
      (*src)(srcData, lineBuf, alphaLineBuf);
      for (j = 0; j < srcWidth * nComps; ++j) {
	pixBuf[j] += lineBuf[j];
      }
      if (srcAlpha) {
	for (j = 0; j < srcWidth; ++j) {
	  alphaPixBuf[j] += alphaLineBuf[j];
	}
      }
    }

    // init x scale Bresenham
    xt = 0;
    d = (1 << 23) / yStep;

    for (x = 0; x < srcWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= srcWidth) {
	xt -= srcWidth;
	xStep = xp + 1;
      } else {
	xStep = xp;
      }

      // compute the final pixel
      for (i = 0; i < nComps; ++i) {
	// pixBuf[] / yStep
	pix[i] = (pixBuf[x * nComps + i] * d) >> 23;
      }

      // store the pixel
      switch (srcMode) {
      case splashModeMono1: // mono1 is not allowed
	break;
      case splashModeMono8:
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = (Guchar)pix[0];
	}
	break;
      case splashModeRGB8:
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = (Guchar)pix[0];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[2];
	}
	break;
      case splashModeBGR8:
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = (Guchar)pix[2];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[0];
	}
	break;
#if SPLASH_CMYK
      case splashModeCMYK8:
	for (i = 0; i < xStep; ++i) {
	  *destPtr++ = (Guchar)pix[0];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[2];
	  *destPtr++ = (Guchar)pix[3];
	}
	break;
#endif
      }

      // process alpha
      if (srcAlpha) {
	// alphaPixBuf[] / yStep
	alpha = (alphaPixBuf[x] * d) >> 23;
	for (i = 0; i < xStep; ++i) {
	  *destAlphaPtr++ = (Guchar)alpha;
	}
      }
    }
  }

  gfree(alphaPixBuf);
  gfree(alphaLineBuf);
  gfree(pixBuf);
  gfree(lineBuf);
}

void Splash::scaleImageYuXd(SplashImageSource src, void *srcData,
			    SplashColorMode srcMode, int nComps,
			    GBool srcAlpha, int srcWidth, int srcHeight,
			    int scaledWidth, int scaledHeight,
			    SplashBitmap *dest) {
  Guchar *lineBuf, *alphaLineBuf;
  Guint pix[splashMaxColorComps];
  Guint alpha;
  Guchar *destPtr0, *destPtr, *destAlphaPtr0, *destAlphaPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx, xxa, d, d0, d1;
  int i, j;

  // Bresenham parameters for y scale
  yp = scaledHeight / srcHeight;
  yq = scaledHeight % srcHeight;

  // Bresenham parameters for x scale
  xp = srcWidth / scaledWidth;
  xq = srcWidth % scaledWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmallocn(srcWidth, nComps);
  if (srcAlpha) {
    alphaLineBuf = (Guchar *)gmalloc(srcWidth);
  } else {
    alphaLineBuf = NULL;
  }

  // init y scale Bresenham
  yt = 0;

  destPtr0 = dest->data;
  destAlphaPtr0 = dest->alpha;
  for (y = 0; y < srcHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= srcHeight) {
      yt -= srcHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read row from image
    (*src)(srcData, lineBuf, alphaLineBuf);

    // init x scale Bresenham
    xt = 0;
    d0 = (1 << 23) / xp;
    d1 = (1 << 23) / (xp + 1);

    xx = xxa = 0;
    for (x = 0; x < scaledWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= scaledWidth) {
	xt -= scaledWidth;
	xStep = xp + 1;
	d = d1;
      } else {
	xStep = xp;
	d = d0;
      }

      // compute the final pixel
      for (i = 0; i < nComps; ++i) {
	pix[i] = 0;
      }
      for (i = 0; i < xStep; ++i) {
	for (j = 0; j < nComps; ++j, ++xx) {
	  pix[j] += lineBuf[xx];
	}
      }
      for (i = 0; i < nComps; ++i) {
	// pix[] / xStep
	pix[i] = (pix[i] * d) >> 23;
      }

      // store the pixel
      switch (srcMode) {
      case splashModeMono1: // mono1 is not allowed
	break;
      case splashModeMono8:
	for (i = 0; i < yStep; ++i) {
	  destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
	  *destPtr++ = (Guchar)pix[0];
	}
	break;
      case splashModeRGB8:
	for (i = 0; i < yStep; ++i) {
	  destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
	  *destPtr++ = (Guchar)pix[0];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[2];
	}
	break;
      case splashModeBGR8:
	for (i = 0; i < yStep; ++i) {
	  destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
	  *destPtr++ = (Guchar)pix[2];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[0];
	}
	break;
#if SPLASH_CMYK
      case splashModeCMYK8:
	for (i = 0; i < yStep; ++i) {
	  destPtr = destPtr0 + (i * scaledWidth + x) * nComps;
	  *destPtr++ = (Guchar)pix[0];
	  *destPtr++ = (Guchar)pix[1];
	  *destPtr++ = (Guchar)pix[2];
	  *destPtr++ = (Guchar)pix[3];
	}
	break;
#endif
      }

      // process alpha
      if (srcAlpha) {
	alpha = 0;
	for (i = 0; i < xStep; ++i, ++xxa) {
	  alpha += alphaLineBuf[xxa];
	}
	// alpha / xStep
	alpha = (alpha * d) >> 23;
	for (i = 0; i < yStep; ++i) {
	  destAlphaPtr = destAlphaPtr0 + i * scaledWidth + x;
	  *destAlphaPtr = (Guchar)alpha;
	}
      }
    }

    destPtr0 += yStep * scaledWidth * nComps;
    if (srcAlpha) {
      destAlphaPtr0 += yStep * scaledWidth;
    }
  }

  gfree(alphaLineBuf);
  gfree(lineBuf);
}

void Splash::scaleImageYuXu(SplashImageSource src, void *srcData,
			    SplashColorMode srcMode, int nComps,
			    GBool srcAlpha, int srcWidth, int srcHeight,
			    int scaledWidth, int scaledHeight,
			    SplashBitmap *dest) {
  Guchar *lineBuf, *alphaLineBuf;
  Guint pix[splashMaxColorComps];
  Guint alpha;
  Guchar *destPtr0, *destPtr, *destAlphaPtr0, *destAlphaPtr;
  int yp, yq, xp, xq, yt, y, yStep, xt, x, xStep, xx;
  int i, j;

  // Bresenham parameters for y scale
  yp = scaledHeight / srcHeight;
  yq = scaledHeight % srcHeight;

  // Bresenham parameters for x scale
  xp = scaledWidth / srcWidth;
  xq = scaledWidth % srcWidth;

  // allocate buffers
  lineBuf = (Guchar *)gmallocn(srcWidth, nComps);
  if (srcAlpha) {
    alphaLineBuf = (Guchar *)gmalloc(srcWidth);
  } else {
    alphaLineBuf = NULL;
  }

  // init y scale Bresenham
  yt = 0;

  destPtr0 = dest->data;
  destAlphaPtr0 = dest->alpha;
  for (y = 0; y < srcHeight; ++y) {

    // y scale Bresenham
    if ((yt += yq) >= srcHeight) {
      yt -= srcHeight;
      yStep = yp + 1;
    } else {
      yStep = yp;
    }

    // read row from image
    (*src)(srcData, lineBuf, alphaLineBuf);

    // init x scale Bresenham
    xt = 0;

    xx = 0;
    for (x = 0; x < srcWidth; ++x) {

      // x scale Bresenham
      if ((xt += xq) >= srcWidth) {
	xt -= srcWidth;
	xStep = xp + 1;
      } else {
	xStep = xp;
      }

      // compute the final pixel
      for (i = 0; i < nComps; ++i) {
	pix[i] = lineBuf[x * nComps + i];
      }

      // store the pixel
      switch (srcMode) {
      case splashModeMono1: // mono1 is not allowed
	break;
      case splashModeMono8:
	for (i = 0; i < yStep; ++i) {
	  for (j = 0; j < xStep; ++j) {
	    destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
	    *destPtr++ = (Guchar)pix[0];
	  }
	}
	break;
      case splashModeRGB8:
	for (i = 0; i < yStep; ++i) {
	  for (j = 0; j < xStep; ++j) {
	    destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
	    *destPtr++ = (Guchar)pix[0];
	    *destPtr++ = (Guchar)pix[1];
	    *destPtr++ = (Guchar)pix[2];
	  }
	}
	break;
      case splashModeBGR8:
	for (i = 0; i < yStep; ++i) {
	  for (j = 0; j < xStep; ++j) {
	    destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
	    *destPtr++ = (Guchar)pix[2];
	    *destPtr++ = (Guchar)pix[1];
	    *destPtr++ = (Guchar)pix[0];
	  }
	}
	break;
#if SPLASH_CMYK
      case splashModeCMYK8:
	for (i = 0; i < yStep; ++i) {
	  for (j = 0; j < xStep; ++j) {
	    destPtr = destPtr0 + (i * scaledWidth + xx + j) * nComps;
	    *destPtr++ = (Guchar)pix[0];
	    *destPtr++ = (Guchar)pix[1];
	    *destPtr++ = (Guchar)pix[2];
	    *destPtr++ = (Guchar)pix[3];
	  }
	}
	break;
#endif
      }

      // process alpha
      if (srcAlpha) {
	alpha = alphaLineBuf[x];
	for (i = 0; i < yStep; ++i) {
	  for (j = 0; j < xStep; ++j) {
	    destAlphaPtr = destAlphaPtr0 + i * scaledWidth + xx + j;
	    *destAlphaPtr = (Guchar)alpha;
	  }
	}
      }

      xx += xStep;
    }

    destPtr0 += yStep * scaledWidth * nComps;
    if (srcAlpha) {
      destAlphaPtr0 += yStep * scaledWidth;
    }
  }

  gfree(alphaLineBuf);
  gfree(lineBuf);
}

void Splash::vertFlipImage(SplashBitmap *img, int width, int height,
			   int nComps) {
  Guchar *lineBuf;
  Guchar *p0, *p1;
  int w;

  w = width * nComps;
  lineBuf = (Guchar *)gmalloc(w);
  for (p0 = img->data, p1 = img->data + (height - 1) * w;
       p0 < p1;
       p0 += w, p1 -= w) {
    memcpy(lineBuf, p0, w);
    memcpy(p0, p1, w);
    memcpy(p1, lineBuf, w);
  }
  if (img->alpha) {
    for (p0 = img->alpha, p1 = img->alpha + (height - 1) * width;
	 p0 < p1;
	 p0 += width, p1 -= width) {
      memcpy(lineBuf, p0, width);
      memcpy(p0, p1, width);
      memcpy(p1, lineBuf, width);
    }
  }
  gfree(lineBuf);
}

void Splash::blitImage(SplashBitmap *src, GBool srcAlpha, int xDest, int yDest,
		       SplashClipResult clipRes) {
  SplashPipe pipe;
  SplashColor pixel;
  Guchar *ap;
  int w, h, x0, y0, x1, y1, x, y;

  // split the image into clipped and unclipped regions
  w = src->getWidth();
  h = src->getHeight();
  if (clipRes == splashClipAllInside) {
    x0 = 0;
    y0 = 0;
    x1 = w;
    y1 = h;
  } else {
    if (state->clip->getNumPaths()) {
      x0 = x1 = w;
      y0 = y1 = h;
    } else {
      if ((x0 = splashCeil(state->clip->getXMin()) - xDest) < 0) {
	x0 = 0;
      }
      if ((y0 = splashCeil(state->clip->getYMin()) - yDest) < 0) {
	y0 = 0;
      }
      if ((x1 = splashFloor(state->clip->getXMax()) - xDest) > w) {
	x1 = w;
      }
      if (x1 < x0) {
	x1 = x0;
      }
      if ((y1 = splashFloor(state->clip->getYMax()) - yDest) > h) {
	y1 = h;
      }
      if (y1 < y0) {
	y1 = y0;
      }
    }
  }

  // draw the unclipped region
  if (x0 < w && y0 < h && x0 < x1 && y0 < y1) {
    pipeInit(&pipe, xDest + x0, yDest + y0, NULL, pixel,
	     (Guchar)splashRound(state->fillAlpha * 255), srcAlpha, gFalse);
    if (srcAlpha) {
      for (y = y0; y < y1; ++y) {
	pipeSetXY(&pipe, xDest + x0, yDest + y);
	ap = src->getAlphaPtr() + y * w + x0;
	for (x = x0; x < x1; ++x) {
	  src->getPixel(x, y, pixel);
	  pipe.shape = *ap++;
	  (this->*pipe.run)(&pipe);
	}
      }
    } else {
      for (y = y0; y < y1; ++y) {
	pipeSetXY(&pipe, xDest + x0, yDest + y);
	for (x = x0; x < x1; ++x) {
	  src->getPixel(x, y, pixel);
	  (this->*pipe.run)(&pipe);
	}
      }
    }
    updateModX(xDest + x0);
    updateModX(xDest + x1 - 1);
    updateModY(yDest + y0);
    updateModY(yDest + y1 - 1);
  }

  // draw the clipped regions
  if (y0 > 0) {
    blitImageClipped(src, srcAlpha, 0, 0, xDest, yDest, w, y0);
  }
  if (y1 < h) {
    blitImageClipped(src, srcAlpha, 0, y1, xDest, yDest + y1, w, h - y1);
  }
  if (x0 > 0 && y0 < y1) {
    blitImageClipped(src, srcAlpha, 0, y0, xDest, yDest + y0, x0, y1 - y0);
  }
  if (x1 < w && y0 < y1) {
    blitImageClipped(src, srcAlpha, x1, y0, xDest + x1, yDest + y0,
		     w - x1, y1 - y0);
  }
}

void Splash::blitImageClipped(SplashBitmap *src, GBool srcAlpha,
			      int xSrc, int ySrc, int xDest, int yDest,
			      int w, int h) {
  SplashPipe pipe;
  SplashColor pixel;
  Guchar *ap;
  int x, y;

  if (vectorAntialias) {
    pipeInit(&pipe, xDest, yDest, NULL, pixel,
	     (Guchar)splashRound(state->fillAlpha * 255), gTrue, gFalse);
    drawAAPixelInit();
    if (srcAlpha) {
      for (y = 0; y < h; ++y) {
	ap = src->getAlphaPtr() + (ySrc + y) * src->getWidth() + xSrc;
	for (x = 0; x < w; ++x) {
	  src->getPixel(xSrc + x, ySrc + y, pixel);
	  pipe.shape = *ap++;
	  drawAAPixel(&pipe, xDest + x, yDest + y);
	}
      }
    } else {
      for (y = 0; y < h; ++y) {
	for (x = 0; x < w; ++x) {
	  src->getPixel(xSrc + x, ySrc + y, pixel);
	  pipe.shape = 255;
	  drawAAPixel(&pipe, xDest + x, yDest + y);
	}
      }
    }
  } else {
    pipeInit(&pipe, xDest, yDest, NULL, pixel,
	     (Guchar)splashRound(state->fillAlpha * 255), srcAlpha, gFalse);
    if (srcAlpha) {
      for (y = 0; y < h; ++y) {
	ap = src->getAlphaPtr() + (ySrc + y) * src->getWidth() + xSrc;
	pipeSetXY(&pipe, xDest, yDest + y);
	for (x = 0; x < w; ++x) {
	  if (state->clip->test(xDest + x, yDest + y)) {
	    src->getPixel(xSrc + x, ySrc + y, pixel);
	    pipe.shape = *ap++;
	    (this->*pipe.run)(&pipe);
	    updateModX(xDest + x);
	    updateModY(yDest + y);
	  } else {
	    pipeIncX(&pipe);
	    ++ap;
	  }
	}
      }
    } else {
      for (y = 0; y < h; ++y) {
	pipeSetXY(&pipe, xDest, yDest + y);
	for (x = 0; x < w; ++x) {
	  if (state->clip->test(xDest + x, yDest + y)) {
	    src->getPixel(xSrc + x, ySrc + y, pixel);
	    (this->*pipe.run)(&pipe);
	    updateModX(xDest + x);
	    updateModY(yDest + y);
	  } else {
	    pipeIncX(&pipe);
	  }
	}
      }
    }
  }
}

SplashError Splash::composite(SplashBitmap *src, int xSrc, int ySrc,
			      int xDest, int yDest, int w, int h,
			      GBool noClip, GBool nonIsolated) {
  SplashPipe pipe;
  SplashColor pixel;
  Guchar alpha;
  Guchar *ap;
  int x, y;

  if (src->mode != bitmap->mode) {
    return splashErrModeMismatch;
  }

  if (src->alpha) {
    pipeInit(&pipe, xDest, yDest, NULL, pixel,
	     (Guchar)splashRound(state->fillAlpha * 255), gTrue, nonIsolated);
    if (noClip) {
      for (y = 0; y < h; ++y) {
	pipeSetXY(&pipe, xDest, yDest + y);
	ap = src->getAlphaPtr() + (ySrc + y) * src->getWidth() + xSrc;
	for (x = 0; x < w; ++x) {
	  src->getPixel(xSrc + x, ySrc + y, pixel);
	  alpha = *ap++;
	  // this uses shape instead of alpha, which isn't technically
	  // correct, but works out the same
	  pipe.shape = alpha;
	  (this->*pipe.run)(&pipe);
	}
      }
      updateModX(xDest);
      updateModX(xDest + w - 1);
      updateModY(yDest);
      updateModY(yDest + h - 1);
    } else {
      for (y = 0; y < h; ++y) {
	pipeSetXY(&pipe, xDest, yDest + y);
	ap = src->getAlphaPtr() + (ySrc + y) * src->getWidth() + xSrc;
	for (x = 0; x < w; ++x) {
	  src->getPixel(xSrc + x, ySrc + y, pixel);
	  alpha = *ap++;
	  if (state->clip->test(xDest + x, yDest + y)) {
	    // this uses shape instead of alpha, which isn't technically
	    // correct, but works out the same
	    pipe.shape = alpha;
	    (this->*pipe.run)(&pipe);
	    updateModX(xDest + x);
	    updateModY(yDest + y);
	  } else {
	    pipeIncX(&pipe);
	  }
	}
      }
    }
  } else {
    pipeInit(&pipe, xDest, yDest, NULL, pixel,
	     (Guchar)splashRound(state->fillAlpha * 255), gFalse, nonIsolated);
    if (noClip) {
      for (y = 0; y < h; ++y) {
	pipeSetXY(&pipe, xDest, yDest + y);
	for (x = 0; x < w; ++x) {
	  src->getPixel(xSrc + x, ySrc + y, pixel);
	  (this->*pipe.run)(&pipe);
	}
      }
      updateModX(xDest);
      updateModX(xDest + w - 1);
      updateModY(yDest);
      updateModY(yDest + h - 1);
    } else {
      for (y = 0; y < h; ++y) {
	pipeSetXY(&pipe, xDest, yDest + y);
	for (x = 0; x < w; ++x) {
	  src->getPixel(xSrc + x, ySrc + y, pixel);
	  if (state->clip->test(xDest + x, yDest + y)) {
	    (this->*pipe.run)(&pipe);
	    updateModX(xDest + x);
	    updateModY(yDest + y);
	  } else {
	    pipeIncX(&pipe);
	  }
	}
      }
    }
  }

  return splashOk;
}

void Splash::compositeBackground(SplashColorPtr color) {
  SplashColorPtr p;
  Guchar *q;
  Guchar alpha, alpha1, c, color0, color1, color2;
#if SPLASH_CMYK
  Guchar color3;
#endif
  int x, y, mask;

  switch (bitmap->mode) {
  case splashModeMono1:
    color0 = color[0];
    for (y = 0; y < bitmap->height; ++y) {
      p = &bitmap->data[y * bitmap->rowSize];
      q = &bitmap->alpha[y * bitmap->width];
      mask = 0x80;
      for (x = 0; x < bitmap->width; ++x) {
	alpha = *q++;
	alpha1 = 255 - alpha;
	c = (*p & mask) ? 0xff : 0x00;
	c = div255(alpha1 * color0 + alpha * c);
	if (c & 0x80) {
	  *p |= mask;
	} else {
	  *p &= ~mask;
	}
	if (!(mask >>= 1)) {
	  mask = 0x80;
	  ++p;
	}
      }
    }
    break;
  case splashModeMono8:
    color0 = color[0];
    for (y = 0; y < bitmap->height; ++y) {
      p = &bitmap->data[y * bitmap->rowSize];
      q = &bitmap->alpha[y * bitmap->width];
      for (x = 0; x < bitmap->width; ++x) {
	alpha = *q++;
	alpha1 = 255 - alpha;
	p[0] = div255(alpha1 * color0 + alpha * p[0]);
	++p;
      }
    }
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    color0 = color[0];
    color1 = color[1];
    color2 = color[2];
    for (y = 0; y < bitmap->height; ++y) {
      p = &bitmap->data[y * bitmap->rowSize];
      q = &bitmap->alpha[y * bitmap->width];
      for (x = 0; x < bitmap->width; ++x) {
	alpha = *q++;
	alpha1 = 255 - alpha;
	p[0] = div255(alpha1 * color0 + alpha * p[0]);
	p[1] = div255(alpha1 * color1 + alpha * p[1]);
	p[2] = div255(alpha1 * color2 + alpha * p[2]);
	p += 3;
      }
    }
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    color0 = color[0];
    color1 = color[1];
    color2 = color[2];
    color3 = color[3];
    for (y = 0; y < bitmap->height; ++y) {
      p = &bitmap->data[y * bitmap->rowSize];
      q = &bitmap->alpha[y * bitmap->width];
      for (x = 0; x < bitmap->width; ++x) {
	alpha = *q++;
	alpha1 = 255 - alpha;
	p[0] = div255(alpha1 * color0 + alpha * p[0]);
	p[1] = div255(alpha1 * color1 + alpha * p[1]);
	p[2] = div255(alpha1 * color2 + alpha * p[2]);
	p[3] = div255(alpha1 * color3 + alpha * p[3]);
	p += 4;
      }
    }
    break;
#endif
  }
  memset(bitmap->alpha, 255, bitmap->width * bitmap->height);
}

SplashError Splash::blitTransparent(SplashBitmap *src, int xSrc, int ySrc,
				    int xDest, int yDest, int w, int h) {
  SplashColorPtr p, q;
  int x, y, mask, srcMask;

  if (src->mode != bitmap->mode) {
    return splashErrModeMismatch;
  }

  switch (bitmap->mode) {
  case splashModeMono1:
    for (y = 0; y < h; ++y) {
      p = &bitmap->data[(yDest + y) * bitmap->rowSize + (xDest >> 3)];
      mask = 0x80 >> (xDest & 7);
      q = &src->data[(ySrc + y) * src->rowSize + (xSrc >> 3)];
      srcMask = 0x80 >> (xSrc & 7);
      for (x = 0; x < w; ++x) {
	if (*q & srcMask) {
	  *p |= mask;
	} else {
	  *p &= ~mask;
	}
	if (!(mask >>= 1)) {
	  mask = 0x80;
	  ++p;
	}
	if (!(srcMask >>= 1)) {
	  srcMask = 0x80;
	  ++q;
	}
      }
    }
    break;
  case splashModeMono8:
    for (y = 0; y < h; ++y) {
      p = &bitmap->data[(yDest + y) * bitmap->rowSize + xDest];
      q = &src->data[(ySrc + y) * src->rowSize + xSrc];
      for (x = 0; x < w; ++x) {
	*p++ = *q++;
      }
    }
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    for (y = 0; y < h; ++y) {
      p = &bitmap->data[(yDest + y) * bitmap->rowSize + 3 * xDest];
      q = &src->data[(ySrc + y) * src->rowSize + 3 * xSrc];
      for (x = 0; x < w; ++x) {
	*p++ = *q++;
	*p++ = *q++;
	*p++ = *q++;
      }
    }
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    for (y = 0; y < h; ++y) {
      p = &bitmap->data[(yDest + y) * bitmap->rowSize + 4 * xDest];
      q = &src->data[(ySrc + y) * src->rowSize + 4 * xSrc];
      for (x = 0; x < w; ++x) {
	*p++ = *q++;
	*p++ = *q++;
	*p++ = *q++;
	*p++ = *q++;
      }
    }
    break;
#endif
  }

  if (bitmap->alpha) {
    for (y = 0; y < h; ++y) {
      q = &bitmap->alpha[(yDest + y) * bitmap->width + xDest];
      for (x = 0; x < w; ++x) {
	*q++ = 0x00;
      }
    }
  }

  return splashOk;
}

SplashPath *Splash::makeStrokePath(SplashPath *path, GBool flatten) {
  SplashPath *pathIn, *dashPath, *pathOut;
  SplashCoord w, d, dx, dy, wdx, wdy, dxNext, dyNext, wdxNext, wdyNext;
  SplashCoord crossprod, dotprod, miter, m;
  GBool first, last, closed;
  int subpathStart0, subpathStart1, seg, i0, i1, j0, j1, k0, k1;
  int left0, left1, left2, right0, right1, right2, join0, join1, join2;
  int leftFirst, rightFirst, firstPt;

  pathOut = new SplashPath();

  if (path->length == 0) {
    return pathOut;
  }

  if (flatten) {
    pathIn = flattenPath(path, state->matrix, state->flatness);
    if (state->lineDashLength > 0) {
      dashPath = makeDashedPath(pathIn);
      delete pathIn;
      pathIn = dashPath;
    }
  } else {
    pathIn = path;
  }

  w = state->lineWidth;

  subpathStart0 = subpathStart1 = 0; // make gcc happy
  seg = 0; // make gcc happy
  closed = gFalse; // make gcc happy
  left0 = left1 = right0 = right1 = join0 = join1 = 0; // make gcc happy
  leftFirst = rightFirst = firstPt = 0; // make gcc happy

  i0 = 0;
  for (i1 = i0;
       !(pathIn->flags[i1] & splashPathLast) &&
	 i1 + 1 < pathIn->length &&
	 pathIn->pts[i1+1].x == pathIn->pts[i1].x &&
	 pathIn->pts[i1+1].y == pathIn->pts[i1].y;
       ++i1) ;

  while (i1 < pathIn->length) {
    if ((first = pathIn->flags[i0] & splashPathFirst)) {
      subpathStart0 = i0;
      subpathStart1 = i1;
      seg = 0;
      closed = pathIn->flags[i0] & splashPathClosed;
    }
    j0 = i1 + 1;
    if (j0 < pathIn->length) {
      for (j1 = j0;
	   !(pathIn->flags[j1] & splashPathLast) &&
	     j1 + 1 < pathIn->length &&
	     pathIn->pts[j1+1].x == pathIn->pts[j1].x &&
	     pathIn->pts[j1+1].y == pathIn->pts[j1].y;
	   ++j1) ;
    } else {
      j1 = j0;
    }
    if (pathIn->flags[i1] & splashPathLast) {
      if (first && state->lineCap == splashLineCapRound) {
	// special case: zero-length subpath with round line caps -->
	// draw a circle
	pathOut->moveTo(pathIn->pts[i0].x + (SplashCoord)0.5 * w,
			pathIn->pts[i0].y);
	pathOut->curveTo(pathIn->pts[i0].x + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y + bezierCircle2 * w,
			 pathIn->pts[i0].x + bezierCircle2 * w,
			 pathIn->pts[i0].y + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].x,
			 pathIn->pts[i0].y + (SplashCoord)0.5 * w);
	pathOut->curveTo(pathIn->pts[i0].x - bezierCircle2 * w,
			 pathIn->pts[i0].y + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].x - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y + bezierCircle2 * w,
			 pathIn->pts[i0].x - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y);
	pathOut->curveTo(pathIn->pts[i0].x - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y - bezierCircle2 * w,
			 pathIn->pts[i0].x - bezierCircle2 * w,
			 pathIn->pts[i0].y - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].x,
			 pathIn->pts[i0].y - (SplashCoord)0.5 * w);
	pathOut->curveTo(pathIn->pts[i0].x + bezierCircle2 * w,
			 pathIn->pts[i0].y - (SplashCoord)0.5 * w,
			 pathIn->pts[i0].x + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y - bezierCircle2 * w,
			 pathIn->pts[i0].x + (SplashCoord)0.5 * w,
			 pathIn->pts[i0].y);
	pathOut->close();
      }
      i0 = j0;
      i1 = j1;
      continue;
    }
    last = pathIn->flags[j1] & splashPathLast;
    if (last) {
      k0 = subpathStart1 + 1;
    } else {
      k0 = j1 + 1;
    }
    for (k1 = k0;
	 !(pathIn->flags[k1] & splashPathLast) &&
	   k1 + 1 < pathIn->length &&
	   pathIn->pts[k1+1].x == pathIn->pts[k1].x &&
	   pathIn->pts[k1+1].y == pathIn->pts[k1].y;
	 ++k1) ;

    // compute the deltas for segment (i1, j0)
    d = (SplashCoord)1 / splashDist(pathIn->pts[i1].x, pathIn->pts[i1].y,
				    pathIn->pts[j0].x, pathIn->pts[j0].y);
    dx = d * (pathIn->pts[j0].x - pathIn->pts[i1].x);
    dy = d * (pathIn->pts[j0].y - pathIn->pts[i1].y);
    wdx = (SplashCoord)0.5 * w * dx;
    wdy = (SplashCoord)0.5 * w * dy;

    // draw the start cap
    pathOut->moveTo(pathIn->pts[i0].x - wdy, pathIn->pts[i0].y + wdx);
    if (i0 == subpathStart0) {
      firstPt = pathOut->length - 1;
    }
    if (first && !closed) {
      switch (state->lineCap) {
      case splashLineCapButt:
	pathOut->lineTo(pathIn->pts[i0].x + wdy, pathIn->pts[i0].y - wdx);
	break;
      case splashLineCapRound:
	pathOut->curveTo(pathIn->pts[i0].x - wdy - bezierCircle * wdx,
			 pathIn->pts[i0].y + wdx - bezierCircle * wdy,
			 pathIn->pts[i0].x - wdx - bezierCircle * wdy,
			 pathIn->pts[i0].y - wdy + bezierCircle * wdx,
			 pathIn->pts[i0].x - wdx,
			 pathIn->pts[i0].y - wdy);
	pathOut->curveTo(pathIn->pts[i0].x - wdx + bezierCircle * wdy,
			 pathIn->pts[i0].y - wdy - bezierCircle * wdx,
			 pathIn->pts[i0].x + wdy - bezierCircle * wdx,
			 pathIn->pts[i0].y - wdx - bezierCircle * wdy,
			 pathIn->pts[i0].x + wdy,
			 pathIn->pts[i0].y - wdx);
	break;
      case splashLineCapProjecting:
	pathOut->lineTo(pathIn->pts[i0].x - wdx - wdy,
			pathIn->pts[i0].y + wdx - wdy);
	pathOut->lineTo(pathIn->pts[i0].x - wdx + wdy,
			pathIn->pts[i0].y - wdx - wdy);
	pathOut->lineTo(pathIn->pts[i0].x + wdy,
			pathIn->pts[i0].y - wdx);
	break;
      }
    } else {
      pathOut->lineTo(pathIn->pts[i0].x + wdy, pathIn->pts[i0].y - wdx);
    }

    // draw the left side of the segment rectangle
    left2 = pathOut->length - 1;
    pathOut->lineTo(pathIn->pts[j0].x + wdy, pathIn->pts[j0].y - wdx);

    // draw the end cap
    if (last && !closed) {
      switch (state->lineCap) {
      case splashLineCapButt:
	pathOut->lineTo(pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
	break;
      case splashLineCapRound:
	pathOut->curveTo(pathIn->pts[j0].x + wdy + bezierCircle * wdx,
			 pathIn->pts[j0].y - wdx + bezierCircle * wdy,
			 pathIn->pts[j0].x + wdx + bezierCircle * wdy,
			 pathIn->pts[j0].y + wdy - bezierCircle * wdx,
			 pathIn->pts[j0].x + wdx,
			 pathIn->pts[j0].y + wdy);
	pathOut->curveTo(pathIn->pts[j0].x + wdx - bezierCircle * wdy,
			 pathIn->pts[j0].y + wdy + bezierCircle * wdx,
			 pathIn->pts[j0].x - wdy + bezierCircle * wdx,
			 pathIn->pts[j0].y + wdx + bezierCircle * wdy,
			 pathIn->pts[j0].x - wdy,
			 pathIn->pts[j0].y + wdx);
	break;
      case splashLineCapProjecting:
	pathOut->lineTo(pathIn->pts[j0].x + wdy + wdx,
			pathIn->pts[j0].y - wdx + wdy);
	pathOut->lineTo(pathIn->pts[j0].x - wdy + wdx,
			pathIn->pts[j0].y + wdx + wdy);
	pathOut->lineTo(pathIn->pts[j0].x - wdy,
			pathIn->pts[j0].y + wdx);
	break;
      }
    } else {
      pathOut->lineTo(pathIn->pts[j0].x - wdy, pathIn->pts[j0].y + wdx);
    }

    // draw the right side of the segment rectangle
    // (NB: if stroke adjustment is enabled, the closepath operation MUST
    // add a segment because this segment is used for a hint)
    right2 = pathOut->length - 1;
    pathOut->close(state->strokeAdjust);

    // draw the join
    join2 = pathOut->length;
    if (!last || closed) {

      // compute the deltas for segment (j1, k0)
      d = (SplashCoord)1 / splashDist(pathIn->pts[j1].x, pathIn->pts[j1].y,
				      pathIn->pts[k0].x, pathIn->pts[k0].y);
      dxNext = d * (pathIn->pts[k0].x - pathIn->pts[j1].x);
      dyNext = d * (pathIn->pts[k0].y - pathIn->pts[j1].y);
      wdxNext = (SplashCoord)0.5 * w * dxNext;
      wdyNext = (SplashCoord)0.5 * w * dyNext;

      // compute the join parameters
      crossprod = dx * dyNext - dy * dxNext;
      dotprod = -(dx * dxNext + dy * dyNext);
      if (dotprod > 0.99999) {
	// avoid a divide-by-zero -- set miter to something arbitrary
	// such that sqrt(miter) will exceed miterLimit (and m is never
	// used in that situation)
	miter = (state->miterLimit + 1) * (state->miterLimit + 1);
	m = 0;
      } else {
	miter = (SplashCoord)2 / ((SplashCoord)1 - dotprod);
	if (miter < 1) {
	  // this can happen because of floating point inaccuracies
	  miter = 1;
	}
	m = splashSqrt(miter - 1);
      }

      // round join
      if (state->lineJoin == splashLineJoinRound) {
	pathOut->moveTo(pathIn->pts[j0].x + (SplashCoord)0.5 * w,
			pathIn->pts[j0].y);
	pathOut->curveTo(pathIn->pts[j0].x + (SplashCoord)0.5 * w,
			 pathIn->pts[j0].y + bezierCircle2 * w,
			 pathIn->pts[j0].x + bezierCircle2 * w,
			 pathIn->pts[j0].y + (SplashCoord)0.5 * w,
			 pathIn->pts[j0].x,
			 pathIn->pts[j0].y + (SplashCoord)0.5 * w);
	pathOut->curveTo(pathIn->pts[j0].x - bezierCircle2 * w,
			 pathIn->pts[j0].y + (SplashCoord)0.5 * w,
			 pathIn->pts[j0].x - (SplashCoord)0.5 * w,
			 pathIn->pts[j0].y + bezierCircle2 * w,
			 pathIn->pts[j0].x - (SplashCoord)0.5 * w,
			 pathIn->pts[j0].y);
	pathOut->curveTo(pathIn->pts[j0].x - (SplashCoord)0.5 * w,
			 pathIn->pts[j0].y - bezierCircle2 * w,
			 pathIn->pts[j0].x - bezierCircle2 * w,
			 pathIn->pts[j0].y - (SplashCoord)0.5 * w,
			 pathIn->pts[j0].x,
			 pathIn->pts[j0].y - (SplashCoord)0.5 * w);
	pathOut->curveTo(pathIn->pts[j0].x + bezierCircle2 * w,
			 pathIn->pts[j0].y - (SplashCoord)0.5 * w,
			 pathIn->pts[j0].x + (SplashCoord)0.5 * w,
			 pathIn->pts[j0].y - bezierCircle2 * w,
			 pathIn->pts[j0].x + (SplashCoord)0.5 * w,
			 pathIn->pts[j0].y);

      } else {
	pathOut->moveTo(pathIn->pts[j0].x, pathIn->pts[j0].y);

	// angle < 180
	if (crossprod < 0) {
	  pathOut->lineTo(pathIn->pts[j0].x - wdyNext,
			  pathIn->pts[j0].y + wdxNext);
	  // miter join inside limit
	  if (state->lineJoin == splashLineJoinMiter &&
	      splashSqrt(miter) <= state->miterLimit) {
	    pathOut->lineTo(pathIn->pts[j0].x - wdy + wdx * m,
			    pathIn->pts[j0].y + wdx + wdy * m);
	    pathOut->lineTo(pathIn->pts[j0].x - wdy,
			    pathIn->pts[j0].y + wdx);
	  // bevel join or miter join outside limit
	  } else {
	    pathOut->lineTo(pathIn->pts[j0].x - wdy,
			    pathIn->pts[j0].y + wdx);
	  }

	// angle >= 180
	} else {
	  pathOut->lineTo(pathIn->pts[j0].x + wdy,
			  pathIn->pts[j0].y - wdx);
	  // miter join inside limit
	  if (state->lineJoin == splashLineJoinMiter &&
	      splashSqrt(miter) <= state->miterLimit) {
	    pathOut->lineTo(pathIn->pts[j0].x + wdy + wdx * m,
			    pathIn->pts[j0].y - wdx + wdy * m);
	    pathOut->lineTo(pathIn->pts[j0].x + wdyNext,
			    pathIn->pts[j0].y - wdxNext);
	  // bevel join or miter join outside limit
	  } else {
	    pathOut->lineTo(pathIn->pts[j0].x + wdyNext,
			    pathIn->pts[j0].y - wdxNext);
	  }
	}
      }

      pathOut->close();
    }

    // add stroke adjustment hints
    if (state->strokeAdjust) {
      if (seg >= 1) {
	if (seg >= 2) {
	  pathOut->addStrokeAdjustHint(left1, right1, left0 + 1, right0);
	  pathOut->addStrokeAdjustHint(left1, right1, join0, left2);
	} else {
	  pathOut->addStrokeAdjustHint(left1, right1, firstPt, left2);
	}
	pathOut->addStrokeAdjustHint(left1, right1, right2 + 1, right2 + 1);
      }
      left0 = left1;
      left1 = left2;
      right0 = right1;
      right1 = right2;
      join0 = join1;
      join1 = join2;
      if (seg == 0) {
	leftFirst = left2;
	rightFirst = right2;
      }
      if (last) {
	if (seg >= 2) {
	  pathOut->addStrokeAdjustHint(left1, right1, left0 + 1, right0);
	  pathOut->addStrokeAdjustHint(left1, right1,
				       join0, pathOut->length - 1);
	} else {
	  pathOut->addStrokeAdjustHint(left1, right1,
				       firstPt, pathOut->length - 1);
	}
	if (closed) {
	  pathOut->addStrokeAdjustHint(left1, right1, firstPt, leftFirst);
	  pathOut->addStrokeAdjustHint(left1, right1,
				       rightFirst + 1, rightFirst + 1);
	  pathOut->addStrokeAdjustHint(leftFirst, rightFirst,
				       left1 + 1, right1);
	  pathOut->addStrokeAdjustHint(leftFirst, rightFirst,
				       join1, pathOut->length - 1);
	}
      }
    }

    i0 = j0;
    i1 = j1;
    ++seg;
  }

  if (pathIn != path) {
    delete pathIn;
  }

  return pathOut;
}

void Splash::dumpPath(SplashPath *path) {
  int i;

  for (i = 0; i < path->length; ++i) {
    printf("  %3d: x=%8.2f y=%8.2f%s%s%s%s\n",
	   i, (double)path->pts[i].x, (double)path->pts[i].y,
	   (path->flags[i] & splashPathFirst) ? " first" : "",
	   (path->flags[i] & splashPathLast) ? " last" : "",
	   (path->flags[i] & splashPathClosed) ? " closed" : "",
	   (path->flags[i] & splashPathCurve) ? " curve" : "");
  }
}

void Splash::dumpXPath(SplashXPath *path) {
  int i;

  for (i = 0; i < path->length; ++i) {
    printf("  %4d: x0=%8.2f y0=%8.2f x1=%8.2f y1=%8.2f %s%s%s%s%s%s%s\n",
	   i, (double)path->segs[i].x0, (double)path->segs[i].y0,
	   (double)path->segs[i].x1, (double)path->segs[i].y1,
	   (path->segs[i].flags	& splashXPathFirst) ? "F" : " ",
	   (path->segs[i].flags	& splashXPathLast) ? "L" : " ",
	   (path->segs[i].flags	& splashXPathEnd0) ? "0" : " ",
	   (path->segs[i].flags	& splashXPathEnd1) ? "1" : " ",
	   (path->segs[i].flags	& splashXPathHoriz) ? "H" : " ",
	   (path->segs[i].flags	& splashXPathVert) ? "V" : " ",
	   (path->segs[i].flags	& splashXPathFlip) ? "P" : " ");
  }
}
