//========================================================================
//
// XPDFApp.cc
//
// Copyright 2002-2003 Glyph & Cog, LLC
//
//========================================================================

#include <aconf.h>

#ifdef USE_GCC_PRAGMAS
#pragma implementation
#endif

#include "GString.h"
#include "GList.h"
#include "Error.h"
#include "XPDFViewer.h"
#include "XPDFApp.h"
#include "config.h"

// these macro defns conflict with xpdf's Object class
#ifdef LESSTIF_VERSION
#undef XtDisplay
#undef XtScreen
#undef XtWindow
#undef XtParent
#undef XtIsRealized
#endif

//------------------------------------------------------------------------

#define remoteCmdSize 512

//------------------------------------------------------------------------

static String fallbackResources[] = {
  "*.zoomComboBox*fontList: -*-helvetica-medium-r-normal--12-*-*-*-*-*-iso8859-1",
  "*XmTextField.fontList: -*-courier-medium-r-normal--12-*-*-*-*-*-iso8859-1",
  "*.fontList: -*-helvetica-medium-r-normal--12-*-*-*-*-*-iso8859-1",
  "*XmTextField.translations: #override\\n"
  "  Ctrl<Key>a:beginning-of-line()\\n"
  "  Ctrl<Key>b:backward-character()\\n"
  "  Ctrl<Key>d:delete-next-character()\\n"
  "  Ctrl<Key>e:end-of-line()\\n"
  "  Ctrl<Key>f:forward-character()\\n"
  "  Ctrl<Key>u:beginning-of-line()delete-to-end-of-line()\\n"
  "  Ctrl<Key>k:delete-to-end-of-line()\\n",
  "*.toolTipEnable: True",
  "*.toolTipPostDelay: 1500",
  "*.toolTipPostDuration: 0",
  "*.TipLabel.foreground: black",
  "*.TipLabel.background: LightYellow",
  "*.TipShell.borderWidth: 1",
  "*.TipShell.borderColor: black",
  NULL
};

static XrmOptionDescRec xOpts[] = {
  {"-display",       ".display",         XrmoptionSepArg,  NULL},
  {"-foreground",    "*Foreground",      XrmoptionSepArg,  NULL},
  {"-fg",            "*Foreground",      XrmoptionSepArg,  NULL},
  {"-background",    "*Background",      XrmoptionSepArg,  NULL},
  {"-bg",            "*Background",      XrmoptionSepArg,  NULL},
  {"-geometry",      ".geometry",        XrmoptionSepArg,  NULL},
  {"-g",             ".geometry",        XrmoptionSepArg,  NULL},
  {"-font",          "*.fontList",       XrmoptionSepArg,  NULL},
  {"-fn",            "*.fontList",       XrmoptionSepArg,  NULL},
  {"-title",         ".title",           XrmoptionSepArg,  NULL},
  {"-cmap",          ".installCmap",     XrmoptionNoArg,   (XPointer)"on"},
  {"-rgb",           ".rgbCubeSize",     XrmoptionSepArg,  NULL},
  {"-rv",            ".reverseVideo",    XrmoptionNoArg,   (XPointer)"true"},
  {"-papercolor",    ".paperColor",      XrmoptionSepArg,  NULL},
  {"-mattecolor",    ".matteColor",      XrmoptionSepArg,  NULL},
  {"-z",             ".initialZoom",     XrmoptionSepArg,  NULL}
};

#define nXOpts (sizeof(xOpts) / sizeof(XrmOptionDescRec))

struct XPDFAppResources {
  String geometry;
  String title;
  Bool installCmap;
  int rgbCubeSize;
  Bool reverseVideo;
  String paperColor;
  String matteColor;
  String fullScreenMatteColor;
  String initialZoom;
};

static Bool defInstallCmap = False;
static int defRGBCubeSize = defaultRGBCube;
static Bool defReverseVideo = False;

