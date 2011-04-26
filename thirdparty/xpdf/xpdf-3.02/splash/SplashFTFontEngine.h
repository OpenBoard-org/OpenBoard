//========================================================================
//
// SplashFTFontEngine.h
//
//========================================================================

#ifndef SPLASHFTFONTENGINE_H
#define SPLASHFTFONTENGINE_H

#include <aconf.h>

#if HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include "gtypes.h"

class SplashFontFile;
class SplashFontFileID;

//------------------------------------------------------------------------
// SplashFTFontEngine
//------------------------------------------------------------------------

class SplashFTFontEngine {
public:

  static SplashFTFontEngine *init(GBool aaA);

  ~SplashFTFontEngine();

  // Load fonts.
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

private:

  SplashFTFontEngine(GBool aaA, FT_Library libA);

  GBool aa;
  FT_Library lib;
  GBool useCIDs;

  friend class SplashFTFontFile;
  friend class SplashFTFont;
};

#endif // HAVE_FREETYPE_FREETYPE_H || HAVE_FREETYPE_H

#endif
