//========================================================================
//
// gfile.cc
//
// Miscellaneous file and directory name manipulation.
//
// Copyright 1996-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef WIN32
#  include <time.h>
#else
#  if defined(MACOS)
#    include <sys/stat.h>
#  elif !defined(ACORN)
#    include <sys/types.h>
#    include <sys/stat.h>
#    include <fcntl.h>
#  endif
#  include <limits.h>
#  include <string.h>
#  if !defined(VMS) && !defined(ACORN) && !defined(MACOS)
#    include <pwd.h>
#  endif
#  if defined(VMS) && (__DECCXX_VER < 50200000)
#    include <unixlib.h>
#  endif
#endif // WIN32
#include "GString.h"
#include "gfile.h"

// Some systems don't define this, so just make it something reasonably
// large.
#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

//------------------------------------------------------------------------

GString *getHomeDir() {
#ifdef VMS
  //---------- VMS ----------
  return new GString("SYS$LOGIN:");

#elif defined(__EMX__) || defined(WIN32)
  //---------- OS/2+EMX and Win32 ----------
  char *s;
  GString *ret;

  if ((s = getenv("HOME")))
    ret = new GString(s);
  else
    ret = new GString(".");
  return ret;

#elif defined(ACORN)
  //---------- RISCOS ----------
  return new GString("@");

#elif defined(MACOS)
  //---------- MacOS ----------
  return new GString(":");

#else
  //---------- Unix ----------
  char *s;
  struct passwd *pw;
  GString *ret;

  if ((s = getenv("HOME"))) {
    ret = new GString(s);
  } else {
    if ((s = getenv("USER")))
      pw = getpwnam(s);
    else
      pw = getpwuid(getuid());
    if (pw)
      ret = new GString(pw->pw_dir);
    else
      ret = new GString(".");
  }
  return ret;
#endif
}

GString *getCurrentDir() {
  char buf[PATH_MAX+1];

#if defined(__EMX__)
  if (_getcwd2(buf, sizeof(buf)))
#elif defined(WIN32)
  if (GetCurrentDirectory(sizeof(buf), buf))
#elif defined(ACORN)
  if (strcpy(buf, "@"))
#elif defined(MACOS)
  if (strcpy(buf, ":"))
#else
  if (getcwd(buf, sizeof(buf)))
#endif
    return new GString(buf);
  return new GString();
}

GString *appendToPath(GString *path, char *fileName) {
#if defined(VMS)
  //---------- VMS ----------
  //~ this should handle everything necessary for file
  //~ requesters, but it's certainly not complete
  char *p0, *p1, *p2;
  char *q1;

  p0 = path->getCString();
  p1 = p0 + path->getLength() - 1;
  if (!strcmp(fileName, "-")) {
    if (*p1 == ']') {
      for (p2 = p1; p2 > p0 && *p2 != '.' && *p2 != '['; --p2) ;
      if (*p2 == '[')
	++p2;
      path->del(p2 - p0, p1 - p2);
    } else if (*p1 == ':') {
      path->append("[-]");
    } else {
      path->clear();
      path->append("[-]");
    }
  } else if ((q1 = strrchr(fileName, '.')) && !strncmp(q1, ".DIR;", 5)) {
    if (*p1 == ']') {
      path->insert(p1 - p0, '.');
      path->insert(p1 - p0 + 1, fileName, q1 - fileName);
    } else if (*p1 == ':') {
      path->append('[');
      path->append(']');
      path->append(fileName, q1 - fileName);
    } else {
      path->clear();
      path->append(fileName, q1 - fileName);
    }
  } else {
    if (*p1 != ']' && *p1 != ':')
      path->clear();
    path->append(fileName);
  }
  return path;

#elif defined(WIN32)
  //---------- Win32 ----------
  GString *tmp;
  char buf[256];
  char *fp;

  tmp = new GString(path);
  tmp->append('/');
  tmp->append(fileName);
  GetFullPathName(tmp->getCString(), sizeof(buf), buf, &fp);
  delete tmp;
  path->clear();
  path->append(buf);
  return path;

#elif defined(ACORN)
  //---------- RISCOS ----------
  char *p;
  int i;

  path->append(".");
  i = path->getLength();
  path->append(fileName);
  for (p = path->getCString() + i; *p; ++p) {
    if (*p == '/') {
      *p = '.';
    } else if (*p == '.') {
      *p = '/';
    }
  }
  return path;

#elif defined(MACOS)
  //---------- MacOS ----------
  char *p;
  int i;

  path->append(":");
  i = path->getLength();
  path->append(fileName);
  for (p = path->getCString() + i; *p; ++p) {
    if (*p == '/') {
      *p = ':';
    } else if (*p == '.') {
      *p = ':';
    }
  }
  return path;

#elif defined(__EMX__)
  //---------- OS/2+EMX ----------
  int i;

  // appending "." does nothing
  if (!strcmp(fileName, "."))
    return path;

  // appending ".." goes up one directory
  if (!strcmp(fileName, "..")) {
    for (i = path->getLength() - 2; i >= 0; --i) {
      if (path->getChar(i) == '/' || path->getChar(i) == '\\' ||
	  path->getChar(i) == ':')
	break;
    }
    if (i <= 0) {
      if (path->getChar(0) == '/' || path->getChar(0) == '\\') {
	path->del(1, path->getLength() - 1);
      } else if (path->getLength() >= 2 && path->getChar(1) == ':') {
	path->del(2, path->getLength() - 2);
      } else {
	path->clear();
	path->append("..");
      }
    } else {
      if (path->getChar(i-1) == ':')
	++i;
      path->del(i, path->getLength() - i);
    }
    return path;
  }

  // otherwise, append "/" and new path component
  if (path->getLength() > 0 &&
      path->getChar(path->getLength() - 1) != '/' &&
      path->getChar(path->getLength() - 1) != '\\')
    path->append('/');
  path->append(fileName);
  return path;

#else
  //---------- Unix ----------
  int i;

  // appending "." does nothing
  if (!strcmp(fileName, "."))
    return path;

  // appending ".." goes up one directory
  if (!strcmp(fileName, "..")) {
    for (i = path->getLength() - 2; i >= 0; --i) {
      if (path->getChar(i) == '/')
	break;
    }
    if (i <= 0) {
      if (path->getChar(0) == '/') {
	path->del(1, path->getLength() - 1);
      } else {
	path->clear();
	path->append("..");
      }
    } else {
      path->del(i, path->getLength() - i);
    }
    return path;
  }

  // otherwise, append "/" and new path component
  if (path->getLength() > 0 &&
      path->getChar(path->getLength() - 1) != '/')
    path->append('/');
  path->append(fileName);
  return path;
#endif
}