static XtResource xResources[] = {
  { "geometry",             "Geometry",             XtRString, sizeof(String), XtOffsetOf(XPDFAppResources, geometry),             XtRString, (XtPointer)NULL             },
  { "title",                "Title",                XtRString, sizeof(String), XtOffsetOf(XPDFAppResources, title),                XtRString, (XtPointer)NULL             },
  { "installCmap",          "InstallCmap",          XtRBool,   sizeof(Bool),   XtOffsetOf(XPDFAppResources, installCmap),          XtRBool,   (XtPointer)&defInstallCmap  },
  { "rgbCubeSize",          "RgbCubeSize",          XtRInt,    sizeof(int),    XtOffsetOf(XPDFAppResources, rgbCubeSize),          XtRInt,    (XtPointer)&defRGBCubeSize  },
  { "reverseVideo",         "ReverseVideo",         XtRBool,   sizeof(Bool),   XtOffsetOf(XPDFAppResources, reverseVideo),         XtRBool,   (XtPointer)&defReverseVideo },
  { "paperColor",           "PaperColor",           XtRString, sizeof(String), XtOffsetOf(XPDFAppResources, paperColor),           XtRString, (XtPointer)NULL             },
  { "matteColor",           "MatteColor",           XtRString, sizeof(String), XtOffsetOf(XPDFAppResources, matteColor),           XtRString, (XtPointer)"gray50"         },
  { "fullScreenMatteColor", "FullScreenMatteColor", XtRString, sizeof(String), XtOffsetOf(XPDFAppResources, fullScreenMatteColor), XtRString, (XtPointer)"black"          },
  { "initialZoom",          "InitialZoom",          XtRString, sizeof(String), XtOffsetOf(XPDFAppResources, initialZoom),          XtRString, (XtPointer)NULL             }
};

#define nXResources (sizeof(xResources) / sizeof(XtResource))

//------------------------------------------------------------------------
// XPDFApp
//------------------------------------------------------------------------

#if 0 //~ for debugging
static int xErrorHandler(Display *display, XErrorEvent *ev) {
  printf("X error:\n");
  printf("  resource ID = %08lx\n", ev->resourceid);
  printf("  serial = %lu\n", ev->serial);
  printf("  error_code = %d\n", ev->error_code);
  printf("  request_code = %d\n", ev->request_code);
  printf("  minor_code = %d\n", ev->minor_code);
  fflush(stdout);
  abort();
}
#endif

XPDFApp::XPDFApp(int *argc, char *argv[]) {
  appShell = XtAppInitialize(&appContext, xpdfAppName, xOpts, nXOpts,
			     argc, argv, fallbackResources, NULL, 0);
  display = XtDisplay(appShell);
  screenNum = XScreenNumberOfScreen(XtScreen(appShell));
#if XmVERSION > 1
  XtVaSetValues(XmGetXmDisplay(XtDisplay(appShell)),
		XmNenableButtonTab, True, NULL);
#endif
#if XmVERSION > 1
  // Drag-and-drop appears to be buggy -- I'm seeing weird crashes
  // deep in the Motif code when I destroy widgets in the XpdfForms
  // code.  Xpdf doesn't use it, so just turn it off.
  XtVaSetValues(XmGetXmDisplay(XtDisplay(appShell)),
		XmNdragInitiatorProtocolStyle, XmDRAG_NONE,
		XmNdragReceiverProtocolStyle, XmDRAG_NONE,
		NULL);
#endif

#if 0 //~ for debugging
  XSynchronize(display, True);
  XSetErrorHandler(&xErrorHandler);
#endif

  fullScreen = gFalse;
  remoteAtom = None;
  remoteViewer = NULL;
  remoteWin = None;

  getResources();

  viewers = new GList();

}

void XPDFApp::getResources() {
  XPDFAppResources resources;
  XColor xcol, xcol2;
  Colormap colormap;
  
  XtGetApplicationResources(appShell, &resources, xResources, nXResources,
			    NULL, 0);
  geometry = resources.geometry ? new GString(resources.geometry)
                                : (GString *)NULL;
  title = resources.title ? new GString(resources.title) : (GString *)NULL;
  installCmap = (GBool)resources.installCmap;
  rgbCubeSize = resources.rgbCubeSize;
  reverseVideo = (GBool)resources.reverseVideo;
  if (reverseVideo) {
    paperRGB[0] = paperRGB[1] = paperRGB[2] = 0;
    paperPixel = BlackPixel(display, screenNum);
  } else {
    paperRGB[0] = paperRGB[1] = paperRGB[2] = 0xff;
    paperPixel = WhitePixel(display, screenNum);
  }
  XtVaGetValues(appShell, XmNcolormap, &colormap, NULL);
  if (resources.paperColor) {
    if (XAllocNamedColor(display, colormap, resources.paperColor,
			 &xcol, &xcol2)) {
      paperRGB[0] = xcol.red >> 8;
      paperRGB[1] = xcol.green >> 8;
      paperRGB[2] = xcol.blue >> 8;
      paperPixel = xcol.pixel;
    } else {
      error(-1, "Couldn't allocate color '%s'", resources.paperColor);
    }
  }
  if (XAllocNamedColor(display, colormap, resources.matteColor,
		       &xcol, &xcol2)) {
    mattePixel = xcol.pixel;
  } else {
    mattePixel = paperPixel;
  }
  if (XAllocNamedColor(display, colormap, resources.fullScreenMatteColor,
		       &xcol, &xcol2)) {
    fullScreenMattePixel = xcol.pixel;
  } else {
    fullScreenMattePixel = paperPixel;
  }
  initialZoom = resources.initialZoom ? new GString(resources.initialZoom)
                                      : (GString *)NULL;
}

