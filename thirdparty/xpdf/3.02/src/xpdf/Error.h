//========================================================================
//
// Error.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef ERROR_H
#define ERROR_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include "config.h"

enum ErrorCategory {
  errSyntaxWarning,	// PDF syntax error which can be worked around;
			//   output will probably be correct
  errSyntaxError,	// PDF syntax error which can be worked around;
			//   output will probably be incorrect
  errConfig,		// error in Xpdf config info (xpdfrc file, etc.)
  errCommandLine,	// error in user-supplied parameters, action not
			//   allowed, etc. (only used by command-line tools)
  errIO,		// error in file I/O
  errNotAllowed,	// action not allowed by PDF permission bits
  errUnimplemented,	// unimplemented PDF feature - display will be
			//   incorrect
  errInternal		// internal error - malfunction within the Xpdf code
};

extern void setErrorCallback(void (*cbk)(void *data, ErrorCategory category,
					 int pos, char *msg),
			     void *data);

extern void CDECL error(ErrorCategory category, int pos, char *msg, ...);

#endif