GString *grabPath(char *fileName) {
#ifdef VMS
  //---------- VMS ----------
  char *p;

  if ((p = strrchr(fileName, ']')))
    return new GString(fileName, p + 1 - fileName);
  if ((p = strrchr(fileName, ':')))
    return new GString(fileName, p + 1 - fileName);
  return new GString();

#elif defined(__EMX__) || defined(WIN32)
  //---------- OS/2+EMX and Win32 ----------
  char *p;

  if ((p = strrchr(fileName, '/')))
    return new GString(fileName, p - fileName);
  if ((p = strrchr(fileName, '\\')))
    return new GString(fileName, p - fileName);
  if ((p = strrchr(fileName, ':')))
    return new GString(fileName, p + 1 - fileName);
  return new GString();

#elif defined(ACORN)
  //---------- RISCOS ----------
  char *p;

  if ((p = strrchr(fileName, '.')))
    return new GString(fileName, p - fileName);
  return new GString();

#elif defined(MACOS)
  //---------- MacOS ----------
  char *p;

  if ((p = strrchr(fileName, ':')))
    return new GString(fileName, p - fileName);
  return new GString();

#else
  //---------- Unix ----------
  char *p;

  if ((p = strrchr(fileName, '/')))
    return new GString(fileName, p - fileName);
  return new GString();
#endif
}

GBool isAbsolutePath(char *path) {
#ifdef VMS
  //---------- VMS ----------
  return strchr(path, ':') ||
	 (path[0] == '[' && path[1] != '.' && path[1] != '-');

#elif defined(__EMX__) || defined(WIN32)
  //---------- OS/2+EMX and Win32 ----------
  return path[0] == '/' || path[0] == '\\' || path[1] == ':';

#elif defined(ACORN)
  //---------- RISCOS ----------
  return path[0] == '$';

#elif defined(MACOS)
  //---------- MacOS ----------
  return path[0] != ':';

#else
  //---------- Unix ----------
  return path[0] == '/';
#endif
}

