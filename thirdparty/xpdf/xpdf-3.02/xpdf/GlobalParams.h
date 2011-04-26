//========================================================================
//
// GlobalParams.h
//
// Copyright 2001-2003 Glyph & Cog, LLC
//
//========================================================================

#ifndef GLOBALPARAMS_H
#define GLOBALPARAMS_H

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma interface
#endif

#include <stdio.h>
#include "gtypes.h"
#include "CharTypes.h"

#if MULTITHREADED
#include "GMutex.h"
#endif

class GString;
class GList;
class GHash;
class NameToCharCode;
class CharCodeToUnicode;
class CharCodeToUnicodeCache;
class UnicodeMap;
class UnicodeMapCache;
class CMap;
class CMapCache;
struct XpdfSecurityHandler;
class GlobalParams;
#ifdef WIN32
class WinFontList;
#endif

//------------------------------------------------------------------------

// The global parameters object.
extern GlobalParams *globalParams;

//------------------------------------------------------------------------

enum DisplayFontParamKind {
  displayFontT1,
  displayFontTT
};

struct DisplayFontParamT1 {
  GString *fileName;
};

struct DisplayFontParamTT {
  GString *fileName;
};

class DisplayFontParam {
public:

  GString *name;		// font name for 8-bit fonts and named
				//   CID fonts; collection name for
				//   generic CID fonts
  DisplayFontParamKind kind;
  union {
    DisplayFontParamT1 t1;
    DisplayFontParamTT tt;
  };

  DisplayFontParam(GString *nameA, DisplayFontParamKind kindA);
  virtual ~DisplayFontParam();
};

//------------------------------------------------------------------------

class PSFontParam {
public:

  GString *pdfFontName;		// PDF font name for 8-bit fonts and
				//   named 16-bit fonts; char collection
				//   name for generic 16-bit fonts
  int wMode;			// writing mode (0=horiz, 1=vert) for
				//   16-bit fonts
  GString *psFontName;		// PostScript font name
  GString *encoding;		// encoding, for 16-bit fonts only

  PSFontParam(GString *pdfFontNameA, int wModeA,
	      GString *psFontNameA, GString *encodingA);
  ~PSFontParam();
};

//------------------------------------------------------------------------

enum PSLevel {
  psLevel1,
  psLevel1Sep,
  psLevel2,
  psLevel2Sep,
  psLevel3,
  psLevel3Sep
};

//------------------------------------------------------------------------

enum EndOfLineKind {
  eolUnix,			// LF
  eolDOS,			// CR+LF
  eolMac			// CR
};

//------------------------------------------------------------------------

enum ScreenType {
  screenUnset,
  screenDispersed,
  screenClustered,
  screenStochasticClustered
};

//------------------------------------------------------------------------

class KeyBinding {
public:

  int code;			// 0x20 .. 0xfe = ASCII,
				//   >=0x10000 = special keys, mouse buttons,
				//   etc. (xpdfKeyCode* symbols)
  int mods;			// modifiers (xpdfKeyMod* symbols, or-ed
				//   together)
  int context;			// context (xpdfKeyContext* symbols, or-ed
				//   together)
  GList *cmds;			// list of commands [GString]

  KeyBinding(int codeA, int modsA, int contextA, char *cmd0);
  KeyBinding(int codeA, int modsA, int contextA, char *cmd0, char *cmd1);
  KeyBinding(int codeA, int modsA, int contextA, GList *cmdsA);
  ~KeyBinding();
};

