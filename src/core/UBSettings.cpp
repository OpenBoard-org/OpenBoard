/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBSettings.h"

#include <QtGui>


#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBCryptoUtils.h"

#include "UB.h"
#include "UBSetting.h"

#include "tools/UBToolsManager.h"

#include "core/memcheck.h"

QPointer<UBSettings> UBSettings::sSingleton = 0;

int UBSettings::pointerDiameter = 40;
int UBSettings::crossSize = 24;
int UBSettings::defaultCrossSize = 24;
int UBSettings::minCrossSize = 12;
int UBSettings::maxCrossSize = 96; //TODO: user-settable?
int UBSettings::colorPaletteSize = 5;
int UBSettings::objectFrameWidth = 20;
int UBSettings::boardMargin = 10;

QString UBSettings::documentGroupName = QString("Subject");
QString UBSettings::documentName = QString("Lesson");
QString UBSettings::documentSize = QString("Size");
QString UBSettings::documentIdentifer = QString("ID");
QString UBSettings::documentVersion = QString("Version");
QString UBSettings::documentUpdatedAt = QString("UpdatedAt");
QString UBSettings::documentDate = QString("date");

QString UBSettings::trashedDocumentGroupNamePrefix = QString("_Trash:");

QString UBSettings::uniboardDocumentNamespaceUri = "http://uniboard.mnemis.com/document";
QString UBSettings::uniboardApplicationNamespaceUri = "http://uniboard.mnemis.com/application";

QString UBSettings::undoCommandTransactionName = "UndoTransaction";

const int UBSettings::sDefaultFontPixelSize = 36;
const char *UBSettings::sDefaultFontFamily = "Arial";
const char *UBSettings::sDefaultFontStyleName = "Regular";

QString UBSettings::currentFileVersion = "4.8.0";

QBrush UBSettings::eraserBrushDarkBackground = QBrush(QColor(127, 127, 127, 80));
QBrush UBSettings::eraserBrushLightBackground = QBrush(QColor(127, 127, 127, 80));

QPen UBSettings::eraserPenDarkBackground = QPen(QColor(255, 255, 255, 127));
QPen UBSettings::eraserPenLightBackground = QPen(QColor(0, 0, 0, 127));

QColor UBSettings::markerCircleBrushColorDarkBackground = QColor(127, 127, 127, 80);
QColor UBSettings::markerCircleBrushColorLightBackground = QColor(127, 127, 127, 80);

QColor UBSettings::markerCirclePenColorDarkBackground = QColor(255, 255, 255, 127);
QColor UBSettings::markerCirclePenColorLightBackground = QColor(0, 0, 0, 127);

QColor UBSettings::penCircleBrushColorDarkBackground = QColor(127, 127, 127, 80);
QColor UBSettings::penCircleBrushColorLightBackground = QColor(127, 127, 127, 80);

QColor UBSettings::penCirclePenColorDarkBackground = QColor(255, 255, 255, 127);
QColor UBSettings::penCirclePenColorLightBackground = QColor(0, 0, 0, 127);

QColor UBSettings::documentSizeMarkColorDarkBackground = QColor(44, 44, 44, 200);
QColor UBSettings::documentSizeMarkColorLightBackground = QColor(241, 241, 241);

QColor UBSettings::paletteColor = QColor(127, 127, 127, 127);
QColor UBSettings::opaquePaletteColor = QColor(66, 66, 66, 200);

QColor UBSettings::documentViewLightColor = QColor(241, 241, 241);

QPointer<QSettings> UBSettings::sAppSettings = 0;

const int UBSettings::maxThumbnailWidth = 400;
const int UBSettings::defaultThumbnailWidth = 150;
const int UBSettings::defaultSortKind = 0;
const int UBSettings::defaultSortOrder = 0;
const int UBSettings::defaultSplitterLeftSize = 200;
const int UBSettings::defaultSplitterRightSize = 800;

const int UBSettings::defaultLibraryIconSize = 80;

const int UBSettings::defaultGipWidth = 150;
const int UBSettings::defaultSoundWidth = 50;
const int UBSettings::defaultImageWidth = 150;
const int UBSettings::defaultShapeWidth = 50;
const int UBSettings::defaultWidgetIconWidth = 110;
const int UBSettings::defaultVideoWidth = 80;

const int UBSettings::thumbnailSpacing = 20;
const int UBSettings::longClickInterval = 1200;

const qreal UBSettings::minScreenRatio = 1.33; // 800/600 or 1024/768

QStringList UBSettings::bitmapFileExtensions;
QStringList UBSettings::vectoFileExtensions;
QStringList UBSettings::imageFileExtensions;
QStringList UBSettings::widgetFileExtensions;
QStringList UBSettings::interactiveContentFileExtensions;

QColor UBSettings::treeViewBackgroundColor = QColor(209, 215, 226); //in synch with css tree view background

int UBSettings::objectInControlViewMargin = 100;

QString UBSettings::appPingMessage = "__uniboard_ping";

UBSettings* UBSettings::settings()
{
    if (!sSingleton) {
        sSingleton = new UBSettings(qApp);
    }
    return sSingleton;
}

void UBSettings::destroy()
{
    if (sSingleton)
        delete sSingleton;
    sSingleton = NULL;
}


QSettings* UBSettings::getAppSettings()
{
    if (!UBSettings::sAppSettings)
    {
        QString tmpSettings = QDir::tempPath() + "/" + qApp->applicationName() + ".config";
        QString appSettings = UBPlatformUtils::applicationEtcDirectory() + "/" + qApp->applicationName() + ".config";

        // tmpSettings exists when upgrading Uniboard on Mac (see UBPlatformUtils_mac.mm updater:willInstallUpdate:)
        if (QFile::exists(tmpSettings))
        {
            QFile::rename(tmpSettings, appSettings);
        }

        UBSettings::sAppSettings = new QSettings(appSettings, QSettings::IniFormat, 0);
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        UBSettings::sAppSettings->setIniCodec("utf-8");
#endif

        qDebug() << "sAppSettings location: " << appSettings;
    }

    return UBSettings::sAppSettings;
}


UBSettings::UBSettings(QObject *parent)
    : QObject(parent)
{
    InitKeyboardPaletteKeyBtnSizes();

    mAppSettings = UBSettings::getAppSettings();

    QString userSettingsFile = UBSettings::userDataDirectory() + "/"+qApp->applicationName()+"User.config";

    mUserSettings = new QSettings(userSettingsFile, QSettings::IniFormat, parent);

    init();
}


