
/*
 * UBSettings.cpp
 *
 *  Created on: Oct 29, 2008
 *      Author: luc
 */

#include "UBSettings.h"

#include <QtGui>


#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"
#include "frameworks/UBDesktopServices.h"
#include "frameworks/UBCryptoUtils.h"

#include "UB.h"
#include "UBSetting.h"

#include "tools/UBToolsManager.h"

#include "core/memcheck.h"

QPointer<UBSettings> UBSettings::sSingleton = 0;

int UBSettings::pointerDiameter = 40;
int UBSettings::crossSize = 32;
int UBSettings::colorPaletteSize = 4;
int UBSettings::objectFrameWidth = 20;
int UBSettings::boardMargin = 10;

QString UBSettings::documentGroupName = QString("Subject");
QString UBSettings::documentName = QString("Lesson");
QString UBSettings::documentSize = QString("Size");
QString UBSettings::documentIdentifer = QString("ID");
QString UBSettings::documentVersion = QString("Version");
QString UBSettings::documentUpdatedAt = QString("UpdatedAt");

QString UBSettings::trashedDocumentGroupNamePrefix = QString("_Trash:");

QString UBSettings::uniboardDocumentNamespaceUri = "http://uniboard.mnemis.com/document";
QString UBSettings::uniboardApplicationNamespaceUri = "http://uniboard.mnemis.com/application";


const int UBSettings::sDefaultFontPixelSize = 36;
const char *UBSettings::sDefaultFontFamily = "Arial";

QString UBSettings::currentFileVersion = "4.4.0";

QColor UBSettings::crossDarkBackground = QColor(44, 44, 44, 200);
QColor UBSettings::crossLightBackground = QColor(165, 225, 255);

QBrush UBSettings::eraserBrushLightBackground = QBrush(QColor(255, 255, 255, 30));
QBrush UBSettings::eraserBrushDarkBackground = QBrush(QColor(127, 127, 127, 30));

QPen UBSettings::eraserPenDarkBackground = QPen(QColor(255, 255, 255, 63));
QPen UBSettings::eraserPenLightBackground = QPen(QColor(0, 0, 0, 63));

QColor UBSettings::documentSizeMarkColorDarkBackground = QColor(44, 44, 44, 200);
QColor UBSettings::documentSizeMarkColorLightBackground = QColor(241, 241, 241);

QColor UBSettings::paletteColor = QColor(127, 127, 127, 127);
QColor UBSettings::opaquePaletteColor = QColor(66, 66, 66, 200);

QColor UBSettings::documentViewLightColor = QColor(241, 241, 241);

QPointer<QSettings> UBSettings::sAppSettings = 0;

const int UBSettings::maxThumbnailWidth = 400;
const int UBSettings::defaultThumbnailWidth = 150;

const int UBSettings::defaultGipWidth = 150;
const int UBSettings::defaultSoundWidth = 50;
const int UBSettings::defaultImageWidth = 150;
const int UBSettings::defaultShapeWidth = 50;
const int UBSettings::defaultWidgetIconWidth = 110;
const int UBSettings::defaultVideoWidth = 80;

const int UBSettings::thumbnailSpacing = 20;

const qreal UBSettings::minScreenRatio = 1.33; // 800/600 or 1024/768

QStringList UBSettings::bitmapFileExtensions;
QStringList UBSettings::vectoFileExtensions;
QStringList UBSettings::imageFileExtensions;
QStringList UBSettings::widgetFileExtensions;
QStringList UBSettings::interactiveContentFileExtensions;

QColor UBSettings::treeViewBackgroundColor = QColor(209, 215, 226); //in synch with css tree view background

int UBSettings::objectInControlViewMargin = 100;

QString UBSettings::appPingMessage = "__uniboard_ping";

QString UBSettings::defaultDocumentGroupName;
QString UBSettings::documentTrashGroupName;