GString *makePathAbsolute(GString *path) {
#ifdef VMS
  //---------- VMS ----------
  char buf[PATH_MAX+1];

  if (!isAbsolutePath(path->getCString())) {
    if (getcwd(buf, sizeof(buf))) {
      path->insert(0, buf);
    }
  }
  return path;

#elif defined(WIN32)
  //---------- Win32 ----------
  char buf[_MAX_PATH];
  char *fp;

  buf[0] = '\0';
  if (!GetFullPathName(path->getCString(), _MAX_PATH, buf, &fp)) {
    path->clear();
    return path;
  }
  path->clear();
  path->append(buf);
  return path;

#elif defined(ACORN)
  //---------- RISCOS ----------
  path->insert(0, '@');
  return path;

#elif defined(MACOS)
  //---------- MacOS ----------
  path->del(0, 1);
  return path;

#else
  //---------- Unix and OS/2+EMX ----------
  struct passwd *pw;
  char buf[PATH_MAX+1];
  GString *s;
  char *p1, *p2;
  int n;

  if (path->getChar(0) == '~') {
    if (path->getChar(1) == '/' ||
#ifdef __EMX__
	path->getChar(1) == '\\' ||
#endif
	path->getLength() == 1) {
      path->del(0, 1);
      s = getHomeDir();
      path->insert(0, s);
      delete s;
    } else {
      p1 = path->getCString() + 1;
#ifdef __EMX__
      for (p2 = p1; *p2 && *p2 != '/' && *p2 != '\\'; ++p2) ;
#else
      for (p2 = p1; *p2 && *p2 != '/'; ++p2) ;
#endif
      if ((n = p2 - p1) > PATH_MAX)
	n = PATH_MAX;
      strncpy(buf, p1, n);
      buf[n] = '\0';
      if ((pw = getpwnam(buf))) {
	path->del(0, p2 - p1 + 1);
	path->insert(0, pw->pw_dir);
      }
    }
  } else if (!isAbsolutePath(path->getCString())) {
    if (getcwd(buf, sizeof(buf))) {
#ifndef __EMX__
      path->insert(0, '/');
#endif
      path->insert(0, buf);
    }
  }
  return path;
#endif
}

time_t getModTime(char *fileName) {
#ifdef WIN32
  //~ should implement this, but it's (currently) only used in xpdf
  return 0;
#else
  struct stat statBuf;

  if (stat(fileName, &statBuf)) {
    return 0;
  }
  return statBuf.st_mtime;
#endif
}

GBool openTempFile(GString **name, FILE **f, char *mode, char *ext) {
#if defined(WIN32)
  //---------- Win32 ----------
  char *tempDir;
  GString *s, *s2;
  char buf[32];
  FILE *f2;
  int t, i;

  // this has the standard race condition problem, but I haven't found
  // a better way to generate temp file names with extensions on
  // Windows
  if ((tempDir = getenv("TEMP"))) {
    s = new GString(tempDir);
    s->append('\\');
  } else {
    s = new GString();
  }
  s->append("x");
  t = (int)time(NULL);
  for (i = 0; i < 1000; ++i) {
    sprintf(buf, "%d", t + i);
    s2 = s->copy()->append(buf);
    if (ext) {
      s2->append(ext);
    }
    if (!(f2 = fopen(s2->getCString(), "r"))) {
      if (!(f2 = fopen(s2->getCString(), mode))) {
	delete s2;
	delete s;
	return gFalse;
      }
      *name = s2;
      *f = f2;
      delete s;
      return gTrue;
    }
    fclose(f2);
    delete s2;
  }
  delete s;
  return gFalse;
#elif defined(VMS) || defined(__EMX__) || defined(ACORN) || defined(MACOS)
  //---------- non-Unix ----------
  char *s;

  // There is a security hole here: an attacker can create a symlink
  // with this file name after the tmpnam call and before the fopen
  // call.  I will happily accept fixes to this function for non-Unix
  // OSs.
  if (!(s = tmpnam(NULL))) {
    return gFalse;
  }
  *name = new GString(s);
  if (ext) {
    (*name)->append(ext);
  }
  if (!(*f = fopen((*name)->getCString(), mode))) {
    delete (*name);
    return gFalse;
  }
  return gTrue;
#else
  //---------- Unix ----------
  char *s;
  int fd;

  if (ext) {
#if HAVE_MKSTEMPS
    if ((s = getenv("TMPDIR"))) {
      *name = new GString(s);
    } else {
      *name = new GString("/tmp");
    }
    (*name)->append("/XXXXXX")->append(ext);
    fd = mkstemps((*name)->getCString(), strlen(ext));
#else
    if (!(s = tmpnam(NULL))) {
      return gFalse;
    }
    *name = new GString(s);
    (*name)->append(ext);
    fd = open((*name)->getCString(), O_WRONLY | O_CREAT | O_EXCL, 0600);
#endif
  } else {
#if HAVE_MKSTEMP
    if ((s = getenv("TMPDIR"))) {
      *name = new GString(s);
    } else {
      *name = new GString("/tmp");
    }
    (*name)->append("/XXXXXX");
    fd = mkstemp((*name)->getCString());
#else // HAVE_MKSTEMP
    if (!(s = tmpnam(NULL))) {
      return gFalse;
    }
    *name = new GString(s);
    fd = open((*name)->getCString(), O_WRONLY | O_CREAT | O_EXCL, 0600);
#endif // HAVE_MKSTEMP
  }
  if (fd < 0 || !(*f = fdopen(fd, mode))) {
    delete *name;
    return gFalse;
  }
  return gTrue;
#endif
}

