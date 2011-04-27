//========================================================================
//
// SplashT1FontFile.cc
//
//========================================================================

#include <aconf.h>

#if HAVE_T1LIB_H

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <string.h>
#include <t1lib.h>
#include "gmem.h"
#include "SplashT1FontEngine.h"
#include "SplashT1Font.h"
#include "SplashT1FontFile.h"

//------------------------------------------------------------------------
// SplashT1FontFile
//------------------------------------------------------------------------

SplashFontFile *SplashT1FontFile::loadType1Font(SplashT1FontEngine *engineA,
						SplashFontFileID *idA,
						char *fileNameA,
						GBool deleteFileA,
						char **encA) {
  int t1libIDA;
  char **encTmp;
  char *encStrTmp;
  int encStrSize;
  char *encPtr;
  int i;

  // load the font file
  if ((t1libIDA = T1_AddFont(fileNameA)) < 0) {
    return NULL;
  }
  T1_LoadFont(t1libIDA);

  // reencode it
  encStrSize = 0;
  for (i = 0; i < 256; ++i) {
    if (encA[i]) {
      encStrSize += strlen(encA[i]) + 1;
    }
  }
  encTmp = (char **)gmallocn(257, sizeof(char *));
  encStrTmp = (char *)gmallocn(encStrSize, sizeof(char));
  encPtr = encStrTmp;
  for (i = 0; i < 256; ++i) {
    if (encA[i]) {
      strcpy(encPtr, encA[i]);
      encTmp[i] = encPtr;
      encPtr += strlen(encPtr) + 1;
    } else {
      encTmp[i] = ".notdef";
    }
  }
  encTmp[256] = "custom";
  T1_ReencodeFont(t1libIDA, encTmp);

  return new SplashT1FontFile(engineA, idA, fileNameA, deleteFileA,
			      t1libIDA, encTmp, encStrTmp);
}

SplashT1FontFile::SplashT1FontFile(SplashT1FontEngine *engineA,
				   SplashFontFileID *idA,
				   char *fileNameA, GBool deleteFileA,
				   int t1libIDA, char **encA, char *encStrA):
  SplashFontFile(idA, fileNameA, deleteFileA)
{
  engine = engineA;
  t1libID = t1libIDA;
  enc = encA;
  encStr = encStrA;
}

SplashT1FontFile::~SplashT1FontFile() {
  gfree(encStr);
  gfree(enc);
  T1_DeleteFont(t1libID);
}

SplashFont *SplashT1FontFile::makeFont(SplashCoord *mat,
				       SplashCoord *textMat) {
  SplashFont *font;

  font = new SplashT1Font(this, mat, textMat);
  font->initCache();
  return font;
}

#endif // HAVE_T1LIB_H