#define xpdfKeyCodeTab            0x1000
#define xpdfKeyCodeReturn         0x1001
#define xpdfKeyCodeEnter          0x1002
#define xpdfKeyCodeBackspace      0x1003
#define xpdfKeyCodeInsert         0x1004
#define xpdfKeyCodeDelete         0x1005
#define xpdfKeyCodeHome           0x1006
#define xpdfKeyCodeEnd            0x1007
#define xpdfKeyCodePgUp           0x1008
#define xpdfKeyCodePgDn           0x1009
#define xpdfKeyCodeLeft           0x100a
#define xpdfKeyCodeRight          0x100b
#define xpdfKeyCodeUp             0x100c
#define xpdfKeyCodeDown           0x100d
#define xpdfKeyCodeF1             0x1100
#define xpdfKeyCodeF35            0x1122
#define xpdfKeyCodeMousePress1    0x2001
#define xpdfKeyCodeMousePress2    0x2002
#define xpdfKeyCodeMousePress3    0x2003
#define xpdfKeyCodeMousePress4    0x2004
#define xpdfKeyCodeMousePress5    0x2005
#define xpdfKeyCodeMousePress6    0x2006
#define xpdfKeyCodeMousePress7    0x2007
#define xpdfKeyCodeMouseRelease1  0x2101
#define xpdfKeyCodeMouseRelease2  0x2102
#define xpdfKeyCodeMouseRelease3  0x2103
#define xpdfKeyCodeMouseRelease4  0x2104
#define xpdfKeyCodeMouseRelease5  0x2105
#define xpdfKeyCodeMouseRelease6  0x2106
#define xpdfKeyCodeMouseRelease7  0x2107
#define xpdfKeyModNone            0
#define xpdfKeyModShift           (1 << 0)
#define xpdfKeyModCtrl            (1 << 1)
#define xpdfKeyModAlt             (1 << 2)
#define xpdfKeyContextAny         0
#define xpdfKeyContextFullScreen  (1 << 0)
#define xpdfKeyContextWindow      (2 << 0)
#define xpdfKeyContextContinuous  (1 << 2)
#define xpdfKeyContextSinglePage  (2 << 2)
#define xpdfKeyContextOverLink    (1 << 4)
#define xpdfKeyContextOffLink     (2 << 4)
#define xpdfKeyContextOutline     (1 << 6)
#define xpdfKeyContextMainWin     (2 << 6)
#define xpdfKeyContextScrLockOn   (1 << 8)
#define xpdfKeyContextScrLockOff  (2 << 8)

//------------------------------------------------------------------------

class GlobalParams {
public:

  // Initialize the global parameters by attempting to read a config
  // file.
  GlobalParams(char *cfgFileName);

  ~GlobalParams();

  void setBaseDir(char *dir);
  void setupBaseFonts(char *dir);

  void parseLine(char *buf, GString *fileName, int line);

  //----- accessors

  CharCode getMacRomanCharCode(char *charName);

  GString *getBaseDir();
  Unicode mapNameToUnicode(char *charName);
  UnicodeMap *getResidentUnicodeMap(GString *encodingName);
  FILE *getUnicodeMapFile(GString *encodingName);
  FILE *findCMapFile(GString *collection, GString *cMapName);
  FILE *findToUnicodeFile(GString *name);
  DisplayFontParam *getDisplayFont(GString *fontName);
  DisplayFontParam *getDisplayCIDFont(GString *fontName, GString *collection);
  GString *getPSFile();
  int getPSPaperWidth();
  int getPSPaperHeight();
  void getPSImageableArea(int *llx, int *lly, int *urx, int *ury);
  GBool getPSDuplex();
  GBool getPSCrop();
  GBool getPSExpandSmaller();
  GBool getPSShrinkLarger();
  GBool getPSCenter();
  PSLevel getPSLevel();
  PSFontParam *getPSFont(GString *fontName);
  PSFontParam *getPSFont16(GString *fontName, GString *collection, int wMode);
  GBool getPSEmbedType1();
  GBool getPSEmbedTrueType();
  GBool getPSEmbedCIDPostScript();
  GBool getPSEmbedCIDTrueType();
  GBool getPSPreload();
  GBool getPSOPI();
  GBool getPSASCIIHex();
  GString *getTextEncodingName();
  EndOfLineKind getTextEOL();
  GBool getTextPageBreaks();
  GBool getTextKeepTinyChars();
  GString *findFontFile(GString *fontName, char **exts);
  GString *getInitialZoom();
  GBool getContinuousView();
  GBool getEnableT1lib();
  GBool getEnableFreeType();
  GBool getAntialias();
  GBool getVectorAntialias();
  GBool getStrokeAdjust();
  ScreenType getScreenType();
  int getScreenSize();
  int getScreenDotRadius();
  double getScreenGamma();
  double getScreenBlackThreshold();
  double getScreenWhiteThreshold();
  GString *getURLCommand() { return urlCommand; }
  GString *getMovieCommand() { return movieCommand; }
  GBool getMapNumericCharNames();
  GBool getMapUnknownCharNames();
  GList *getKeyBinding(int code, int mods, int context);
  GBool getPrintCommands();
  GBool getErrQuiet();

  CharCodeToUnicode *getCIDToUnicode(GString *collection);
  CharCodeToUnicode *getUnicodeToUnicode(GString *fontName);
  UnicodeMap *getUnicodeMap(GString *encodingName);
  CMap *getCMap(GString *collection, GString *cMapName);
  UnicodeMap *getTextEncoding();

  //----- functions to set parameters