XPDFApp::~XPDFApp() {
  deleteGList(viewers, XPDFViewer);
  if (geometry) {
    delete geometry;
  }
  if (title) {
    delete title;
  }
  if (initialZoom) {
    delete initialZoom;
  }
}

XPDFViewer *XPDFApp::open(GString *fileName, int page,
			  GString *ownerPassword, GString *userPassword) {
  XPDFViewer *viewer;

  viewer = new XPDFViewer(this, fileName, page, NULL, fullScreen,
			  ownerPassword, userPassword);
  if (!viewer->isOk()) {
    delete viewer;
    return NULL;
  }
  if (remoteAtom != None) {
    remoteViewer = viewer;
    remoteWin = viewer->getWindow();
    XtAddEventHandler(remoteWin, PropertyChangeMask, False,
		      &remoteMsgCbk, this);
    XSetSelectionOwner(display, remoteAtom, XtWindow(remoteWin), CurrentTime);
  }
  viewers->append(viewer);
  return viewer;
}

XPDFViewer *XPDFApp::openAtDest(GString *fileName, GString *dest,
				GString *ownerPassword,
				GString *userPassword) {
  XPDFViewer *viewer;

  viewer = new XPDFViewer(this, fileName, 1, dest, fullScreen,
			  ownerPassword, userPassword);
  if (!viewer->isOk()) {
    delete viewer;
    return NULL;
  }
  if (remoteAtom != None) {
    remoteViewer = viewer;
    remoteWin = viewer->getWindow();
    XtAddEventHandler(remoteWin, PropertyChangeMask, False,
		      &remoteMsgCbk, this);
    XSetSelectionOwner(display, remoteAtom, XtWindow(remoteWin), CurrentTime);
  }
  viewers->append(viewer);
  return viewer;
}

XPDFViewer *XPDFApp::reopen(XPDFViewer *viewer, PDFDoc *doc, int page,
			    GBool fullScreenA) {
  int i;

  for (i = 0; i < viewers->getLength(); ++i) {
    if (((XPDFViewer *)viewers->get(i)) == viewer) {
      viewers->del(i);
      delete viewer;
    }
  }
  viewer = new XPDFViewer(this, doc, page, NULL, fullScreenA);
  if (!viewer->isOk()) {
    delete viewer;
    return NULL;
  }
  if (remoteAtom != None) {
    remoteViewer = viewer;
    remoteWin = viewer->getWindow();
    XtAddEventHandler(remoteWin, PropertyChangeMask, False,
		      &remoteMsgCbk, this);
    XSetSelectionOwner(display, remoteAtom, XtWindow(remoteWin), CurrentTime);
  }
  viewers->append(viewer);
  return viewer;
}

void XPDFApp::close(XPDFViewer *viewer, GBool closeLast) {
  int i;

  if (viewers->getLength() == 1) {
    if (viewer != (XPDFViewer *)viewers->get(0)) {
      return;
    }
    if (closeLast) {
      quit();
    } else {
      viewer->clear();
    }
  } else {
    for (i = 0; i < viewers->getLength(); ++i) {
      if (((XPDFViewer *)viewers->get(i)) == viewer) {
	viewers->del(i);
	if (remoteAtom != None && remoteViewer == viewer) {
	  remoteViewer = (XPDFViewer *)viewers->get(viewers->getLength() - 1);
	  remoteWin = remoteViewer->getWindow();
	  XSetSelectionOwner(display, remoteAtom, XtWindow(remoteWin),
			     CurrentTime);
	}
	delete viewer;
	return;
      }
    }
  }
}

