HEADERS += src/gui/UBThumbnailView.h \
    src/gui/UBFloatingPalette.h \
    src/gui/UBToolbarButtonGroup.h \
    src/gui/UBStylusPalette.h \
    src/gui/UBIconButton.h \
    src/gui/UBThumbnailWidget.h \
    src/gui/UBCircleFrame.h \
    src/gui/UBColorPicker.h \
    src/gui/UBWidgetMirror.h \
    src/gui/UBScreenMirror.h \
    src/gui/UBResources.h \
    src/gui/UBMessageWindow.h \
    src/gui/UBDocumentThumbnailWidget.h \
#    src/gui/UBDocumentTreeWidget.h \
    src/gui/UBMousePressFilter.h \
    src/gui/UBBlackoutWidget.h \
    src/gui/UBMainWindow.h \
    src/gui/UBToolWidget.h \
    src/gui/UBSpinningWheel.h \
    src/gui/UBZoomPalette.h \
    src/gui/UBRubberBand.h \
    src/gui/UBDocumentToolsPalette.h \
    src/gui/UBWebToolsPalette.h \
    src/gui/UBActionPalette.h \
    src/gui/UBFavoriteToolPalette.h \
    src/gui/UBKeyboardPalette.h \
    src/gui/UBDockPalette.h \
    src/gui/UBPropertyPalette.h \
    src/gui/UBUpdateDlg.h \
    src/gui/UBDockPaletteWidget.h \
    src/gui/UBLeftPalette.h \
    src/gui/UBRightPalette.h \
    src/gui/UBPageNavigationWidget.h \
    src/gui/UBMagnifer.h \
    src/gui/UBCachePropertiesWidget.h \
    src/gui/UBDownloadWidget.h \
    src/gui/UBDockDownloadWidget.h \
    src/gui/UBFeaturesWidget.h \
    src/gui/UBFeaturesActionBar.h \
    src/gui/UBMessagesDialog.h \
    src/gui/UBOpenSankoreImporterWidget.h \
    src/gui/UBStartupHintsPalette.h \
    src/gui/UBBackgroundPalette.h \
    src/gui/UBBoardThumbnailsView.h \
    src/gui/UBDocumentNavigator.h
SOURCES += src/gui/UBThumbnailView.cpp \
    src/gui/UBFloatingPalette.cpp \
    src/gui/UBToolbarButtonGroup.cpp \
    src/gui/UBStylusPalette.cpp \
    src/gui/UBIconButton.cpp \
    src/gui/UBThumbnailWidget.cpp \
    src/gui/UBCircleFrame.cpp \
    src/gui/UBColorPicker.cpp \
    src/gui/UBWidgetMirror.cpp \
    src/gui/UBScreenMirror.cpp \
    src/gui/UBResources.cpp \
    src/gui/UBMessageWindow.cpp \
    src/gui/UBDocumentThumbnailWidget.cpp \
#    src/gui/UBDocumentTreeWidget.cpp \
    src/gui/UBMousePressFilter.cpp \
    src/gui/UBBlackoutWidget.cpp \
    src/gui/UBMainWindow.cpp \
    src/gui/UBToolWidget.cpp \
    src/gui/UBSpinningWheel.cpp \
    src/gui/UBZoomPalette.cpp \
    src/gui/UBRubberBand.cpp \
    src/gui/UBDocumentToolsPalette.cpp \
    src/gui/UBWebToolsPalette.cpp \
    src/gui/UBActionPalette.cpp \
    src/gui/UBFavoriteToolPalette.cpp \
    src/gui/UBKeyboardPalette.cpp \
    src/gui/UBDockPalette.cpp \
    src/gui/UBPropertyPalette.cpp \
    src/gui/UBUpdateDlg.cpp \
    src/gui/UBDockPaletteWidget.cpp \
    src/gui/UBLeftPalette.cpp \
    src/gui/UBRightPalette.cpp \
    src/gui/UBPageNavigationWidget.cpp \
    src/gui/UBMagnifer.cpp \
    src/gui/UBCachePropertiesWidget.cpp \
    src/gui/UBDownloadWidget.cpp \
    src/gui/UBDockDownloadWidget.cpp \
    src/gui/UBFeaturesWidget.cpp \
    src/gui/UBFeaturesActionBar.cpp \
    src/gui/UBMessagesDialog.cpp \
    src/gui/UBOpenSankoreImporterWidget.cpp \
    src/gui/UBStartupHintsPalette.cpp \
    src/gui/UBBackgroundPalette.cpp \
    src/gui/UBBoardThumbnailsView.cpp \
    src/gui/UBDocumentNavigator.cpp
win32:SOURCES += src/gui/UBKeyboardPalette_win.cpp
macx:OBJECTIVE_SOURCES += src/gui/UBKeyboardPalette_mac.mm
linux-g++:SOURCES += src/gui/UBKeyboardPalette_linux.cpp
linux-g++-32:SOURCES += src/gui/UBKeyboardPalette_linux.cpp
linux-g++-64:SOURCES += src/gui/UBKeyboardPalette_linux.cpp

