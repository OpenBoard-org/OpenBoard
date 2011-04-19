//========================================================================
//
// SplashFontFile.h
//
//========================================================================

#ifndef SPLASHFONTFILE_H
#define SPLASHFONTFILE_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "gtypes.h"
#include "SplashTypes.h"

class GString;
class SplashFontEngine;
class SplashFont;
class SplashFontFileID;

//------------------------------------------------------------------------
// SplashFontFile
//------------------------------------------------------------------------

class SplashFontFile {
public:

  virtual ~SplashFontFile();

  // Create a new SplashFont, i.e., a scaled instance of this font
  // file.
  virtual SplashFont *makeFont(SplashCoord *mat, SplashCoord *textMat) = 0;

  // Get the font file ID.
  SplashFontFileID *getID() { return id; }

  // Increment the reference count.
  void incRefCnt();

  // Decrement the reference count.  If the new value is zero, delete
  // the SplashFontFile object.
  void decRefCnt();

protected:

  SplashFontFile(SplashFontFileID *idA, char *fileNameA,
		 GBool deleteFileA);

  SplashFontFileID *id;
  GString *fileName;
  GBool deleteFile;
  int refCnt;

  friend class SplashFontEngine;
};

#endif