void XPDFApp::quit() {
  if (remoteAtom != None) {
    XSetSelectionOwner(display, remoteAtom, None, CurrentTime);
  }
  while (viewers->getLength() > 0) {
    delete (XPDFViewer *)viewers->del(0);
  }
#if HAVE_XTAPPSETEXITFLAG
  XtAppSetExitFlag(appContext);
#else
  exit(0);
#endif
}

void XPDFApp::run() {
  XtAppMainLoop(appContext);
}

void XPDFApp::setRemoteName(char *remoteName) {
  remoteAtom = XInternAtom(display, remoteName, False);
  remoteXWin = XGetSelectionOwner(display, remoteAtom);
}

GBool XPDFApp::remoteServerRunning() {
  return remoteXWin != None;
}

void XPDFApp::remoteExec(char *cmd) {
  char cmd2[remoteCmdSize];
  int n;

  n = strlen(cmd);
  if (n > remoteCmdSize - 2) {
    n = remoteCmdSize - 2;
  }
  memcpy(cmd2, cmd, n);
  cmd2[n] = '\n';
  cmd2[n+1] = '\0';
  XChangeProperty(display, remoteXWin, remoteAtom, remoteAtom, 8,
		  PropModeReplace, (Guchar *)cmd2, n + 2);
  XFlush(display);
}

void XPDFApp::remoteOpen(GString *fileName, int page, GBool raise) {
  char cmd[remoteCmdSize];

  sprintf(cmd, "openFileAtPage(%.200s,%d)\n",
	  fileName->getCString(), page);
  if (raise) {
    strcat(cmd, "raise\n");
  }
  XChangeProperty(display, remoteXWin, remoteAtom, remoteAtom, 8,
		  PropModeReplace, (Guchar *)cmd, strlen(cmd) + 1);
  XFlush(display);
}

void XPDFApp::remoteOpenAtDest(GString *fileName, GString *dest, GBool raise) {
  char cmd[remoteCmdSize];

  sprintf(cmd, "openFileAtDest(%.200s,%.256s)\n",
	  fileName->getCString(), dest->getCString());
  if (raise) {
    strcat(cmd, "raise\n");
  }
  XChangeProperty(display, remoteXWin, remoteAtom, remoteAtom, 8,
		  PropModeReplace, (Guchar *)cmd, strlen(cmd) + 1);
  XFlush(display);
}

void XPDFApp::remoteReload(GBool raise) {
  char cmd[remoteCmdSize];

  strcpy(cmd, "reload\n");
  if (raise) {
    strcat(cmd, "raise\n");
  }
  XChangeProperty(display, remoteXWin, remoteAtom, remoteAtom, 8,
		  PropModeReplace, (Guchar *)cmd, strlen(cmd) + 1);
  XFlush(display);
}

void XPDFApp::remoteRaise() {
  XChangeProperty(display, remoteXWin, remoteAtom, remoteAtom, 8,
		  PropModeReplace, (Guchar *)"raise\n", 7);
  XFlush(display);
}

void XPDFApp::remoteQuit() {
  XChangeProperty(display, remoteXWin, remoteAtom, remoteAtom, 8,
		  PropModeReplace, (Guchar *)"quit\n", 6);
  XFlush(display);
}

void XPDFApp::remoteMsgCbk(Widget widget, XtPointer ptr,
			   XEvent *event, Boolean *cont) {
  XPDFApp *app = (XPDFApp *)ptr;
  char *cmd, *p0, *p1;
  Atom type;
  int format;
  Gulong size, remain;
  GString *cmdStr;

  if (event->xproperty.atom != app->remoteAtom) {
    *cont = True;
    return;
  }
  *cont = False;

  if (XGetWindowProperty(app->display, XtWindow(app->remoteWin),
			 app->remoteAtom, 0, remoteCmdSize/4,
			 True, app->remoteAtom,
			 &type, &format, &size, &remain,
			 (Guchar **)&cmd) != Success) {
    return;
  }
  if (!cmd) {
    return;
  }
  p0 = cmd;
  while (*p0 && (p1 = strchr(p0, '\n'))) {
    cmdStr = new GString(p0, p1 - p0);
    app->remoteViewer->execCmd(cmdStr, NULL);
    delete cmdStr;
    p0 = p1 + 1;
  }
  XFree((XPointer)cmd);
}