GBool executeCommand(char *cmd) {
#ifdef VMS
  return system(cmd) ? gTrue : gFalse;
#else
  return system(cmd) ? gFalse : gTrue;
#endif
}

char *getLine(char *buf, int size, FILE *f) {
  int c, i;

  i = 0;
  while (i < size - 1) {
    if ((c = fgetc(f)) == EOF) {
      break;
    }
    buf[i++] = (char)c;
    if (c == '\x0a') {
      break;
    }
    if (c == '\x0d') {
      c = fgetc(f);
      if (c == '\x0a' && i < size - 1) {
	buf[i++] = (char)c;
      } else if (c != EOF) {
	ungetc(c, f);
      }
      break;
    }
  }
  buf[i] = '\0';
  if (i == 0) {
    return NULL;
  }
  return buf;
}

//------------------------------------------------------------------------
// GDir and GDirEntry
//------------------------------------------------------------------------

GDirEntry::GDirEntry(char *dirPath, char *nameA, GBool doStat) {
#ifdef VMS
  char *p;
#elif defined(WIN32)
  int fa;
  GString *s;
#elif defined(ACORN)
#else
  struct stat st;
  GString *s;
#endif

  name = new GString(nameA);
  dir = gFalse;
  if (doStat) {
#ifdef VMS
    if (!strcmp(nameA, "-") ||
	((p = strrchr(nameA, '.')) && !strncmp(p, ".DIR;", 5)))
      dir = gTrue;
#elif defined(ACORN)
#else
    s = new GString(dirPath);
    appendToPath(s, nameA);
#ifdef WIN32
    fa = GetFileAttributes(s->getCString());
    dir = (fa != 0xFFFFFFFF && (fa & FILE_ATTRIBUTE_DIRECTORY));
#else
    if (stat(s->getCString(), &st) == 0)
      dir = S_ISDIR(st.st_mode);
#endif
    delete s;
#endif
  }
}

GDirEntry::~GDirEntry() {
  delete name;
}

GDir::GDir(char *name, GBool doStatA) {
  path = new GString(name);
  doStat = doStatA;
#if defined(WIN32)
  GString *tmp;

  tmp = path->copy();
  tmp->append("/*.*");
  hnd = FindFirstFile(tmp->getCString(), &ffd);
  delete tmp;
#elif defined(ACORN)
#elif defined(MACOS)
#else
  dir = opendir(name);
#ifdef VMS
  needParent = strchr(name, '[') != NULL;
#endif
#endif
}

GDir::~GDir() {
  delete path;
#if defined(WIN32)
  if (hnd) {
    FindClose(hnd);
    hnd = NULL;
  }
#elif defined(ACORN)
#elif defined(MACOS)
#else
  if (dir)
    closedir(dir);
#endif
}

GDirEntry *GDir::getNextEntry() {
  GDirEntry *e;

#if defined(WIN32)
  if (hnd) {
    e = new GDirEntry(path->getCString(), ffd.cFileName, doStat);
    if (hnd  && !FindNextFile(hnd, &ffd)) {
      FindClose(hnd);
      hnd = NULL;
    }
  } else {
    e = NULL;
  }
#elif defined(ACORN)
#elif defined(MACOS)
#elif defined(VMS)
  struct dirent *ent;
  e = NULL;
  if (dir) {
    if (needParent) {
      e = new GDirEntry(path->getCString(), "-", doStat);
      needParent = gFalse;
      return e;
    }
    ent = readdir(dir);
    if (ent) {
      e = new GDirEntry(path->getCString(), ent->d_name, doStat);
    }
  }
#else
  struct dirent *ent;
  e = NULL;
  if (dir) {
    ent = (struct dirent *)readdir(dir);
    if (ent && !strcmp(ent->d_name, ".")) {
      ent = (struct dirent *)readdir(dir);
    }
    if (ent) {
      e = new GDirEntry(path->getCString(), ent->d_name, doStat);
    }
  }
#endif

  return e;
}

void GDir::rewind() {
#ifdef WIN32
  GString *tmp;

  if (hnd)
    FindClose(hnd);
  tmp = path->copy();
  tmp->append("/*.*");
  hnd = FindFirstFile(tmp->getCString(), &ffd);
  delete tmp;
#elif defined(ACORN)
#elif defined(MACOS)
#else
  if (dir)
    rewinddir(dir);
#ifdef VMS
  needParent = strchr(path->getCString(), '[') != NULL;
#endif
#endif
}
