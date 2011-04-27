//========================================================================
//
// SplashFontFile.cc
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#ifndef WIN32
#  include <unistd.h>
#endif
#include "GString.h"
#include "SplashFontFile.h"
#include "SplashFontFileID.h"

#ifdef VMS
#if (__VMS_VER < 70000000)
extern "C" int unlink(char *filename);
#endif
#endif

//------------------------------------------------------------------------
// SplashFontFile
//------------------------------------------------------------------------

SplashFontFile::SplashFontFile(SplashFontFileID *idA, char *fileNameA,
			       GBool deleteFileA) {
  id = idA;
  fileName = new GString(fileNameA);
  deleteFile = deleteFileA;
  refCnt = 0;
}

SplashFontFile::~SplashFontFile() {
  if (deleteFile) {
    unlink(fileName->getCString());
  }
  delete fileName;
  delete id;
}

void SplashFontFile::incRefCnt() {
  ++refCnt;
}

void SplashFontFile::decRefCnt() {
  if (!--refCnt) {
    delete this;
  }
}
