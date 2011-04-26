//========================================================================
//
// SplashFontEngine.h
//
//========================================================================

#ifndef SPLASHFONTENGINE_H
#define SPLASHFONTENGINE_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "gtypes.h"

class SplashT1FontEngine;
class SplashFTFontEngine;
class SplashDTFontEngine;
class SplashDT4FontEngine;
class SplashFontFile;
class SplashFontFileID;
class SplashFont;

//------------------------------------------------------------------------

#define splashFontCacheSize 16

//------------------------------------------------------------------------
// SplashFontEngine
//------------------------------------------------------------------------

class SplashFontEngine {
public:

  // Create a font engine.
  SplashFontEngine(
#if HAVE_T1LIB_H
		   GBool enableT1lib,
#endif
#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H
		   GBool enableFreeType,
#endif
		   GBool aa);

  ~SplashFontEngine();

  // Get a font file from the cache.  Returns NULL if there is no
  // matching entry in the cache.
  SplashFontFile *getFontFile(SplashFontFileID *id);

  // Load fonts - these create new SplashFontFile objects.
  SplashFontFile *loadType1Font(SplashFontFileID *idA, char *fileName,
				GBool deleteFile, char **enc);
  SplashFontFile *loadType1CFont(SplashFontFileID *idA, char *fileName,
				 GBool deleteFile, char **enc);
  SplashFontFile *loadOpenTypeT1CFont(SplashFontFileID *idA, char *fileName,
				      GBool deleteFile, char **enc);
  SplashFontFile *loadCIDFont(SplashFontFileID *idA, char *fileName,
			      GBool deleteFile);
  SplashFontFile *loadOpenTypeCFFFont(SplashFontFileID *idA, char *fileName,
				      GBool deleteFile);
  SplashFontFile *loadTrueTypeFont(SplashFontFileID *idA, char *fileName,
				   GBool deleteFile,
				   Gushort *codeToGID, int codeToGIDLen);

  // Get a font - this does a cache lookup first, and if not found,
  // creates a new SplashFont object and adds it to the cache.  The
  // matrix, mat = textMat * ctm:
  //    [ mat[0] mat[1] ]
  //    [ mat[2] mat[3] ]
  // specifies the font transform in PostScript style:
  //    [x' y'] = [x y] * mat
  // Note that the Splash y axis points downward.
  SplashFont *getFont(SplashFontFile *fontFile,
		      SplashCoord *textMat, SplashCoord *ctm);

private:

  SplashFont *fontCache[splashFontCacheSize];

#if HAVE_T1LIB_H
  SplashT1FontEngine *t1Engine;
#endif
#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H
  SplashFTFontEngine *ftEngine;
#endif
};

#endif
