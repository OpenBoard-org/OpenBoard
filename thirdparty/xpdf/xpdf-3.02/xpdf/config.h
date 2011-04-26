//========================================================================
//
// config.h
//
// Copyright 1996-2007 Glyph & Cog, LLC
//
//========================================================================

#ifndef CONFIG_H
#define CONFIG_H

//------------------------------------------------------------------------
// version
//------------------------------------------------------------------------

// xpdf version
#define xpdfVersion          "3.02"
#define xpdfVersionNum       3.02
#define xpdfMajorVersion     3
#define xpdfMinorVersion     2
#define xpdfUpdateVersion    0
#define xpdfMajorVersionStr  "3"
#define xpdfMinorVersionStr  "2"
#define xpdfUpdateVersionStr "0"

// supported PDF version
#define supportedPDFVersionStr "1.7"
#define supportedPDFVersionNum 1.7

// copyright notice
#define xpdfCopyright "Copyright 1996-2007 Glyph & Cog, LLC"

// Windows resource file stuff
#define winxpdfVersion "WinXpdf 3.02"
#define xpdfCopyrightAmp "Copyright 1996-2007 Glyph && Cog, LLC"

//------------------------------------------------------------------------
// paper size
//------------------------------------------------------------------------

// default paper size (in points) for PostScript output
#ifdef A4_PAPER
#define defPaperWidth  595    // ISO A4 (210x297 mm)
#define defPaperHeight 842
#else
#define defPaperWidth  612    // American letter (8.5x11")
#define defPaperHeight 792
#endif

//------------------------------------------------------------------------
// config file (xpdfrc) path
//------------------------------------------------------------------------

// user config file name, relative to the user's home directory
#if defined(VMS) || (defined(WIN32) && !defined(__CYGWIN32__))
#define xpdfUserConfigFile "xpdfrc"
#else
#define xpdfUserConfigFile ".xpdfrc"
#endif

// system config file name (set via the configure script)
#ifdef SYSTEM_XPDFRC
#define xpdfSysConfigFile SYSTEM_XPDFRC
#else
// under Windows, we get the directory with the executable and then
// append this file name
#define xpdfSysConfigFile "xpdfrc"
#endif

//------------------------------------------------------------------------
// X-related constants
//------------------------------------------------------------------------

// default maximum size of color cube to allocate
#define defaultRGBCube 5

// number of fonts (combined t1lib, FreeType, X server) to cache
#define xOutFontCacheSize 64

// number of Type 3 fonts to cache
#define xOutT3FontCacheSize 8

//------------------------------------------------------------------------
// popen
//------------------------------------------------------------------------

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define popen _popen
#define pclose _pclose
#endif

#if defined(VMS) || defined(VMCMS) || defined(DOS) || defined(OS2) || defined(__EMX__) || defined(WIN32) || defined(__DJGPP__) || defined(MACOS)
#define POPEN_READ_MODE "rb"
#else
#define POPEN_READ_MODE "r"
#endif

//------------------------------------------------------------------------
// Win32 stuff
//------------------------------------------------------------------------

#ifdef CDECL
#undef CDECL
#endif

#if defined(_MSC_VER) || defined(__BORLANDC__)
#define CDECL __cdecl
#else
#define CDECL
#endif

#endif
