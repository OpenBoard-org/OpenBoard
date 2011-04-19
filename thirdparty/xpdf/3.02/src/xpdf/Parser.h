//========================================================================
//
// Parser.h
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef PARSER_H
#define PARSER_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include "Lexer.h"

//------------------------------------------------------------------------
// Parser
//------------------------------------------------------------------------

class Parser {
public:

  // Constructor.
  Parser(XRef *xrefA, Lexer *lexerA, GBool allowStreamsA);

  // Destructor.
  ~Parser();

  // Get the next object from the input stream.  If <simpleOnly> is
  // true, do not parse compound objects (arrays, dictionaries, or
  // streams).
  Object *getObj(Object *obj, GBool simpleOnly = gFalse,
		 Guchar *fileKey = NULL,
		 CryptAlgorithm encAlgorithm = cryptRC4, int keyLength = 0,
		 int objNum = 0, int objGen = 0, int recursion = 0);

  // Get stream.
  Stream *getStream() { return lexer->getStream(); }

  // Get current position in file.
  int getPos() { return lexer->getPos(); }

private:

  XRef *xref;			// the xref table for this PDF file
  Lexer *lexer;			// input stream
  GBool allowStreams;		// parse stream objects?
  Object buf1, buf2;		// next two tokens
  int inlineImg;		// set when inline image data is encountered

  Stream *makeStream(Object *dict, Guchar *fileKey,
		     CryptAlgorithm encAlgorithm, int keyLength,
		     int objNum, int objGen, int recursion);
  void shift();
};

#endif