  void addDisplayFont(DisplayFontParam *param);
  void setPSFile(char *file);
  GBool setPSPaperSize(char *size);
  void setPSPaperWidth(int width);
  void setPSPaperHeight(int height);
  void setPSImageableArea(int llx, int lly, int urx, int ury);
  void setPSDuplex(GBool duplex);
  void setPSCrop(GBool crop);
  void setPSExpandSmaller(GBool expand);
  void setPSShrinkLarger(GBool shrink);
  void setPSCenter(GBool center);
  void setPSLevel(PSLevel level);
  void setPSEmbedType1(GBool embed);
  void setPSEmbedTrueType(GBool embed);
  void setPSEmbedCIDPostScript(GBool embed);
  void setPSEmbedCIDTrueType(GBool embed);
  void setPSPreload(GBool preload);
  void setPSOPI(GBool opi);
  void setPSASCIIHex(GBool hex);
  void setTextEncoding(char *encodingName);
  GBool setTextEOL(char *s);
  void setTextPageBreaks(GBool pageBreaks);
  void setTextKeepTinyChars(GBool keep);
  void setInitialZoom(char *s);
  void setContinuousView(GBool cont);
  GBool setEnableT1lib(char *s);
  GBool setEnableFreeType(char *s);
  GBool setAntialias(char *s);
  GBool setVectorAntialias(char *s);
  void setScreenType(ScreenType t);
  void setScreenSize(int size);
  void setScreenDotRadius(int r);
  void setScreenGamma(double gamma);
  void setScreenBlackThreshold(double thresh);
  void setScreenWhiteThreshold(double thresh);
  void setMapNumericCharNames(GBool map);
  void setMapUnknownCharNames(GBool map);
  void setPrintCommands(GBool printCommandsA);
  void setErrQuiet(GBool errQuietA);

  //----- security handlers

  void addSecurityHandler(XpdfSecurityHandler *handler);
  XpdfSecurityHandler *getSecurityHandler(char *name);

private:

  void createDefaultKeyBindings();
  void parseFile(GString *fileName, FILE *f);
  void parseNameToUnicode(GList *tokens, GString *fileName, int line);
  void parseCIDToUnicode(GList *tokens, GString *fileName, int line);
  void parseUnicodeToUnicode(GList *tokens, GString *fileName, int line);
  void parseUnicodeMap(GList *tokens, GString *fileName, int line);
  void parseCMapDir(GList *tokens, GString *fileName, int line);
  void parseToUnicodeDir(GList *tokens, GString *fileName, int line);
  void parseDisplayFont(GList *tokens, GHash *fontHash,
			DisplayFontParamKind kind,
			GString *fileName, int line);
  void parsePSFile(GList *tokens, GString *fileName, int line);
  void parsePSPaperSize(GList *tokens, GString *fileName, int line);
  void parsePSImageableArea(GList *tokens, GString *fileName, int line);
  void parsePSLevel(GList *tokens, GString *fileName, int line);
  void parsePSFont(GList *tokens, GString *fileName, int line);
  void parsePSFont16(char *cmdName, GList *fontList,
		     GList *tokens, GString *fileName, int line);
  void parseTextEncoding(GList *tokens, GString *fileName, int line);
  void parseTextEOL(GList *tokens, GString *fileName, int line);
  void parseFontDir(GList *tokens, GString *fileName, int line);
  void parseInitialZoom(GList *tokens, GString *fileName, int line);
  void parseScreenType(GList *tokens, GString *fileName, int line);
  void parseBind(GList *tokens, GString *fileName, int line);
  void parseUnbind(GList *tokens, GString *fileName, int line);
  GBool parseKey(GString *modKeyStr, GString *contextStr,
		 int *code, int *mods, int *context,
		 char *cmdName,
		 GList *tokens, GString *fileName, int line);
  void parseCommand(char *cmdName, GString **val,
		    GList *tokens, GString *fileName, int line);
  void parseYesNo(char *cmdName, GBool *flag,
		  GList *tokens, GString *fileName, int line);
  GBool parseYesNo2(char *token, GBool *flag);
  void parseInteger(char *cmdName, int *val,
		    GList *tokens, GString *fileName, int line);
  void parseFloat(char *cmdName, double *val,
		  GList *tokens, GString *fileName, int line);
  UnicodeMap *getUnicodeMap2(GString *encodingName);
#ifdef ENABLE_PLUGINS
  GBool loadPlugin(char *type, char *name);
#endif

  //----- static tables

  NameToCharCode *		// mapping from char name to
    macRomanReverseMap;		//   MacRomanEncoding index

  //----- user-modifiable settings