UBSettings::~UBSettings()
{
    delete mAppSettings;

    if(supportedKeyboardSizes)
        delete supportedKeyboardSizes;
}

void UBSettings::InitKeyboardPaletteKeyBtnSizes()
{
    supportedKeyboardSizes = new QStringList();
    supportedKeyboardSizes->append("29x29");
    supportedKeyboardSizes->append("41x41");
}

void UBSettings::ValidateKeyboardPaletteKeyBtnSize()
{
    // if boardKeyboardPaletteKeyBtnSize is not initialized, or supportedKeyboardSizes not initialized or empty
    if( !boardKeyboardPaletteKeyBtnSize ||
        !supportedKeyboardSizes ||
        supportedKeyboardSizes->size() == 0 ) return;

    // get original size value
    QString origValue = boardKeyboardPaletteKeyBtnSize->get().toString();

    // parse available size values, for make sure original value is valid
    for(int i = 0; i < supportedKeyboardSizes->size(); i++)
    {
        int compareCode = QString::compare(origValue, supportedKeyboardSizes->at(i));
        if(compareCode == 0) return;
    }

    // if original value is invalid, than set it value to first value from avaliable list
    boardKeyboardPaletteKeyBtnSize->set(supportedKeyboardSizes->at(0));
}

void UBSettings::init()
{
    productWebUrl =  new UBSetting(this, "App", "ProductWebAddress", "http://www.openboard.ch");

    softwareHomeUrl = productWebUrl->get().toString();

    tutorialUrl = new UBSetting(this,"App","TutorialUrl","http://www.openboard.ch");

    documentSizes.insert(DocumentSizeRatio::Ratio4_3, QSize(1280, 960)); // 1.33
    documentSizes.insert(DocumentSizeRatio::Ratio16_9, QSize((960 / 9 * 16), 960)); // 1.77

    appToolBarPositionedAtTop = new UBSetting(this, "App", "ToolBarPositionedAtTop", true);
    appToolBarDisplayText = new UBSetting(this, "App", "ToolBarDisplayText", true);
    appEnableAutomaticSoftwareUpdates = new UBSetting(this, "App", "EnableAutomaticSoftwareUpdates", false);
    appSoftwareUpdateURL = new UBSetting(this, "App", "SoftwareUpdateURL", "http://www.openboard.ch/update.json");
    appHideCheckForSoftwareUpdate = new UBSetting(this, "App", "HideCheckForSoftwareUpdate", false);
    appToolBarOrientationVertical = new UBSetting(this, "App", "ToolBarOrientationVertical", false);
    appPreferredLanguage = new UBSetting(this,"App","PreferredLanguage", "");

    // removed in version 1.7.0
    mUserSettings->remove("App/HideSwapDisplayScreens");

    rightLibPaletteBoardModeWidth = new UBSetting(this, "Board", "RightLibPaletteBoardModeWidth", 270);
    rightLibPaletteBoardModeIsCollapsed = new UBSetting(this,"Board", "RightLibPaletteBoardModeIsCollapsed",true);
    rightLibPaletteDesktopModeWidth = new UBSetting(this, "Board", "RightLibPaletteDesktopModeWidth", 270);
    rightLibPaletteDesktopModeIsCollapsed = new UBSetting(this,"Board", "RightLibPaletteDesktopModeIsCollapsed",true);
    leftLibPaletteBoardModeWidth = new UBSetting(this, "Board", "LeftLibPaletteBoardModeWidth",270);
    leftLibPaletteBoardModeIsCollapsed = new UBSetting(this,"Board","LeftLibPaletteBoardModeIsCollapsed",true);
    leftLibPaletteDesktopModeWidth = new UBSetting(this, "Board", "LeftLibPaletteDesktopModeWidth",270);
    leftLibPaletteDesktopModeIsCollapsed = new UBSetting(this,"Board","LeftLibPaletteDesktopModeIsCollapsed",true);

    appIsInSoftwareUpdateProcess = new UBSetting(this, "App", "IsInSoftwareUpdateProcess", false);
    appLastSessionDocumentUUID = new UBSetting(this, "App", "LastSessionDocumentUUID", "");
    appLastSessionPageIndex = new UBSetting(this, "App", "LastSessionPageIndex", 0);
    appUseMultiscreen = new UBSetting(this, "App", "UseMultiscreenMode", true);
    appScreenList = new UBSetting(this, "App", "ScreenList", QVariant());

    appStartupHintsEnabled = new UBSetting(this,"App","EnableStartupHints",false);

    appStartMode = new UBSetting(this, "App", "StartMode", "");
    appRunInWindow = new UBSetting(this, "App", "RunInWindow", false);

    featureSliderPosition = new UBSetting(this, "Board", "FeatureSliderPosition", 40);

    boardPenFineWidth = new UBSetting(this, "Board", "PenFineWidth", 1.5);
    boardPenMediumWidth = new UBSetting(this, "Board", "PenMediumWidth", 3.0);
    boardPenStrongWidth = new UBSetting(this, "Board", "PenStrongWidth", 8.0);

    boardMarkerFineWidth = new UBSetting(this, "Board", "MarkerFineWidth", 12.0);
    boardMarkerMediumWidth = new UBSetting(this, "Board", "MarkerMediumWidth", 24.0);
    boardMarkerStrongWidth = new UBSetting(this, "Board", "MarkerStrongWidth", 48.0);

    boardPenPressureSensitive = new UBSetting(this, "Board", "PenPressureSensitive", true);
    boardMarkerPressureSensitive = new UBSetting(this, "Board", "MarkerPressureSensitive", false);

    boardUseHighResTabletEvent = new UBSetting(this, "Board", "UseHighResTabletEvent", true);

    boardInterpolatePenStrokes = new UBSetting(this, "Board", "InterpolatePenStrokes", true);
    boardSimplifyPenStrokes = new UBSetting(this, "Board", "SimplifyPenStrokes", true);
    boardSimplifyPenStrokesThresholdAngle = new UBSetting(this, "Board", "SimplifyPenStrokesThresholdAngle", 2);
    boardSimplifyPenStrokesThresholdWidthDifference = new UBSetting(this, "Board", "SimplifyPenStrokesThresholdWidthDifference", 2.0);

    boardInterpolateMarkerStrokes = new UBSetting(this, "Board", "InterpolateMarkerStrokes", true);
    boardSimplifyMarkerStrokes = new UBSetting(this, "Board", "SimplifyMarkerStrokes", true);

    boardKeyboardPaletteKeyBtnSize = new UBSetting(this, "Board", "KeyboardPaletteKeyBtnSize", "16x16");
    ValidateKeyboardPaletteKeyBtnSize();

    pageSize = new UBSetting(this, "Board", "DefaultPageSize", documentSizes.value(DocumentSizeRatio::Ratio4_3));

    boardCrossColorDarkBackground = new UBSetting(this, "Board", "CrossColorDarkBackground", "#C8C0C0C0");
    boardCrossColorLightBackground = new UBSetting(this, "Board", "CrossColorLightBackground", "#A5E1FF");

    QStringList gridLightBackgroundColors;
    gridLightBackgroundColors << "#000000" << "#FF0000" << "#004080" << "#008000" << "#FFDD00" << "#C87400" << "#800040" << "#008080" << "#A5E1FF";
    boardGridLightBackgroundColors = new UBColorListSetting(this, "Board", "GridLightBackgroundColors", gridLightBackgroundColors, -1.0);

    QStringList gridDarkBackgroundColors;
    gridDarkBackgroundColors << "#FFFFFF" << "#FF3400" << "#66C0FF" << "#81FF5C" << "#FFFF00" << "#B68360" << "#FF497E" << "#8D69FF" << "#C8C0C0C0";
    boardGridDarkBackgroundColors = new UBColorListSetting(this, "Board", "GridDarkBackgroundColors", gridDarkBackgroundColors, -1.0);

    QStringList penLightBackgroundColors;
    penLightBackgroundColors << "#000000" << "#FF0000" <<"#004080" << "#008000" << "#FFDD00" << "#C87400" << "#800040" << "#008080"  << "#5F2D0A" << "#FFFFFF";
    boardPenLightBackgroundColors = new UBColorListSetting(this, "Board", "PenLightBackgroundColors", penLightBackgroundColors, 1.0);

    QStringList penDarkBackgroundColors;
    penDarkBackgroundColors << "#FFFFFF" << "#FF3400" <<"#66C0FF" << "#81FF5C" << "#FFFF00" << "#B68360" << "#FF497E" << "#8D69FF" << "#000000";
    boardPenDarkBackgroundColors = new UBColorListSetting(this, "Board", "PenDarkBackgroundColors", penDarkBackgroundColors, 1.0);

    boardMarkerAlpha = new UBSetting(this, "Board", "MarkerAlpha", 0.5);

    QStringList markerLightBackgroundColors;
    markerLightBackgroundColors << "#E3FF00" << "#FF0000" <<"#004080" << "#008000" << "#C87400" << "#800040" << "#008080"  << "#000000";
    boardMarkerLightBackgroundColors = new UBColorListSetting(this, "Board", "MarkerLightBackgroundColors", markerLightBackgroundColors, boardMarkerAlpha->get().toDouble());

    QStringList markerDarkBackgroundColors;
    markerDarkBackgroundColors << "#FFFF00" << "#FF4400" <<"#66C0FF" << "#81FF5C" << "#B68360" << "#FF497E" << "#8D69FF" << "#FFFFFF";
    boardMarkerDarkBackgroundColors = new UBColorListSetting(this, "Board", "MarkerDarkBackgroundColors", markerDarkBackgroundColors, boardMarkerAlpha->get().toDouble());

    QStringList penLightBackgroundSelectedColors;
    QStringList penDarkBackgroundSelectedColors;

    QStringList markerLightBackgroundSelectedColors;
    QStringList markerDarkBackgroundSelectedColors;

    for (int i = 0; i < colorPaletteSize; i++)
    {
        penLightBackgroundSelectedColors << penLightBackgroundColors[i];
        penDarkBackgroundSelectedColors << penDarkBackgroundColors[i];
        markerLightBackgroundSelectedColors << markerLightBackgroundColors[i];
        markerDarkBackgroundSelectedColors << markerDarkBackgroundColors[i];
    }

    boardPenLightBackgroundSelectedColors = new UBColorListSetting(this, "Board", "PenLightBackgroundSelectedColors", penLightBackgroundSelectedColors);
    boardPenDarkBackgroundSelectedColors = new UBColorListSetting(this, "Board", "PenDarkBackgroundSelectedColors", penDarkBackgroundSelectedColors);

    boardMarkerLightBackgroundSelectedColors = new UBColorListSetting(this, "Board", "MarkerLightBackgroundSelectedColors", markerLightBackgroundSelectedColors, boardMarkerAlpha->get().toDouble());
    boardMarkerDarkBackgroundSelectedColors = new UBColorListSetting(this, "Board", "MarkerDarkBackgroundSelectedColors", markerDarkBackgroundSelectedColors, boardMarkerAlpha->get().toDouble());

    showEraserPreviewCircle = new UBSetting(this, "Board", "ShowEraserPreviewCircle", true);
    showMarkerPreviewCircle = new UBSetting(this, "Board", "ShowMarkerPreviewCircle", true);
    showPenPreviewCircle = new UBSetting(this, "Board", "ShowPenPreviewCircle", true);
    penPreviewFromSize = new UBSetting(this, "Board", "PenPreviewFromSize", 5);

    webUseExternalBrowser = new UBSetting(this, "Web", "UseExternalBrowser", false);

    bool defaultShowPageImmediatelyOnMirroredScreen = true;

#if defined(Q_OS_LINUX)
    // screen duplication is very slow on X11
    defaultShowPageImmediatelyOnMirroredScreen = false;
#endif

    webShowPageImmediatelyOnMirroredScreen = new UBSetting(this, "Web", "ShowPageImediatelyOnMirroredScreen", defaultShowPageImmediatelyOnMirroredScreen);

    webHomePage = new UBSetting(this, "Web", "Homepage", softwareHomeUrl);
    webSearchEngineUrl = new UBSetting(this, "Web", "SearchEngineUrl", "https://www.qwant.com/?q=%1");
    alternativeUserAgent = new UBSetting(this, "Web", "AlternativeUserAgent", "Mozilla/5.0 (%1; %2; rv:91.0) Gecko/20100101 Firefox/91.0");
    alternativeUserAgentDomains = new UBSetting(this, "Web", "AlternativeUserAgentDomains", "google.*");
    webCookieAutoDelete = new UBSetting(this, "Web", "CookieAutoDelete", false);
    webCookieKeepDomains = new UBSetting(this, "Web", "CookieKeepDomains", QStringList());
    webCookiePolicy = new UBSetting(this, "Web", "CookiePolicy", "DenyThirdParty");
    webPrivateBrowsing = new UBSetting(this, "Web", "PrivateBrowsing", false);

    pageCacheSize = new UBSetting(this, "App", "PageCacheSize", 20);

    bitmapFileExtensions << "jpg" << "jpeg" <<  "png" <<  "tiff" << "tif" << "bmp" << "gif";
    vectoFileExtensions << "svg" <<  "svgz";
    imageFileExtensions << bitmapFileExtensions << vectoFileExtensions;

    widgetFileExtensions << "wdgt" << "wgt" << "pwgt";
    interactiveContentFileExtensions << widgetFileExtensions << "swf";

    boardZoomBase = new UBSetting(this, "Board", "ZoomBase", 1.0005);

    if (boardZoomBase->get().toDouble() <= 1. || boardZoomBase->get().toDouble() > 1.01)
        boardZoomBase->set(1.0005);

    boardZoomFactor = new UBSetting(this, "Board", "ZoomFactor", QVariant(1.41));

    if (boardZoomFactor->get().toDouble() <= 1.)
        boardZoomFactor->set(1.41);

    int defaultRefreshRateInFramePerSecond = 8;

#if defined(Q_OS_LINUX)
    // screen duplication is very slow on X11
    defaultRefreshRateInFramePerSecond = 2;
#endif

    mirroringRefreshRateInFps = new UBSetting(this, "Mirroring", "RefreshRateInFramePerSecond", QVariant(defaultRefreshRateInFramePerSecond));

    lastImportFilePath = new UBSetting(this, "Import", "LastImportFilePath", QVariant(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    lastImportFolderPath = new UBSetting(this, "Import", "LastImportFolderPath", QVariant(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    lastExportFilePath = new UBSetting(this, "Export", "LastExportFilePath", QVariant(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    lastExportDirPath = new UBSetting(this, "Export", "LastExportDirPath", QVariant(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    lastImportToLibraryPath = new UBSetting(this, "Library", "LastImportToLibraryPath", QVariant(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));

    lastPicturePath = new UBSetting(this, "Library", "LastPicturePath", QVariant(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)));
    lastWidgetPath = new UBSetting(this, "Library", "LastWidgetPath", QVariant(QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)));
    lastVideoPath = new UBSetting(this, "Library", "LastVideoPath", QVariant(QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)));

    boardShowToolsPalette = new UBSetting(this, "Board", "ShowToolsPalette", "false");
    magnifierDrawingMode = new UBSetting(this, "Board", "MagnifierDrawingMode", "0");
    autoSaveInterval = new UBSetting(this, "Board", "AutoSaveIntervalInMinutes", "3");

    svgViewBoxMargin = new UBSetting(this, "SVG", "ViewBoxMargin", "50");

    pdfMargin = new UBSetting(this, "PDF", "Margin", "20");
    pdfPageFormat = new UBSetting(this, "PDF", "PageFormat", "A4");
    pdfUsePDFMerger = new UBSetting(this, "PDF", "UsePDFMerger", "true");
    pdfResolution = new UBSetting(this, "PDF", "Resolution", "300");

    exportBackgroundGrid = new UBSetting(this, "PDF", "ExportBackgroundGrid", false);
    exportBackgroundColor = new UBSetting(this, "PDF", "ExportBackgroundColor", false);

    podcastFramesPerSecond = new UBSetting(this, "Podcast", "FramesPerSecond", 10);
    podcastVideoSize = new UBSetting(this, "Podcast", "VideoSize", "Medium");
    podcastAudioRecordingDevice = new UBSetting(this, "Podcast", "AudioRecordingDevice", "Default");

    podcastWindowsMediaBitsPerSecond = new UBSetting(this, "Podcast", "WindowsMediaBitsPerSecond", 1700000);
    podcastQuickTimeQuality = new UBSetting(this, "Podcast", "QuickTimeQuality", "High");

    podcastPublishToYoutube = new UBSetting(this, "Podcast", "PublishToYouTube", false);
    youTubeUserEMail = new UBSetting(this, "YouTube", "UserEMail", "");
    youTubeCredentialsPersistence = new UBSetting(this,"YouTube", "CredentialsPersistence",false);

    communityUser = new UBSetting(this, "Community", "Username", "");
    communityPsw = new UBSetting(this, "Community", "Password", "");
    communityCredentialsPersistence = new UBSetting(this,"Community", "CredentialsPersistence",false);

    QStringList uris = UBToolsManager::manager()->allToolIDs();

    favoritesNativeToolUris = new UBSetting(this, "App", "FavoriteToolURIs", uris);

    // removed in version 4.4.b.2
    mUserSettings->remove("Podcast/RecordMicrophone");

    documentThumbnailWidth      = new UBSetting(this, "Document", "ThumbnailWidth", UBSettings::defaultThumbnailWidth);
    documentSortKind            = new UBSetting(this, "Document", "SortKind", UBSettings::defaultSortKind);
    documentSortOrder           = new UBSetting(this, "Document", "SortOrder", UBSettings::defaultSortOrder);
    documentSplitterLeftSize    = new UBSetting(this, "Document", "SplitterLeftSize", UBSettings::defaultSplitterLeftSize);
    documentSplitterRightSize   = new UBSetting(this, "Document", "SplitterRightSize", UBSettings::defaultSplitterRightSize);

    libraryShowDetailsForLocalItems = new UBSetting(this, "Library", "ShowDetailsForLocalItems", false);

    imageThumbnailWidth = new UBSetting(this, "Library", "ImageThumbnailWidth", UBSettings::defaultImageWidth);
    videoThumbnailWidth = new UBSetting(this, "Library", "VideoThumbnailWidth", UBSettings::defaultVideoWidth);
    shapeThumbnailWidth = new UBSetting(this, "Library", "ShapeThumbnailWidth", UBSettings::defaultShapeWidth);
    gipThumbnailWidth = new UBSetting(this, "Library", "ImageThumbnailWidth", UBSettings::defaultGipWidth);
    soundThumbnailWidth = new UBSetting(this, "Library", "SoundThumbnailWidth", UBSettings::defaultSoundWidth);;

    podcastPublishToIntranet = new UBSetting(this, "IntranetPodcast", "PublishToIntranet", false);
    intranetPodcastPublishingUrl = new UBSetting(this, "IntranetPodcast", "PublishingUrl", "");
    intranetPodcastAuthor = new UBSetting(this, "IntranetPodcast", "Author", "");

    KeyboardLocale = new UBSetting(this, "Board", "StartupKeyboardLocale", 0);
    swapControlAndDisplayScreens = new UBSetting(this, "App", "SwapControlAndDisplayScreens", false);

    rotationAngleStep = new UBSetting(this, "App", "RotationAngleStep", 5.);
    historyLimit = new UBSetting(this, "Web", "HistoryLimit", 15);

    libIconSize = new UBSetting(this, "Library", "LibIconSize", defaultLibraryIconSize);

    useSystemOnScreenKeyboard = new UBSetting(this, "App", "UseSystemOnScreenKeyboard", true);

    if (!UBPlatformUtils::hasSystemOnScreenKeyboard())
    {
        useSystemOnScreenKeyboard->set(false);
    }

    showDateColumnOnAlphabeticalSort = new UBSetting(this, "Document", "ShowDateColumnOnAlphabeticalSort", false);
    emptyTrashForOlderDocuments = new UBSetting(this, "Document", "emptyTrashForOlderDocuments", false);
    emptyTrashDaysValue = new UBSetting(this, "Document", "emptyTrashDaysValue", 30);

    pointerDiameter = value("Board/PointerDiameter", pointerDiameter).toInt();

    cleanNonPersistentSettings();
    checkNewSettings();
}


/**
 * @brief Returns the value for the *key* setting, or *defaultValue* if the key doesn't exist
 *
 * The value is also added to the local settings queue, to prevent future disk I/O when accessing
 * that same setting.
 *
 * If the value doesn't exist in the application settings (i.e it was not present in the config file),
 * it is also added there.
 */
QVariant UBSettings::value ( const QString & key, const QVariant & defaultValue)
{
    // Check first the settings queue, then the app settings, then the user settings.
    // If the key exists in neither of these, then defaultValue is returned.

    if (mSettingsQueue.contains(key))
        return mSettingsQueue.value(key);

    // If the setting doesn't exist in the App settings, add it there
    if (!sAppSettings->contains(key) && !(defaultValue == QVariant()))
        sAppSettings->setValue(key, defaultValue);

    // Get the value from the user settings (or if it doesn't exist there, from the app settings)
    QVariant val = mUserSettings->value(key, sAppSettings->value(key, defaultValue));

    // Add the value to the settings queue for faster access next time it is requested
    mSettingsQueue[key] = val;

    return val;
}


void UBSettings::setValue (const QString & key, const QVariant & value)
{
    // Save the setting to the queue only; a call to save() is necessary to persist the settings
    mSettingsQueue[key] = value;
}

/**
 * @brief Save all the queued settings to disk
 */
void UBSettings::save()
{
    QHash<QString, QVariant>::const_iterator it = mSettingsQueue.constBegin();

    while (it != mSettingsQueue.constEnd()) {
        /*
         * We save the setting to the user settings if
         * a) it is different from the (non-null) value stored in the user settings, or
         * b) it doesn't currently exist in the user settings AND has changed from the app settings
         * An invalid value indicates removal of the setting
        */
        if (mUserSettings->contains(it.key())
                && it.value() != mUserSettings->value(it.key()))
        {
            if (it.value().isValid())
            {
                mUserSettings->setValue(it.key(), it.value());
            }
            else
            {
                mUserSettings->remove(it.key());
            }
        }

        else if (!mUserSettings->contains(it.key())
                 && it.value() != mAppSettings->value(it.key()))
        {
            if (it.value().isValid())
            {
                mUserSettings->setValue(it.key(), it.value());
            }
            else
            {
                mUserSettings->remove(it.key());
            }
        }

        ++it;
    }

    // Force save to file
    mUserSettings->sync();

    qDebug() << "User settings saved";
}

int UBSettings::penWidthIndex()
{
    return value("Board/PenLineWidthIndex", 0).toInt();
}


void UBSettings::setPenWidthIndex(int index)
{
    if (index != penWidthIndex())
    {
        setValue("Board/PenLineWidthIndex", index);
    }
}


qreal UBSettings::currentPenWidth()
{
    qreal width = 0;

    switch (penWidthIndex())
    {
        case UBWidth::Fine:
            width = boardPenFineWidth->get().toDouble();
            break;
        case UBWidth::Medium:
            width = boardPenMediumWidth->get().toDouble();
            break;
        case UBWidth::Strong:
            width = boardPenStrongWidth->get().toDouble();
            break;
        default:
            Q_ASSERT(false);
            //failsafe
            width = boardPenFineWidth->get().toDouble();
            break;
    }

    return width;
}


int UBSettings::penColorIndex()
{
    return value("Board/PenColorIndex", 0).toInt();
}


void UBSettings::setPenColorIndex(int index)
{
    if (index != penColorIndex())
    {
        setValue("Board/PenColorIndex", index);
    }
}


QColor UBSettings::currentPenColor()
{
    return penColor(isDarkBackground());
}


QColor UBSettings::penColor(bool onDarkBackground)
{
    QList<QColor> colors = penColors(onDarkBackground);
    return colors.at(penColorIndex());
}


QList<QColor> UBSettings::penColors(bool onDarkBackground)
{
    if (onDarkBackground)
    {
        return boardPenDarkBackgroundSelectedColors->colors();
    }
    else
    {
        return boardPenLightBackgroundSelectedColors->colors();
    }
}


int UBSettings::markerWidthIndex()
{
    return value("Board/MarkerLineWidthIndex", 0).toInt();
}


void UBSettings::setMarkerWidthIndex(int index)
{
    if (index != markerWidthIndex())
    {
        setValue("Board/MarkerLineWidthIndex", index);
    }
}


qreal UBSettings::currentMarkerWidth()
{
    qreal width = 0;

    switch (markerWidthIndex())
    {
        case UBWidth::Fine:
            width = boardMarkerFineWidth->get().toDouble();
            break;
        case UBWidth::Medium:
            width = boardMarkerMediumWidth->get().toDouble();
            break;
        case UBWidth::Strong:
            width = boardMarkerStrongWidth->get().toDouble();
            break;
        default:
            Q_ASSERT(false);
            //failsafe
            width = boardMarkerFineWidth->get().toDouble();
            break;
    }

    return width;
}


int UBSettings::markerColorIndex()
{
    return value("Board/MarkerColorIndex", 0).toInt();
}


void UBSettings::setMarkerColorIndex(int index)
{
    if (index != markerColorIndex())
    {
        setValue("Board/MarkerColorIndex", index);
    }
}


QColor UBSettings::currentMarkerColor()
{
    return markerColor(isDarkBackground());
}


QColor UBSettings::markerColor(bool onDarkBackground)
{
    QList<QColor> colors = markerColors(onDarkBackground);
    return colors.at(markerColorIndex());
}


QList<QColor> UBSettings::markerColors(bool onDarkBackground)
{
    if (onDarkBackground)
    {
        return boardMarkerDarkBackgroundSelectedColors->colors();
    }
    else
    {
        return boardMarkerLightBackgroundSelectedColors->colors();
    }
}

//----------------------------------------//
// eraser

int UBSettings::eraserWidthIndex()
{
    return value("Board/EraserCircleWidthIndex", 1).toInt();
}

void UBSettings::setEraserWidthIndex(int index)
{
    setValue("Board/EraserCircleWidthIndex", index);
}

qreal UBSettings::eraserFineWidth()
{
    return value("Board/EraserFineWidth", 16).toDouble();
}

void UBSettings::setEraserFineWidth(qreal width)
{
    setValue("Board/EraserFineWidth", width);
}

qreal UBSettings::eraserMediumWidth()
{
    return value("Board/EraserMediumWidth", 64).toDouble();
}

void UBSettings::setEraserMediumWidth(qreal width)
{
    setValue("Board/EraserMediumWidth", width);
}

qreal UBSettings::eraserStrongWidth()
{
    return value("Board/EraserStrongWidth", 128).toDouble();
}

void UBSettings::setEraserStrongWidth(qreal width)
{
    setValue("Board/EraserStrongWidth", width);
}

qreal UBSettings::currentEraserWidth()
{
    qreal width = 0;

    switch (eraserWidthIndex())
    {
        case UBWidth::Fine:
            width = eraserFineWidth();
            break;
        case UBWidth::Medium:
            width = eraserMediumWidth();
            break;
        case UBWidth::Strong:
            width = eraserStrongWidth();
            break;
        default:
            Q_ASSERT(false);
            //failsafe
            width = eraserFineWidth();
            break;
    }

    return width;
}

bool UBSettings::isDarkBackground()
{
    return value("Board/DarkBackground", 0).toBool();
}


QUuid UBSettings::pageBackgroundUuid()
{
    QUuid uuid = value("Board/PageBackground", 0).toUuid();
    return uuid;
}

void UBSettings::setDarkBackground(bool isDarkBackground)
{
    setValue("Board/DarkBackground", isDarkBackground);
    emit colorContextChanged();
}


void UBSettings::setPageBackgroundUuid(const QUuid& background)
{
    setValue("Board/PageBackground", background.toString());
}

void UBSettings::setPenPressureSensitive(bool sensitive)
{
    boardPenPressureSensitive->set(sensitive);
}

void UBSettings::setPenPreviewCircle(bool circle)
{
    showPenPreviewCircle->set(circle);
}

void UBSettings::setPenPreviewFromSize(int size)
{
    penPreviewFromSize->set(size);
}

void UBSettings::setMarkerPressureSensitive(bool sensitive)
{
    boardMarkerPressureSensitive->set(sensitive);
}


bool UBSettings::isStylusPaletteVisible()
{
    return value("Board/StylusPaletteIsVisible", true).toBool();
}


void UBSettings::setStylusPaletteVisible(bool visible)
{
    setValue("Board/StylusPaletteIsVisible", visible);
}


QString UBSettings::fontFamily()
{
    return value("Board/FontFamily", sDefaultFontFamily).toString();
}


void UBSettings::setFontFamily(const QString &family)
{
    setValue("Board/FontFamily", family);
}


QString UBSettings::fontStyleName()
{
    return value("Board/FontStyleName", sDefaultFontStyleName).toString();
}


void UBSettings::setFontStyleName(const QString &styleName)
{
    setValue("Board/FontStyleName", styleName);
}

int UBSettings::fontPixelSize()
{
    return value("Board/FontPixelSize", sDefaultFontPixelSize).toInt();
}


void UBSettings::setFontPixelSize(int pixelSize)
{
    setValue("Board/FontPixelSize", pixelSize);
}

int UBSettings::fontPointSize()
{
    return value("Board/FontPointSize", 12).toInt();
}

void UBSettings::setFontPointSize(int pointSize)
{
    setValue("Board/FontPointSize", pointSize);
}

bool UBSettings::isBoldFont()
{
    return value("Board/FontIsBold", false).toBool();
}


void UBSettings::setBoldFont(bool bold)
{
    setValue("Board/FontIsBold", bold);
}


bool UBSettings::isItalicFont()
{
    return value("Board/FontIsItalic", false).toBool();
}


void UBSettings::setItalicFont(bool italic)
{
    setValue("Board/FontIsItalic", italic);
}


QString UBSettings::userDataDirectory()
{
    static QString dataDirPath = "";
    if(dataDirPath.isEmpty()){
        if (getAppSettings() && getAppSettings()->contains("App/DataDirectory")) {
            qDebug() << "getAppSettings()->contains(App/DataDirectory):" << getAppSettings()->contains("App/DataDirectory");
            dataDirPath = getAppSettings()->value("App/DataDirectory").toString();
            dataDirPath = replaceWildcard(dataDirPath);

            if(checkDirectory(dataDirPath))
                return dataDirPath;
            else
                qCritical() << "Impossible to create datadirpath " << dataDirPath;

        }
        dataDirPath = UBFileSystemUtils::normalizeFilePath(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
        if (qApp->organizationName().size() > 0)
            dataDirPath.replace(qApp->organizationName() + "/", "");
    }
    return dataDirPath;
}


QString UBSettings::userImageDirectory()
{
    static QString imageDirectory = "";
    if(imageDirectory.isEmpty()){
        if (sAppSettings && getAppSettings()->contains("App/UserImageDirectory")) {
            imageDirectory = getAppSettings()->value("App/UserImageDirectory").toString();

            imageDirectory = replaceWildcard(imageDirectory);
            if(checkDirectory(imageDirectory))
                return imageDirectory;
            else
                qCritical() << "failed to create image directory " << imageDirectory;
        }

        imageDirectory = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation) + "/" + qApp->applicationName();
        checkDirectory(imageDirectory);
    }
    return imageDirectory;
}


QString UBSettings::userVideoDirectory()
{
    static QString videoDirectory = "";
    if(videoDirectory.isEmpty()){
        if (sAppSettings && getAppSettings()->contains("App/UserVideoDirectory")) {
            videoDirectory = getAppSettings()->value("App/UserVideoDirectory").toString();
            videoDirectory = replaceWildcard(videoDirectory);
            if(checkDirectory(videoDirectory))
                return videoDirectory;
            else
                qCritical() << "failed to create video directory " << videoDirectory;
        }


        videoDirectory = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);

        if(videoDirectory.isEmpty())
            videoDirectory = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/" + tr("My Movies");
        else
            videoDirectory = videoDirectory + "/" + qApp->applicationName();

        checkDirectory(videoDirectory);
    }
    return videoDirectory;
}

QString UBSettings::userAudioDirectory()
{
    static QString audioDirectory = "";
    if(audioDirectory.isEmpty()){
        if (sAppSettings && getAppSettings()->contains("App/UserAudioDirectory")) {
            audioDirectory = getAppSettings()->value("App/UserAudioDirectory").toString();

            audioDirectory = replaceWildcard(audioDirectory);
            if(checkDirectory(audioDirectory))
                return audioDirectory;
            else
                qCritical() << "failed to create image directory " << audioDirectory;
        }

        audioDirectory = QStandardPaths::writableLocation(QStandardPaths::MusicLocation) + "/" + qApp->applicationName();
        checkDirectory(audioDirectory);
    }
    return audioDirectory;
}


QString UBSettings::userPodcastRecordingDirectory()
{
    static QString dirPath = "";
    if(dirPath.isEmpty()){
        if (sAppSettings && getAppSettings()->contains("Podcast/RecordingDirectory"))
        {
            dirPath = getAppSettings()->value("Podcast/RecordingDirectory").toString();
            dirPath = replaceWildcard(dirPath);
            if(checkDirectory(dirPath))
                return dirPath;
            else
                qCritical() << "failed to create dir " << dirPath;

        }
        dirPath = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
        checkDirectory(dirPath);
    }
    return dirPath;
}

QString UBSettings::userDocumentDirectory()
{
    static QString documentDirectory = "";
    if(documentDirectory.isEmpty()){
        documentDirectory = userDataDirectory() + "/document";
        checkDirectory(documentDirectory);
    }
    qDebug() << "userDocumentDirectory()" << documentDirectory;
    return documentDirectory;
}

QString UBSettings::userFavoriteListFilePath()
{
    static QString filePath = "";
    if(filePath.isEmpty()){
        QString dirPath = userDataDirectory() + "/libraryPalette";
        checkDirectory(dirPath);
        filePath = dirPath + "/favorite.dat";
    }
    return filePath;
}

QString UBSettings::userTrashDirPath()
{
    static QString trashPath = "";
    if(trashPath.isEmpty()){
        trashPath = userDataDirectory() + "/libraryPalette/trash";
        checkDirectory(trashPath);
    }
    return trashPath;
}


QString UBSettings::userGipLibraryDirectory()
{
    static QString dirPath = "";
    if(dirPath.isEmpty()){
        dirPath = userDataDirectory() + "/library/gips";
        checkDirectory(dirPath);
    }
    return dirPath;
}


QString UBSettings::applicationShapeLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/shape");

    QString configPath = value("Library/ShapeDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith(".")) {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else {
        return configPath;
    }
}

QString UBSettings::applicationCustomizationDirectory()
{
    QString defaultRelativePath = QString("/customizations");
    return UBPlatformUtils::applicationResourcesDirectory() + defaultRelativePath;

}

QString UBSettings::applicationCustomFontDirectory()
{
    QString defaultFontDirectory = "/fonts";
    return applicationCustomizationDirectory() + defaultFontDirectory;
}

QString UBSettings::userSearchDirectory()
{
    static QString dirPath = "";
    if(dirPath.isEmpty()){
        dirPath = UBPlatformUtils::applicationResourcesDirectory() + "/library/search";
        checkDirectory(dirPath);
    }
    return dirPath;
}

QString UBSettings::applicationImageLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/pictures");

    QString configPath = value("Library/ImageDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith(".")) {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else {
        return configPath;
    }
}

QString UBSettings::userAnimationDirectory()
{
    static QString animationDirectory = "";
    if(animationDirectory.isEmpty()){
        animationDirectory = userDataDirectory() + "/animationUserDirectory";
        checkDirectory(animationDirectory);
    }
    return animationDirectory;
}

QString UBSettings::userInteractiveDirectory()
{
    static QString interactiveDirectory = "";
    if(interactiveDirectory.isEmpty()){
        if (sAppSettings && getAppSettings()->contains("App/UserInteractiveContentDirectory")) {
            interactiveDirectory = getAppSettings()->value("App/UserInteractiveContentDirectory").toString();
            interactiveDirectory = replaceWildcard(interactiveDirectory);
            if(checkDirectory(interactiveDirectory))
                return interactiveDirectory;
            else
                qCritical() << "failed to create directory " << interactiveDirectory;
        }
        interactiveDirectory = userDataDirectory() + "/interactive content";
        checkDirectory(interactiveDirectory);
    }
    return interactiveDirectory;
}


QString UBSettings::applicationInteractivesDirectory()
{
    QString defaultRelativePath = QString("./library/interactivities");

    QString configPath = value("Library/InteractivitiesDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith(".")) {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else {
        return configPath;
    }
}

QString UBSettings::applicationApplicationsLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/applications");

    QString configPath = value("Library/ApplicationsDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith(".")) {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else {
        return configPath;
    }
}


QString UBSettings::applicationAudiosLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/audios");

    QString configPath = value("Library/AudiosDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith(".")) {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else {
        return configPath;
    }
}

