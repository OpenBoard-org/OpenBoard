//========================================================================
//
// SplashBitmap.cc
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <limits.h>
#include "gmem.h"
#include "SplashErrorCodes.h"
#include "SplashBitmap.h"

//------------------------------------------------------------------------
// SplashBitmap
//------------------------------------------------------------------------

SplashBitmap::SplashBitmap(int widthA, int heightA, int rowPad,
			   SplashColorMode modeA, GBool alphaA,
			   GBool topDown) {
  width = widthA;
  height = heightA;
  mode = modeA;
  switch (mode) {
  case splashModeMono1:
    if (width > 0) {
      rowSize = (width + 7) >> 3;
    } else {
      rowSize = -1;
    }
    break;
  case splashModeMono8:
    if (width > 0) {
      rowSize = width;
    } else {
      rowSize = -1;
    }
    break;
  case splashModeRGB8:
  case splashModeBGR8:
    if (width > 0 && width <= INT_MAX / 3) {
      rowSize = width * 3;
    } else {
      rowSize = -1;
    }
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    if (width > 0 && width <= INT_MAX / 4) {
      rowSize = width * 4;
    } else {
      rowSize = -1;
    }
    break;
#endif
  }
  if (rowSize > 0) {
    rowSize += rowPad - 1;
    rowSize -= rowSize % rowPad;
  }
  data = (SplashColorPtr)gmallocn(height, rowSize);
  if (!topDown) {
    data += (height - 1) * rowSize;
    rowSize = -rowSize;
  }
  if (alphaA) {
    alpha = (Guchar *)gmallocn(width, height);
  } else {
    alpha = NULL;
  }
}

SplashBitmap::~SplashBitmap() {
  if (data) {
    if (rowSize < 0) {
      gfree(data + (height - 1) * rowSize);
    } else {
      gfree(data);
    }
  }
  gfree(alpha);
}

SplashError SplashBitmap::writePNMFile(char *fileName) {
  FILE *f;
  SplashError err;

  if (!(f = fopen(fileName, "wb"))) {
    return splashErrOpenFile;
  }
  err = writePNMFile(f);
  fclose(f);
  return err;
}

SplashError SplashBitmap::writePNMFile(FILE *f) {
  SplashColorPtr row, p;
  int x, y;

  switch (mode) {

  case splashModeMono1:
    fprintf(f, "P4\n%d %d\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; x += 8) {
	fputc(*p ^ 0xff, f);
	++p;
      }
      row += rowSize;
    }
    break;

  case splashModeMono8:
    fprintf(f, "P5\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      fwrite(row, 1, width, f);
      row += rowSize;
    }
    break;

  case splashModeRGB8:
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      fwrite(row, 1, 3 * width, f);
      row += rowSize;
    }
    break;

  case splashModeBGR8:
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    row = data;
    for (y = 0; y < height; ++y) {
      p = row;
      for (x = 0; x < width; ++x) {
	fputc(splashBGR8R(p), f);
	fputc(splashBGR8G(p), f);
	fputc(splashBGR8B(p), f);
	p += 3;
      }
      row += rowSize;
    }
    break;

#if SPLASH_CMYK
  case splashModeCMYK8:
    // PNM doesn't support CMYK
    break;
#endif

  }

  return splashOk;
}

SplashError SplashBitmap::writeAlphaPGMFile(char *fileName) {
  FILE *f;

  if (!alpha) {
    return splashErrModeMismatch;
  }
  if (!(f = fopen(fileName, "wb"))) {
    return splashErrOpenFile;
  }
  fprintf(f, "P5\n%d %d\n255\n", width, height);
  fwrite(alpha, 1, width * height, f);
  fclose(f);
  return splashOk;
}


void SplashBitmap::getPixel(int x, int y, SplashColorPtr pixel) {
  SplashColorPtr p;

  if (y < 0 || y >= height || x < 0 || x >= width) {
    return;
  }
  switch (mode) {
  case splashModeMono1:
    p = &data[y * rowSize + (x >> 3)];
    pixel[0] = (p[0] & (0x80 >> (x & 7))) ? 0xff : 0x00;
    break;
  case splashModeMono8:
    p = &data[y * rowSize + x];
    pixel[0] = p[0];
    break;
  case splashModeRGB8:
    p = &data[y * rowSize + 3 * x];
    pixel[0] = p[0];
    pixel[1] = p[1];
    pixel[2] = p[2];
    break;
  case splashModeBGR8:
    p = &data[y * rowSize + 3 * x];
    pixel[0] = p[2];
    pixel[1] = p[1];
    pixel[2] = p[0];
    break;
#if SPLASH_CMYK
  case splashModeCMYK8:
    p = &data[y * rowSize + 4 * x];
    pixel[0] = p[0];
    pixel[1] = p[1];
    pixel[2] = p[2];
    pixel[3] = p[3];
    break;
#endif
  }
}

Guchar SplashBitmap::getAlpha(int x, int y) {
  return alpha[y * width + x];
}

SplashColorPtr SplashBitmap::takeData() {
  SplashColorPtr data2;

  data2 = data;
  data = NULL;
  return data2;
}