QSettings* UBSettings::getAppSettings()
{
    if (!UBSettings::sAppSettings)
    {
        QString tmpSettings = QDir::tempPath() + "/Uniboard.config";
        QString appSettings = UBPlatformUtils::applicationResourcesDirectory() + "/etc/Uniboard.config";

        // tmpSettings exists when upgrading Uniboard on Mac (see UBPlatformUtils_mac.mm updater:willInstallUpdate:)
        if (QFile::exists(tmpSettings))
        {
            QFile::rename(tmpSettings, appSettings);
        }

        UBSettings::sAppSettings = new QSettings(appSettings, QSettings::IniFormat, 0);
    }

    return UBSettings::sAppSettings;
}


UBSettings::UBSettings(QObject *parent)
    : QObject(parent)
{
    mAppSettings = UBSettings::getAppSettings();

    QString userSettingsFile = UBSettings::uniboardDataDirectory() + "/UniboardUser.config";

    mUserSettings = new QSettings(userSettingsFile, QSettings::IniFormat, parent);

    init();
}


UBSettings::~UBSettings()
{
    delete mAppSettings;
}


void UBSettings::init()
{
    uniboardWebUrl = new UBSetting(this, "App", "UniboardWebAddress", "http://web.getuniboard.com");
    productWebUrl =  new UBSetting(this, "App", "ProductWebAddress", "http://www.sankore.org");

    softwareUpdatesUrl = uniboardWebUrl->get().toString() + "/software/installer/";
    softwareLibrariesUrl = uniboardWebUrl->get().toString() + "/software/library/";
    softwareHomeUrl = productWebUrl->get().toString();

    softwarePurchaseUrl = uniboardWebUrl->get().toString() + "/software/purchase/";

    oEmbedJsLibraryUrl = uniboardWebUrl->get().toString() + "/widgets/videopicker/scripts/jquery.oembed.js";

    documentsPublishingUrl = uniboardWebUrl->get().toString() + "/documents/publish";
    capturesPublishingUrl = uniboardWebUrl->get().toString() + "/captures/publish";
    publishingAuthenticationUrl = uniboardWebUrl->get().toString() + "/publish/token";

    documentPlayerScriptUrl = uniboardWebUrl->get().toString() + "/uniboard-player/scripts/ubplayer.js";
    documentPlayerPageScriptUrl = uniboardWebUrl->get().toString() + "/uniboard-player/scripts/ubplayer.page.js";
    documentPlayerCssUrl = uniboardWebUrl->get().toString() + QLatin1String("/uniboard-player/stylesheets/viewer.css");

#ifdef Q_WS_MAC
    appRssUrl = softwareUpdatesUrl + "appcast-mac.xml";
#elif defined(Q_WS_WIN)
    appRssUrl = softwareUpdatesUrl + "appcast-win.xml";
#else
    appRssUrl = ""; // TODO Linux
#endif

    appToolBarPositionedAtTop = new UBSetting(this, "App", "ToolBarPositionedAtTop", true);
    appToolBarDisplayText = new UBSetting(this, "App", "ToolBarDisplayText", true);
    appEnableAutomaticSoftwareUpdates = new UBSetting(this, "App", "EnableAutomaticSoftwareUpdates", true);
    appEnableSoftwareUpdates = new UBSetting(this, "App", "EnableSoftwareUpdates", true);
    appToolBarOrientationVertical = new UBSetting(this, "App", "ToolBarOrientationVertical", false);
    navigPaletteWidth = new UBSetting(this, "Board", "NavigPaletteWidth", 300);
    libPaletteWidth = new UBSetting(this, "Board", "LibPaletteWidth", 300);
    appEnableUniboardTransition = new UBSetting(this, "App", "EnableUniboardTransition", true);

    appIsInSoftwareUpdateProcess = new UBSetting(this, "App", "IsInSoftwareUpdateProcess", false);
    appLastSessionDocumentUUID = new UBSetting(this, "App", "LastSessionDocumentUUID", "");
    appLastSessionPageIndex = new UBSetting(this, "App", "LastSessionPageIndex", 0);

    boardPenFineWidth = new UBSetting(this, "Board", "PenFineWidth", 1.5);
    boardPenMediumWidth = new UBSetting(this, "Board", "PenMediumWidth", 3.0);
    boardPenStrongWidth = new UBSetting(this, "Board", "PenStrongWidth", 8.0);

    boardMarkerFineWidth = new UBSetting(this, "Board", "MarkerFineWidth", 12.0);
    boardMarkerMediumWidth = new UBSetting(this, "Board", "MarkerMediumWidth", 24.0);
    boardMarkerStrongWidth = new UBSetting(this, "Board", "MarkerStrongWidth", 48.0);

    boardPenPressureSensitive = new UBSetting(this, "Board", "PenPressureSensitive", true);
    boardMarkerPressureSensitive = new UBSetting(this, "Board", "MarkerPressureSensitive", false);

    boardUseHighResTabletEvent = new UBSetting(this, "Board", "UseHighResTabletEvent", true);

    boardKeyboardPaletteAutoMinimize = new UBSetting(this, "Board", "KeyboardPaletteAutoMinimize", true);
    boardKeyboardPaletteKeyBtnSize = new UBSetting(this, "Board", "KeyboardPaletteKeyBtnSize", "24x24");

    QStringList penLightBackgroundColors;
    penLightBackgroundColors << "#000000" << "#FF0000" <<"#004080" << "#008000" << "#C87400" << "#800040" << "#008080"  << "#5F2D0A";
    boardPenLightBackgroundColors = new UBColorListSetting(this, "Board", "PenLightBackgroundColors", penLightBackgroundColors, 1.0);

    QStringList penDarkBackgroundColors;
    penDarkBackgroundColors << "#FFFFFF" << "#FF3400" <<"#66C0FF" << "#81FF5C" << "#FFFF00" << "#B68360" << "#FF497E" << "#8D69FF";
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

    webUseExternalBrowser = new UBSetting(this, "Web", "UseExternalBrowser", false);

    bool defaultShowPageImmediatelyOnMirroredScreen = true;

#if defined(Q_WS_X11)
    // screen duplication is very slow on X11
    defaultShowPageImmediatelyOnMirroredScreen = false;
#endif

    webShowPageImmediatelyOnMirroredScreen = new UBSetting(this, "Web", "ShowPageImediatelyOnMirroredScreen", defaultShowPageImmediatelyOnMirroredScreen);

    webHomePage = new UBSetting(this, "Web", "Homepage", softwareHomeUrl);
    webBookmarksPage = new UBSetting(this, "Web", "BookmarksPage", "http://www.myuniboard.com");
    webAddBookmarkUrl = new UBSetting(this, "Web", "AddBookmarkURL", "http://www.myuniboard.com/bookmarks/save/?url=");
    webShowAddBookmarkButton = new UBSetting(this, "Web", "ShowAddBookmarkButton", false);

    pageCacheSize = new UBSetting(this, "App", "PageCacheSize", 20);

    bitmapFileExtensions << "jpg" << "jpeg" <<  "png" <<  "tiff" << "tif" << "bmp" << "gif";
    vectoFileExtensions << "svg" <<  "svgz";
    imageFileExtensions << bitmapFileExtensions << vectoFileExtensions;

    widgetFileExtensions << "wdgt" << "wgt" << "pwgt";
    interactiveContentFileExtensions << widgetFileExtensions << "swf";

    boardZoomFactor = new UBSetting(this, "Board", "ZoomFactor", QVariant(1.41));

    if (boardZoomFactor->get().toDouble() <= 1.)
        boardZoomFactor->set(1.41);

    int defaultRefreshRateInFramePerSecond = 8;

#if defined(Q_WS_X11)
    // screen duplication is very slow on X11
    defaultRefreshRateInFramePerSecond = 2;
#endif

    mirroringRefreshRateInFps = new UBSetting(this, "Mirroring", "RefreshRateInFramePerSecond", QVariant(defaultRefreshRateInFramePerSecond));

    lastImportFilePath = new UBSetting(this, "Import", "LastImportFilePath", QVariant(UBDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));
    lastImportFolderPath = new UBSetting(this, "Import", "LastImportFolderPath", QVariant(UBDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));
    lastExportFilePath = new UBSetting(this, "Export", "LastExportFilePath", QVariant(UBDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));
    lastExportDirPath = new UBSetting(this, "Export", "LastExportDirPath", QVariant(UBDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));
    lastImportToLibraryPath = new UBSetting(this, "Library", "LastImportToLibraryPath", QVariant(UBDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));

    lastPicturePath = new UBSetting(this, "Library", "LastPicturePath", QVariant(UBDesktopServices::storageLocation(QDesktopServices::PicturesLocation)));
    lastWidgetPath = new UBSetting(this, "Library", "LastWidgetPath", QVariant(UBDesktopServices::storageLocation(QDesktopServices::DocumentsLocation)));
    lastVideoPath = new UBSetting(this, "Library", "LastVideoPath", QVariant(UBDesktopServices::storageLocation(QDesktopServices::MoviesLocation)));

    defaultDocumentGroupName = tr("Untitled Documents");
    documentTrashGroupName = tr("Trash");

    appOnlineUserName = new UBSetting(this, "App", "OnlineUserName", "");

    boardShowToolsPalette = new UBSetting(this, "Board", "ShowToolsPalette", "false");

    documentSizes.insert(DocumentSizeRatio::Ratio4_3, QSize(1280, 960)); // 1.33
    documentSizes.insert(DocumentSizeRatio::Ratio16_9, QSize((960 / 9 * 16), 960)); // 1.77

    defaultDocumentSize = documentSizes.value(DocumentSizeRatio::Ratio4_3);

    svgViewBoxMargin = new UBSetting(this, "SVG", "ViewBoxMargin", "50");

    pdfMargin = new UBSetting(this, "PDF", "Margin", "20");
    pdfPageFormat = new UBSetting(this, "PDF", "PageFormat", "A4");
    pdfResolution = new UBSetting(this, "PDF", "Resolution", "300");

    podcastFramesPerSecond = new UBSetting(this, "Podcast", "FramesPerSecond", 10);
    podcastVideoSize = new UBSetting(this, "Podcast", "VideoSize", "Medium");
    podcastAudioRecordingDevice = new UBSetting(this, "Podcast", "AudioRecordingDevice", "Default");

    podcastWindowsMediaBitsPerSecond = new UBSetting(this, "Podcast", "WindowsMediaBitsPerSecond", 1700000);
    podcastQuickTimeQuality = new UBSetting(this, "Podcast", "QuickTimeQuality", "High");

    podcastPublishToYoutube = new UBSetting(this, "Podcast", "PublishToYouTube", false);
    youTubeUserEMail = new UBSetting(this, "YouTube", "UserEMail", "");

    uniboardWebEMail = new UBSetting(this, "UniboardWeb", "EMail", "");
    uniboardWebAuthor = new UBSetting(this, "UniboardWeb", "Author", "");
    uniboardWebGoogleMapApiKey = new UBSetting(this, "UniboardWeb", "GoogleMapAPIKey", "ABQIAAAAsWU4bIbaeCLinpZ30N_erRQEk562OPinwQkG9J-ZXUNAqYhJ5RT_z2EmpfVXiUg8c41BcsD_XM6P5g");

    communityUser = new UBSetting(this, "Community", "Username", "");
    communityPsw = new UBSetting(this, "Community", "Password", "");

    QStringList uris = UBToolsManager::manager()->allToolIDs();

    favoritesNativeToolUris = new UBSetting(this, "App", "FavoriteToolURIs", uris);

    // removed in version 4.4.b.2
    mUserSettings->remove("Podcast/RecordMicrophone");

    replyWWSerialPort = new UBSetting(this, "Voting", "ReplyWWSerialPort", 3);

    replyPlusConnectionURL = new UBSetting(this, "Voting", "ReplyPlusConnectionURL", "USB");
    replyPlusAddressingMode = new UBSetting(this, "Voting", "ReplyPlusAddressingMode", "static");
    replyPlusMaxKeypads = new UBSetting(this, "Voting", "ReplyPlusMaxKeypads", "100");

    documentThumbnailWidth = new UBSetting(this, "Document", "ThumbnailWidth", UBSettings::defaultThumbnailWidth);

    imageThumbnailWidth = new UBSetting(this, "Library", "ImageThumbnailWidth", UBSettings::defaultImageWidth);
    videoThumbnailWidth = new UBSetting(this, "Library", "VideoThumbnailWidth", UBSettings::defaultVideoWidth);
    shapeThumbnailWidth = new UBSetting(this, "Library", "ShapeThumbnailWidth", UBSettings::defaultShapeWidth);
    gipThumbnailWidth = new UBSetting(this, "Library", "ImageThumbnailWidth", UBSettings::defaultGipWidth);
    soundThumbnailWidth = new UBSetting(this, "Library", "SoundThumbnailWidth", UBSettings::defaultSoundWidth);;

    podcastPublishToIntranet = new UBSetting(this, "IntranetPodcast", "PublishToIntranet", false);
    intranetPodcastPublishingUrl = new UBSetting(this, "IntranetPodcast", "PublishingUrl", "");
    intranetPodcastAuthor = new UBSetting(this, "IntranetPodcast", "Author", "");
}