QString UBSettings::applicationVideosLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/videos");

    QString configPath = value("Library/VideosDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith(".")) {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else {
        return configPath;
    }
}

QString UBSettings::applicationAnimationsLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/animations");

    QString configPath = value("Library/AnimationsDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith(".")) {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else {
        return configPath;
    }
}

QString UBSettings::applicationStartupHintsDirectory()
{
    QString defaultRelativePath = QString("./startupHints");

    QString configPath = value("StartupHintsDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith(".")) {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else
        return configPath;
}

QString UBSettings::userInteractiveFavoritesDirectory()
{
    static QString dirPath = "";
    if(dirPath.isEmpty()){
        if (sAppSettings && getAppSettings()->contains("App/UserInteractiveFavoritesDirectory")) {
            dirPath = getAppSettings()->value("App/UserInteractiveFavoritesDirectory").toString();
            dirPath = replaceWildcard(dirPath);
            if(checkDirectory(dirPath))
                return dirPath;
            else
                qCritical() << "failed to create directory " << dirPath;
        }

        dirPath = userDataDirectory() + "/interactive favorites";
        checkDirectory(dirPath);
    }
    return dirPath;
}


QNetworkProxy* UBSettings::httpProxy()
{
    QNetworkProxy* proxy = 0;

    if (mAppSettings->value("Proxy/Enabled", false).toBool()) {

        proxy = new QNetworkProxy();

        if (mAppSettings->value("Proxy/Type", "HTTP").toString() == "Socks5")
            proxy->setType(QNetworkProxy::Socks5Proxy);
        else
            proxy->setType(QNetworkProxy::HttpProxy);

        proxy->setHostName(mAppSettings->value("Proxy/HostName").toString());
        proxy->setPort(mAppSettings->value("Proxy/Port", 1080).toInt());
        proxy->setUser(mAppSettings->value("Proxy/UserName").toString());
        proxy->setPassword(mAppSettings->value("Proxy/Password").toString());
    }

    return proxy;
}


void UBSettings::setPassword(const QString& id, const QString& password)
{
    QString encrypted = UBCryptoUtils::instance()->symetricEncrypt(password);

    mUserSettings->setValue(QString("Vault/") + id, encrypted);
}


void UBSettings::removePassword(const QString& id)
{
    mUserSettings->remove(QString("Vault/") + id);
}


QString UBSettings::password(const QString& id)
{
    QString encrypted = mUserSettings->value(QString("Vault/") + id).toString();

    QString result = "";

    if (encrypted.length() > 0)
        result =  UBCryptoUtils::instance()->symetricDecrypt(encrypted);

    return result;
}


QString UBSettings::proxyUsername()
{
    QString idUsername = "http.proxy.user";
    return password(idUsername);
}


void UBSettings::setProxyUsername(const QString& username)
{
    QString idUsername = "http.proxy.user";

    if (username.length() > 0)
        setPassword(idUsername, username);
    else
        removePassword(idUsername);
}


QString UBSettings::proxyPassword()
{
    QString idPassword = "http.proxy.pass";
    return password(idPassword);
}


void UBSettings::setProxyPassword(const QString& password)
{
    QString idPassword = "http.proxy.pass";

    if (password.length() > 0)
       setPassword(idPassword, password);
    else
        removePassword(idPassword);

}

QString UBSettings::communityUsername()
{
    return communityUser->get().toString();
}

void UBSettings::setCommunityUsername(const QString &username)
{
    communityUser->set(QVariant(username));
}

QString UBSettings::communityPassword()
{
    return communityPsw->get().toString();
}

void UBSettings::setCommunityPassword(const QString &password)
{
    communityPsw->set(QVariant(password));
}

void UBSettings::setCommunityPersistence(const bool persistence)
{
    communityCredentialsPersistence->set(QVariant(persistence));
}

int UBSettings::libraryIconSize(){
    return libIconSize->get().toInt();
}

void UBSettings::setLibraryIconsize(const int& size){
    libIconSize->set(QVariant(size));
}

bool UBSettings::checkDirectory(QString& dirPath)
{
    bool result = true;
    QDir dir(dirPath);
    if(!dir.exists())
        result = dir.mkpath(dirPath);
    return result;
}

QString UBSettings::replaceWildcard(QString& path)
{
    QString result(path);

    if (result.startsWith("{Documents}")) {
        result = result.replace("{Documents}", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    }
    else if(result.startsWith("{Home}")) {
        result = result.replace("{Home}", QStandardPaths::writableLocation(QStandardPaths::HomeLocation));
    }
    else if(result.startsWith("{Desktop}")) {
        result = result.replace("{Desktop}", QStandardPaths::writableLocation(QStandardPaths::DesktopLocation));
    }

    if(result.contains("{UserLoginName}") && UBPlatformUtils::osUserLoginName().length() > 0) {
        result = result.replace("{UserLoginName}", UBPlatformUtils::osUserLoginName());
    }

    return result;
}

void UBSettings::closing()
{
    save();
    cleanNonPersistentSettings();
}

void UBSettings::cleanNonPersistentSettings()
{
    if(!communityCredentialsPersistence->get().toBool()){
        communityPsw->set(QVariant(""));
        communityUser->set(QVariant(""));
    }

    if(!youTubeCredentialsPersistence->get().toBool()){
        removePassword(youTubeUserEMail->get().toString());
        youTubeUserEMail->set(QVariant(""));
    }
}

/**
 * @brief Permanently remove a setting, from local memory and config files
 * @param setting The setting to remove
 */
void UBSettings::removeSetting(const QString &setting)
{
    if (sAppSettings->contains(setting))
        sAppSettings->remove(setting);

    if (mUserSettings->contains(setting))
        mUserSettings->remove(setting);

    if (mSettingsQueue.contains(setting))
        mSettingsQueue.remove(setting);
}

void UBSettings::checkNewSettings()
{
    /*
     * Some settings were modified in new versions and OpenBoard can crash
     * if an old settings file is used. This function checks these settings and
     * if necessary, resets them to the values initialized in UBSettings::init().
     *
     * Thus this method can be removed when it is no longer deemed useful.
     */

    // OB 1.3 introduced an extra pen color;  for simplicity, if the old settings
    // are still present (i.e only 4 selected pen colors), we just reset all color settings.
    // Having too few colors actually causes OpenBoard to crash, hence these measures.

    QList<UBColorListSetting*> colorSettings;
    colorSettings << boardPenDarkBackgroundSelectedColors
                  << boardPenLightBackgroundSelectedColors
                  << boardMarkerDarkBackgroundSelectedColors
                  << boardMarkerLightBackgroundSelectedColors;

    foreach (UBColorListSetting* setting, colorSettings) {
        if (setting->colors().size() < 5)
            setting->reset();
    }

    colorSettings.clear();

    // Next we check whether all the new colors were added; i.e if some default
    // colors are not also in the config file, we add them to it.

    // This is not nearly as critical as the above issue, but the users (or admins) seemingly
    // can't be trusted to delete or modify their own config file when upgrading, so they might
    // wonder why they don't have the new colors in their app.

    colorSettings << boardPenDarkBackgroundColors
                  << boardPenLightBackgroundColors
                  << boardMarkerDarkBackgroundColors
                  << boardMarkerLightBackgroundColors;

    foreach (UBColorListSetting* setting, colorSettings) {
        QStringList defaultColors = qvariant_cast<QStringList>(setting->defaultValue());
        QStringList currentColors(qvariant_cast<QStringList>(setting->get())); // copy

        foreach (QString c, defaultColors) {
            if (!currentColors.contains(c))
                currentColors.append(QString(c));
        }

        setting->set(currentColors);
    }


    // A typo was corrected in version 1.3
    removeSetting("Board/useSystemOnScreenKeybard");


    // CrossedBackground changed in 1.4 (no longer a bool but an enum; can be crossed or ruled)
    removeSetting("Board/CrossedBackground");

}
