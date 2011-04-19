//========================================================================
//
// Error.cc
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include <stdio.h>
#include <stddef.h>
#include <stdarg.h>
#include "GString.h"
#include "GlobalParams.h"
#include "Error.h"

static char *errorCategoryNames[] = {
  "Syntax Warning",
  "Syntax Error",
  "Config Error",
  "Command Line Error",
  "I/O Error",
  "Permission Error",
  "Unimplemented Feature",
  "Internal Error"
};

static void (*errorCbk)(void *data, ErrorCategory category,
			int pos, char *msg) = NULL;
static void *errorCbkData = NULL;

void setErrorCallback(void (*cbk)(void *data, ErrorCategory category,
				  int pos, char *msg),
		      void *data) {
  errorCbk = cbk;
  errorCbkData = data;
}

void CDECL error(ErrorCategory category, int pos, char *msg, ...) {
  va_list args;
  GString *s;

  // NB: this can be called before the globalParams object is created
  if (!errorCbk && globalParams && globalParams->getErrQuiet()) {
    return;
  }
  va_start(args, msg);
  s = GString::formatv(msg, args);
  va_end(args);
  if (errorCbk) {
    (*errorCbk)(errorCbkData, category, pos, s->getCString());
  } else {
    if (pos >= 0) {
      fprintf(stderr, "%s (%d): %s\n",
	      errorCategoryNames[category], pos, s->getCString());
    } else {
      fprintf(stderr, "%s: %s\n",
	      errorCategoryNames[category], s->getCString());
    }
    fflush(stderr);
  }
  delete s;
}