QVariant UBSettings::value ( const QString & key, const QVariant & defaultValue) const
{
    if (!sAppSettings->contains(key) && !(defaultValue == QVariant()))
    {
        sAppSettings->setValue(key, defaultValue);
    }

    return mUserSettings->value(key, sAppSettings->value(key, defaultValue));
}


void UBSettings::setValue (const QString & key, const QVariant & value)
{
    mUserSettings->setValue(key, value);
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


bool UBSettings::isCrossedBackground()
{
    return value("Board/CrossedBackground", 0).toBool();
}


void UBSettings::setDarkBackground(bool isDarkBackground)
{
    setValue("Board/DarkBackground", isDarkBackground);
    emit colorContextChanged();
}


void UBSettings::setCrossedBackground(bool isCrossedBackground)
{
    setValue("Board/CrossedBackground", isCrossedBackground);
}


void UBSettings::setPenPressureSensitive(bool sensitive)
{
    boardPenPressureSensitive->set(sensitive);
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


int UBSettings::fontPixelSize()
{
    return value("Board/FontPixelSize", sDefaultFontPixelSize).toInt();
}


void UBSettings::setFontPixelSize(int pixelSize)
{
    setValue("Board/FontPixelSize", pixelSize);
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


QString UBSettings::uniboardDataDirectory()
{

    // first look into the application settings
    if (sAppSettings && getAppSettings()->contains("App/DataDirectory"))
    {
        QString dataDirectory = getAppSettings()->value("App/DataDirectory").toString();

        // mute it to something absolute
        if (dataDirectory.startsWith("{Documents}"))
        {
            dataDirectory = dataDirectory.replace("{Documents}",
                    QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
        }
        else if(dataDirectory.startsWith("{Home}"))
        {
            dataDirectory = dataDirectory.replace("{Home}",
                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
        }

        if(dataDirectory.contains("{UserLoginName}") && UBPlatformUtils::osUserLoginName().length() > 0)
        {
            dataDirectory = dataDirectory.replace("{UserLoginName}", UBPlatformUtils::osUserLoginName());
        }

        // valid ?
        QDir dir(dataDirectory);
        dir.mkpath(dataDirectory);

        if (dir.exists(dataDirectory))
        {
            return dataDirectory;
        }
        else
        {
            // Do not log anything here, we need data directory to be valid to be able to log to it
            //
            //qWarning << "Failed to interpret App/DataDirectory config : " + getAppSettings()->value("App/DataDirectory").toString() +
            //    ", defaulting to " + UBDesktopServices::storageLocation(QDesktopServices::DataLocation);
        }
    }
    QString qtDataPath = UBFileSystemUtils::normalizeFilePath(UBDesktopServices::storageLocation(QDesktopServices::DataLocation));
    qtDataPath.replace("/Sankore 3.1", "");
    return qtDataPath;
}


QString UBSettings::uniboardUserImageDirectory()
{
    QString valideUserImageDirectory =
        UBDesktopServices::storageLocation(QDesktopServices::PicturesLocation)
        + "/" + QCoreApplication::applicationName();

    bool hasCreatedDir = false;

    // first look into the application settings
    if (sAppSettings && getAppSettings()->contains("App/UserImageDirectory"))
    {
        QString userImageDirectory = getAppSettings()->value("App/UserImageDirectory").toString();

        // mute it to something absolute
        if (userImageDirectory.startsWith("{Documents}"))
        {
            userImageDirectory = userImageDirectory.replace("{Documents}",
                    QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
        }
        else if (userImageDirectory.startsWith("{Home}"))
        {
            userImageDirectory = userImageDirectory.replace("{Home}",
                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
        }
        else if(userImageDirectory.startsWith("{Pictures}"))
        {
            userImageDirectory = userImageDirectory.replace("{Pictures}",
                    QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
        }

        if(userImageDirectory.contains("{UserLoginName}") && UBPlatformUtils::osUserLoginName() > 0)
        {
            userImageDirectory = userImageDirectory.replace("{UserLoginName}", UBPlatformUtils::osUserLoginName());
        }

        // valid ?
        QDir dir(userImageDirectory);

        if (!dir.exists())
        {
            dir.mkpath(userImageDirectory);
            hasCreatedDir = true;
        }

        if (dir.exists())
        {
            valideUserImageDirectory = userImageDirectory;
        }
        else
        {
            hasCreatedDir = false;
            qWarning() << "Failed to interpret App/UserImageDirectory config : "
                + getAppSettings()->value("App/UserImageDirectory").toString()
                + ", defaulting to " + valideUserImageDirectory;
        }
    }

    QDir userImageDir(valideUserImageDirectory);

    userImageDir.mkpath(valideUserImageDirectory);

    if (hasCreatedDir || !userImageDir.exists())
    {
        QString defaultUserImageDir = valideUserImageDirectory + "/" + tr("Images");
        UBFileSystemUtils::copyDir(uniboardDefaultUserImageLibraryDirectory(),
                defaultUserImageDir);
    }

    return valideUserImageDirectory;
}


QString UBSettings::defaultUserImagesDirectory()
{
    QString userImageDir = uniboardUserImageDirectory() + "/" + tr("Images");
    QDir dir(userImageDir);
    dir.mkpath(userImageDir);

    return userImageDir;
}


QString UBSettings::uniboardUserVideoDirectory()
{
    QString valideUserVideoDirectory =
        UBDesktopServices::storageLocation(QDesktopServices::MoviesLocation);

    bool hasCreatedDir = false;

    // first look into the application settings
    if (sAppSettings && getAppSettings()->contains("App/UserVideoDirectory"))
    {
        QString userVideoDirectory = getAppSettings()->value("App/UserVideoDirectory").toString();

        // mute it to something absolute
        if (userVideoDirectory.startsWith("{Documents}"))
        {
            userVideoDirectory = userVideoDirectory.replace("{Documents}",
                    QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
        }
        else if(userVideoDirectory.startsWith("{Home}"))
        {
            userVideoDirectory = userVideoDirectory.replace("{Home}",
                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
        }
        else if(userVideoDirectory.startsWith("{Videos}"))
        {
            userVideoDirectory = userVideoDirectory.replace("{Videos}",
                    QDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
        }

        if(userVideoDirectory.contains("{UserLoginName}") && UBPlatformUtils::osUserLoginName() > 0)
        {
            userVideoDirectory = userVideoDirectory.replace("{UserLoginName}", UBPlatformUtils::osUserLoginName());
        }

        // valid ?
        QDir dir(userVideoDirectory);

        if (!dir.exists())
        {
            dir.mkpath(userVideoDirectory);
            hasCreatedDir = true;
        }

        if (dir.exists())
        {
            valideUserVideoDirectory = userVideoDirectory;
        }
        else
        {
            hasCreatedDir = false;
            qWarning() << "Failed to interpret App/UserVideoDirectory config : "
                + getAppSettings()->value("App/UserVideoDirectory").toString()
                + ", defaulting to " + valideUserVideoDirectory;
        }
    }
    else
    {
        // May not exists (on windows XP)
        if (valideUserVideoDirectory.length() == 0)
        {
            valideUserVideoDirectory = QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation) + "/" + tr("My Movies");
        }

        QDir dir(valideUserVideoDirectory);

        if (!dir.exists())
        {
            dir.mkpath(valideUserVideoDirectory);
        }
    }

    return valideUserVideoDirectory;
}


QString UBSettings::podcastRecordingDirectory()
{
    QString validePodcastRecordingDirectory =
        UBDesktopServices::storageLocation(QDesktopServices::DesktopLocation);

    bool hasCreatedDir = false;

    // first look into the application settings
    if (sAppSettings && getAppSettings()->contains("Podcast/RecordingDirectory"))
    {
        QString userPodcastRecordingDirectory = getAppSettings()->value("Podcast/RecordingDirectory").toString();

        // mute it to something absolute
        if (userPodcastRecordingDirectory.startsWith("{Documents}"))
        {
            userPodcastRecordingDirectory = userPodcastRecordingDirectory.replace("{Documents}",
                    QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
        }
        else if(userPodcastRecordingDirectory.startsWith("{Home}"))
        {
            userPodcastRecordingDirectory = userPodcastRecordingDirectory.replace("{Home}",
                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
        }
        else if(userPodcastRecordingDirectory.startsWith("{Videos}"))
        {
            userPodcastRecordingDirectory = userPodcastRecordingDirectory.replace("{Videos}",
                    QDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
        }

        if(userPodcastRecordingDirectory.contains("{UserLoginName}") && UBPlatformUtils::osUserLoginName() > 0)
        {
            userPodcastRecordingDirectory = userPodcastRecordingDirectory.replace("{UserLoginName}", UBPlatformUtils::osUserLoginName());
        }

        // valid ?
        QDir dir(userPodcastRecordingDirectory);

        if (!dir.exists())
        {
            dir.mkpath(userPodcastRecordingDirectory);
            hasCreatedDir = true;
        }

        if (dir.exists())
        {
            validePodcastRecordingDirectory = userPodcastRecordingDirectory;
        }
        else
        {
            hasCreatedDir = false;
            qWarning() << "Failed to interpret Podcast/RecordingDirectory config : "
                + getAppSettings()->value("Podcast/RecordingDirectory").toString()
                + ", defaulting to user Desktop";
        }
    }
    else
    {

        QDir dir(validePodcastRecordingDirectory);

        if (!dir.exists())
        {
            dir.mkpath(validePodcastRecordingDirectory);
        }
    }

    return validePodcastRecordingDirectory;
}


QString UBSettings::uniboardDocumentDirectory()
{
    return uniboardDataDirectory() + "/document";
}

QString UBSettings::libraryPaletteFavoriteListFilePath()
{
    QString filePath = uniboardDataDirectory() + "/libraryPalette/favorite.dat";

    if (!QDir(uniboardDataDirectory() + "/libraryPalette").exists())
        QDir().mkpath(uniboardDataDirectory() + "/libraryPalette");

    return filePath;
}

QString UBSettings::trashLibraryPaletteDirPath()
{
    QString trashPath = uniboardDataDirectory() + "/libraryPalette/trash";
    if (!QDir(trashPath).exists())
        QDir().mkpath(trashPath);
    return trashPath;
}


QString UBSettings::uniboardShapeLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/shape");

    QString configPath = value("Library/ShapeDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith("."))
    {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else
    {
        return configPath;
    }
}

QString UBSettings::uniboardGipLibraryDirectory()
{
    QString dirPath = UBPlatformUtils::applicationResourcesDirectory() + "/library/gips";
    if (!QDir(dirPath).exists())
        QDir().mkpath(dirPath);

    return dirPath;
}

QString UBSettings::uniboardImageLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/image");

    QString configPath = value("Library/ImageDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith("."))
    {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else
    {
        return configPath;
    }
}


QString UBSettings::uniboardDefaultUserImageLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/userImage");

    QString configPath = value("Library/DefaultUserImageDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith("."))
    {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else
    {
        return configPath;
    }
}


QString UBSettings::animationUserDirectory()
{
    QString animationDirectory = uniboardDataDirectory() + "/animationUserDirectory";
    if (!QDir(animationDirectory).exists())
        QDir().mkpath(animationDirectory);

    return animationDirectory;
}

QString UBSettings::uniboardInteractiveUserDirectory()
{
    QString valideUserInteractiveDirectory = uniboardDataDirectory() + "/interactive content";

    bool hasCreatedDir = false;

    // first look into the application settings
    if (sAppSettings && getAppSettings()->contains("App/UserInteractiveContentDirectory"))
    {
        QString userWidgetDirectory = getAppSettings()->value("App/UserInteractiveContentDirectory").toString();

        // mute it to something absolute
        if (userWidgetDirectory.startsWith("{Documents}"))
        {
            userWidgetDirectory = userWidgetDirectory.replace("{Documents}",
                    QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
        }
        else if(userWidgetDirectory.startsWith("{Home}"))
        {
            userWidgetDirectory = userWidgetDirectory.replace("{Home}",
                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
        }
        else if(userWidgetDirectory.startsWith("{Videos}"))
        {
            userWidgetDirectory = userWidgetDirectory.replace("{Videos}",
                    QDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
        }

        // valid ?
        QDir dir(userWidgetDirectory);

        if (!dir.exists())
        {
            dir.mkpath(userWidgetDirectory);
            hasCreatedDir = true;
        }

        if (dir.exists())
        {
            valideUserInteractiveDirectory = userWidgetDirectory;
        }
        else
        {
            hasCreatedDir = false;
            qWarning() << "Failed to interpret App/UserInteractiveContentDirectory config : "
                + getAppSettings()->value("App/UserInteractiveContentDirectory").toString()
                + ", defaulting to " + valideUserInteractiveDirectory;
        }
    }
    else
    {
        QDir widgetUserDir = QDir(valideUserInteractiveDirectory);
        if (!widgetUserDir.exists())
        {
            widgetUserDir.mkpath(valideUserInteractiveDirectory);
        }
    }

    return valideUserInteractiveDirectory;
}


QString UBSettings::sankoreDistributedInteractiveDirectory()
{
    QString defaultRelativePath = QString("./library/sankoreInteractivities");

    QString configPath = value("Library/sankoreInteractivities", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith("."))
    {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else
    {
        return configPath;
    }
}

QString UBSettings::uniboardInteractiveLibraryDirectory()
{
    QString defaultRelativePath = QString("./library/interactive");

    QString configPath = value("Library/InteractiveContentDirectory", QVariant(defaultRelativePath)).toString();

    if (configPath.startsWith("."))
    {
        return UBPlatformUtils::applicationResourcesDirectory() + configPath.right(configPath.size() - 1);
    }
    else
    {
        return configPath;
    }
}


QString UBSettings::uniboardInteractiveFavoritesDirectory()
{
    QString valideUserInteractiveDirectory = uniboardDataDirectory() + "/interactive favorites";

    bool hasCreatedDir = false;

    // first look into the application settings
    if (sAppSettings && getAppSettings()->contains("App/UserInteractiveFavoritesDirectory"))
    {
        QString userWidgetDirectory = getAppSettings()->value("App/UserInteractiveFavoritesDirectory").toString();

        // mute it to something absolute
        if (userWidgetDirectory.startsWith("{Documents}"))
        {
            userWidgetDirectory = userWidgetDirectory.replace("{Documents}",
                    QDesktopServices::storageLocation(QDesktopServices::DocumentsLocation));
        }
        else if(userWidgetDirectory.startsWith("{Home}"))
        {
            userWidgetDirectory = userWidgetDirectory.replace("{Home}",
                    QDesktopServices::storageLocation(QDesktopServices::HomeLocation));
        }
        else if(userWidgetDirectory.startsWith("{Videos}"))
        {
            userWidgetDirectory = userWidgetDirectory.replace("{Videos}",
                    QDesktopServices::storageLocation(QDesktopServices::MoviesLocation));
        }

        // valid ?
        QDir dir(userWidgetDirectory);

        if (!dir.exists())
        {
            dir.mkpath(userWidgetDirectory);
            hasCreatedDir = true;
        }

        if (dir.exists())
        {
            valideUserInteractiveDirectory = userWidgetDirectory;
        }
        else
        {
            hasCreatedDir = false;
            qWarning() << "Failed to interpret App/UserInteractiveFavoritesDirectory config : "
                + getAppSettings()->value("App/UserInteractiveFavoritesDirectory").toString()
                + ", defaulting to " + valideUserInteractiveDirectory;
        }
    }
    else
    {
        QDir widgetUserDir = QDir(valideUserInteractiveDirectory);
        if (!widgetUserDir.exists())
        {
            widgetUserDir.mkpath(valideUserInteractiveDirectory);
        }
    }

    return valideUserInteractiveDirectory;
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