  GString *baseDir;		// base directory - for plugins, etc.
  NameToCharCode *		// mapping from char name to Unicode
    nameToUnicode;
  GHash *cidToUnicodes;		// files for mappings from char collections
				//   to Unicode, indexed by collection name
				//   [GString]
  GHash *unicodeToUnicodes;	// files for Unicode-to-Unicode mappings,
				//   indexed by font name pattern [GString]
  GHash *residentUnicodeMaps;	// mappings from Unicode to char codes,
				//   indexed by encoding name [UnicodeMap]
  GHash *unicodeMaps;		// files for mappings from Unicode to char
				//   codes, indexed by encoding name [GString]
  GHash *cMapDirs;		// list of CMap dirs, indexed by collection
				//   name [GList[GString]]
  GList *toUnicodeDirs;		// list of ToUnicode CMap dirs [GString]
  GHash *displayFonts;		// display font info, indexed by font name
				//   [DisplayFontParam]
#ifdef WIN32
  WinFontList *winFontList;	// system TrueType fonts
#endif
  GHash *displayCIDFonts;	// display CID font info, indexed by
				//   collection [DisplayFontParam]
  GHash *displayNamedCIDFonts;	// display CID font info, indexed by
				//   font name [DisplayFontParam]
  GString *psFile;		// PostScript file or command (for xpdf)
  int psPaperWidth;		// paper size, in PostScript points, for
  int psPaperHeight;		//   PostScript output
  int psImageableLLX,		// imageable area, in PostScript points,
      psImageableLLY,		//   for PostScript output
      psImageableURX,
      psImageableURY;
  GBool psCrop;			// crop PS output to CropBox
  GBool psExpandSmaller;	// expand smaller pages to fill paper
  GBool psShrinkLarger;		// shrink larger pages to fit paper
  GBool psCenter;		// center pages on the paper
  GBool psDuplex;		// enable duplexing in PostScript?
  PSLevel psLevel;		// PostScript level to generate
  GHash *psFonts;		// PostScript font info, indexed by PDF
				//   font name [PSFontParam]
  GList *psNamedFonts16;	// named 16-bit fonts [PSFontParam]
  GList *psFonts16;		// generic 16-bit fonts [PSFontParam]
  GBool psEmbedType1;		// embed Type 1 fonts?
  GBool psEmbedTrueType;	// embed TrueType fonts?
  GBool psEmbedCIDPostScript;	// embed CID PostScript fonts?
  GBool psEmbedCIDTrueType;	// embed CID TrueType fonts?
  GBool psPreload;		// preload PostScript images and forms into
				//   memory
  GBool psOPI;			// generate PostScript OPI comments?
  GBool psASCIIHex;		// use ASCIIHex instead of ASCII85?
  GString *textEncoding;	// encoding (unicodeMap) to use for text
				//   output
  EndOfLineKind textEOL;	// type of EOL marker to use for text
				//   output
  GBool textPageBreaks;		// insert end-of-page markers?
  GBool textKeepTinyChars;	// keep all characters in text output
  GList *fontDirs;		// list of font dirs [GString]
  GString *initialZoom;		// initial zoom level
  GBool continuousView;		// continuous view mode
  GBool enableT1lib;		// t1lib enable flag
  GBool enableFreeType;		// FreeType enable flag
  GBool antialias;		// font anti-aliasing enable flag
  GBool vectorAntialias;	// vector anti-aliasing enable flag
  GBool strokeAdjust;		// stroke adjustment enable flag
  ScreenType screenType;	// halftone screen type
  int screenSize;		// screen matrix size
  int screenDotRadius;		// screen dot radius
  double screenGamma;		// screen gamma correction
  double screenBlackThreshold;	// screen black clamping threshold
  double screenWhiteThreshold;	// screen white clamping threshold
  GString *urlCommand;		// command executed for URL links
  GString *movieCommand;	// command executed for movie annotations
  GBool mapNumericCharNames;	// map numeric char names (from font subsets)?
  GBool mapUnknownCharNames;	// map unknown char names?
  GList *keyBindings;		// key & mouse button bindings [KeyBinding]
  GBool printCommands;		// print the drawing commands
  GBool errQuiet;		// suppress error messages?

  CharCodeToUnicodeCache *cidToUnicodeCache;
  CharCodeToUnicodeCache *unicodeToUnicodeCache;
  UnicodeMapCache *unicodeMapCache;
  CMapCache *cMapCache;

#ifdef ENABLE_PLUGINS
  GList *plugins;		// list of plugins [Plugin]
  GList *securityHandlers;	// list of loaded security handlers
				//   [XpdfSecurityHandler]
#endif

#if MULTITHREADED
  GMutex mutex;
  GMutex unicodeMapCacheMutex;
  GMutex cMapCacheMutex;
#endif
};

#endif
