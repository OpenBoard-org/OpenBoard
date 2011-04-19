//========================================================================
//
// SplashMath.h
//
//========================================================================

#ifndef SPLASHMATH_H
#define SPLASHMATH_H

#include <aconf.h>

#if USE_FIXEDPONT
#include "FixedPoint.h"
#else
#include <math.h>
#endif
#include "SplashTypes.h"

static inline SplashCoord splashAbs(SplashCoord x) {
#if USE_FIXEDPOINT
  return FixedPoint::abs(x);
#else
  return fabs(x);
#endif
}

static inline int splashFloor(SplashCoord x) {
#if USE_FIXEDPOINT
  return FixedPoint::floor(x);
#else
#if WIN32 && _M_IX86
  // floor() and (int)() are implemented separately, which results
  // in changing the FPCW multiple times - so we optimize it with
  // some inline assembly
  Gushort oldCW, newCW;
  int result;

  __asm fld QWORD PTR x
  __asm fnstcw WORD PTR oldCW
  __asm mov ax, WORD PTR oldCW
  __asm and ax, 0xf3ff
  __asm or ax, 0x0400
  __asm mov WORD PTR newCW, ax     // round down
  __asm fldcw WORD PTR newCW
  __asm fistp DWORD PTR result
  __asm fldcw WORD PTR oldCW
  return result;
#elif __GNUC__ && __i386__
  // floor() and (int)() are implemented separately, which results
  // in changing the FPCW multiple times - so we optimize it with
  // some inline assembly
  Gushort oldCW, newCW, t;
  int result;

  __asm__ volatile("fldl   %4\n"
		   "fnstcw %0\n"
		   "movw   %0, %3\n"
		   "andw   $0xf3ff, %3\n"
		   "orw    $0x0400, %3\n"
		   "movw   %3, %1\n"       // round down
		   "fldcw  %1\n"
		   "fistpl %2\n"
		   "fldcw  %0\n"
		   : "=m" (oldCW), "=m" (newCW), "=m" (result), "=r" (t)
		   : "m" (x));
  return result;
#else
  return (int)floor(x);
#endif
#endif
}

static inline int splashCeil(SplashCoord x) {
#if USE_FIXEDPOINT
  return FixedPoint::ceil(x);
#else
#if WIN32 && _M_IX86
  // ceil() and (int)() are implemented separately, which results
  // in changing the FPCW multiple times - so we optimize it with
  // some inline assembly
  Gushort oldCW, newCW;
  int result;

  __asm fld QWORD PTR x
  __asm fnstcw WORD PTR oldCW
  __asm mov ax, WORD PTR oldCW
  __asm and ax, 0xf3ff
  __asm or ax, 0x0800
  __asm mov WORD PTR newCW, ax     // round up
  __asm fldcw WORD PTR newCW
  __asm fistp DWORD PTR result
  __asm fldcw WORD PTR oldCW
  return result;
#elif __GNUC__ && __i386__
  // ceil() and (int)() are implemented separately, which results
  // in changing the FPCW multiple times - so we optimize it with
  // some inline assembly
  Gushort oldCW, newCW, t;
  int result;

  __asm__ volatile("fldl   %4\n"
		   "fnstcw %0\n"
		   "movw   %0, %3\n"
		   "andw   $0xf3ff, %3\n"
		   "orw    $0x0800, %3\n"
		   "movw   %3, %1\n"       // round up
		   "fldcw  %1\n"
		   "fistpl %2\n"
		   "fldcw  %0\n"
		   : "=m" (oldCW), "=m" (newCW), "=m" (result), "=r" (t)
		   : "m" (x));
  return result;
#else
  return (int)ceil(x);
#endif
#endif
}

static inline int splashRound(SplashCoord x) {
#if USE_FIXEDPOINT
  return FixedPoint::round(x);
#else
#if WIN32 && _M_IX86
  // this could use round-to-nearest mode and avoid the "+0.5",
  // but that produces slightly different results (because i+0.5
  // sometimes rounds up and sometimes down using the even rule)
  Gushort oldCW, newCW;
  int result;

  x += 0.5;
  __asm fld QWORD PTR x
  __asm fnstcw WORD PTR oldCW
  __asm mov ax, WORD PTR oldCW
  __asm and ax, 0xf3ff
  __asm or ax, 0x0400
  __asm mov WORD PTR newCW, ax     // round down
  __asm fldcw WORD PTR newCW
  __asm fistp DWORD PTR result
  __asm fldcw WORD PTR oldCW
  return result;
#elif __GNUC__ && __i386__
  // this could use round-to-nearest mode and avoid the "+0.5",
  // but that produces slightly different results (because i+0.5
  // sometimes rounds up and sometimes down using the even rule)
  Gushort oldCW, newCW, t;
  int result;

  x += 0.5;
  __asm__ volatile("fldl   %4\n"
		   "fnstcw %0\n"
		   "movw   %0, %3\n"
		   "andw   $0xf3ff, %3\n"
		   "orw    $0x0400, %3\n"
		   "movw   %3, %1\n"       // round down
		   "fldcw  %1\n"
		   "fistpl %2\n"
		   "fldcw  %0\n"
		   : "=m" (oldCW), "=m" (newCW), "=m" (result), "=r" (t)
		   : "m" (x));
  return result;
#else
  return (int)floor(x + 0.5);
#endif
#endif
}

static inline SplashCoord splashSqrt(SplashCoord x) {
#if USE_FIXEDPOINT
  return FixedPoint::sqrt(x);
#else
  return sqrt(x);
#endif
}

static inline SplashCoord splashPow(SplashCoord x, SplashCoord y) {
#if USE_FIXEDPOINT
  return FixedPoint::pow(x, y);
#else
  return pow(x, y);
#endif
}

static inline SplashCoord splashDist(SplashCoord x0, SplashCoord y0,
				     SplashCoord x1, SplashCoord y1) {
  SplashCoord dx, dy;
  dx = x1 - x0;
  dy = y1 - y0;
#if USE_FIXEDPOINT
  // this handles the situation where dx*dx or dy*dy is too large to
  // fit in the 16.16 fixed point format
  SplashCoord dxa, dya;
  dxa = splashAbs(dx);
  dya = splashAbs(dy);
  if (dxa == 0 && dya == 0) {
    return 0;
  } else if (dxa > dya) {
    return dxa * FixedPoint::sqrt(dya / dxa + 1);
  } else {
    return dya * FixedPoint::sqrt(dxa / dya + 1);
  }
#else
  return sqrt(dx * dx + dy * dy);
#endif
}

#endif
