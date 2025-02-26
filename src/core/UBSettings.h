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




#ifndef UBSETTINGS_H_
#define UBSETTINGS_H_

#include <QtCore>
#include <QtGui>
#include <QtNetwork>

#include "UB.h"
#include "UBSetting.h"

class UBSettings : public QObject
{

    Q_OBJECT

    public:

        static UBSettings* settings();
        static void destroy();

    private:

        UBSettings(QObject *parent = 0);
        virtual ~UBSettings();
        void cleanNonPersistentSettings();

    public:

        QStringList* supportedKeyboardSizes;
        void InitKeyboardPaletteKeyBtnSizes();
        void ValidateKeyboardPaletteKeyBtnSize();
        void closing();
        void save();

        int penWidthIndex();

        qreal currentPenWidth();

        int penColorIndex();
        QColor currentPenColor();
        QColor penColor(bool onDarkBackground);
        QList<QColor> penColors(bool onDarkBackground);

        // Marker related
        int markerWidthIndex();
        qreal currentMarkerWidth();
        int markerColorIndex();
        QColor currentMarkerColor();
        QColor markerColor(bool onDarkBackground);
        QList<QColor> markerColors(bool onDarkBackground);

        // Eraser related
        int eraserWidthIndex();
        qreal eraserFineWidth();
        qreal eraserMediumWidth();
        qreal eraserStrongWidth();
        qreal currentEraserWidth();

        // Background related
        bool isDarkBackground();
        QUuid pageBackgroundUuid();
        void setDarkBackground(bool isDarkBackground);
        void setPageBackgroundUuid(const QUuid& background);

        // Stylus palette related
        bool isStylusPaletteVisible();

        // Text related
        QString fontFamily();
        void setFontFamily(const QString &family);
        QString fontStyleName();
        void setFontStyleName(const QString &family);
        int fontPixelSize();
        void setFontPixelSize(int pixelSize);
        int fontPointSize();
        void setFontPointSize(int pointSize);
        bool isBoldFont();
        void setBoldFont(bool bold);
        bool isItalicFont();
        void setItalicFont(bool italic);

        void setPassword(const QString& id, const QString& password);
        QString password(const QString& id);
        void removePassword(const QString& id);

        QString proxyUsername();
        void setProxyUsername(const QString& username);
        QString proxyPassword();
        void setProxyPassword(const QString& password);

        QString communityUsername();
        void setCommunityUsername(const QString& username);
        QString communityPassword();
        void setCommunityPassword(const QString& password);
        bool getCommunityDataPersistence(){return communityCredentialsPersistence->get().toBool();}
        void setCommunityPersistence(const bool persistence);

        int libraryIconSize();
        void setLibraryIconsize(const int& size);

        void init();

        //user directories
        static QString userDataDirectory();
        static QString userDocumentDirectory();
        static QString userFavoriteListFilePath();
        static QString userTrashDirPath();
        static QString userImageDirectory();
        static QString userVideoDirectory();
        static QString userAudioDirectory();
        static QString userSearchDirectory();
        static QString userAnimationDirectory();
        static QString userInteractiveDirectory();
        static QString userInteractiveFavoritesDirectory();
        static QString userPodcastRecordingDirectory();

        QString userGipLibraryDirectory();

        //application directory
        QString applicationShapeLibraryDirectory();
        QString applicationImageLibraryDirectory();
        QString applicationApplicationsLibraryDirectory();
        QString applicationInteractivesDirectory();
        QString applicationCustomizationDirectory();
        QString applicationCustomFontDirectory();
        QString applicationAudiosLibraryDirectory();
        QString applicationVideosLibraryDirectory();
        QString applicationAnimationsLibraryDirectory();
        QString applicationStartupHintsDirectory();

        QNetworkProxy* httpProxy();

        static int pointerDiameter;
        static int boardMargin;

        static QColor paletteColor;
        static QColor opaquePaletteColor;

        static QColor documentViewLightColor;

        static QBrush eraserBrushDarkBackground;
        static QBrush eraserBrushLightBackground;

        static QPen eraserPenDarkBackground;
        static QPen eraserPenLightBackground;

        static QColor markerCircleBrushColorDarkBackground;
        static QColor markerCircleBrushColorLightBackground;

        static QColor markerCirclePenColorDarkBackground;
        static QColor markerCirclePenColorLightBackground;

        static QColor penCircleBrushColorDarkBackground;
        static QColor penCircleBrushColorLightBackground;

        static QColor penCirclePenColorDarkBackground;
        static QColor penCirclePenColorLightBackground;

        static QColor documentSizeMarkColorDarkBackground;
        static QColor documentSizeMarkColorLightBackground;

        // Background grid
        static int crossSize;
        static int defaultCrossSize;
        static int minCrossSize;
        static int maxCrossSize;

        static int colorPaletteSize;
        static int objectFrameWidth;

        static QString documentGroupName;
        static QString documentName;
        static QString documentSize;
        static QString documentIdentifer;
        static QString documentVersion;
        static QString documentUpdatedAt;

        static QString documentDate;

        static QString trashedDocumentGroupNamePrefix;

        static QString currentFileVersion;

        static QString uniboardDocumentNamespaceUri;
        static QString uniboardApplicationNamespaceUri;

        static QString undoCommandTransactionName;

        static const int maxThumbnailWidth;
        static const int defaultThumbnailWidth;
        static const int defaultSortKind;
        static const int defaultSortOrder;
        static const int defaultSplitterLeftSize;
        static const int defaultSplitterRightSize;
        static const int defaultLibraryIconSize;

        static const int defaultImageWidth;
        static const int defaultShapeWidth;
        static const int defaultWidgetIconWidth;
        static const int defaultVideoWidth;
        static const int defaultGipWidth;
        static const int defaultSoundWidth;

        static const int thumbnailSpacing;
        static const int longClickInterval;

        static const qreal minScreenRatio;

        static QStringList bitmapFileExtensions;
        static QStringList vectoFileExtensions;
        static QStringList imageFileExtensions;
        static QStringList widgetFileExtensions;
        static QStringList interactiveContentFileExtensions;

        static QColor treeViewBackgroundColor;

        static int objectInControlViewMargin;

        static QString appPingMessage;

        UBSetting* productWebUrl;

        QString softwareHomeUrl;

        UBSetting* tutorialUrl;

        UBSetting* appToolBarPositionedAtTop;
        UBSetting* appToolBarDisplayText;
        UBSetting* appEnableAutomaticSoftwareUpdates;
        UBSetting* appSoftwareUpdateURL;
        UBSetting* appHideCheckForSoftwareUpdate;
        UBSetting* appToolBarOrientationVertical;
        UBSetting* appPreferredLanguage;
        UBSetting* appRunInWindow;

        UBSetting* appIsInSoftwareUpdateProcess;

        UBSetting* appLastSessionDocumentUUID;
        UBSetting* appLastSessionPageIndex;

        UBSetting* appUseMultiscreen;
        UBSetting* appScreenList;

        UBSetting* appStartupHintsEnabled;

        UBSetting* boardPenFineWidth;
        UBSetting* boardPenMediumWidth;
        UBSetting* boardPenStrongWidth;

        UBSetting* boardMarkerFineWidth;
        UBSetting* boardMarkerMediumWidth;
        UBSetting* boardMarkerStrongWidth;

        UBSetting* boardPenPressureSensitive;
        UBSetting* boardMarkerPressureSensitive;

        UBSetting* boardUseHighResTabletEvent;

        UBSetting* boardInterpolatePenStrokes;
        UBSetting* boardSimplifyPenStrokes;
        UBSetting* boardSimplifyPenStrokesThresholdAngle;
        UBSetting* boardSimplifyPenStrokesThresholdWidthDifference;
        UBSetting* boardInterpolateMarkerStrokes;
        UBSetting* boardSimplifyMarkerStrokes;

        UBSetting* boardKeyboardPaletteKeyBtnSize;

        UBSetting* appStartMode;

        UBSetting* featureSliderPosition;

        UBSetting* boardCrossColorDarkBackground;
        UBSetting* boardCrossColorLightBackground;

        UBColorListSetting* boardGridLightBackgroundColors;
        UBColorListSetting* boardGridDarkBackgroundColors;

        UBColorListSetting* boardPenLightBackgroundColors;
        UBColorListSetting* boardPenLightBackgroundSelectedColors;

        UBColorListSetting* boardPenDarkBackgroundColors;
        UBColorListSetting* boardPenDarkBackgroundSelectedColors;

        UBSetting* boardMarkerAlpha;

        UBColorListSetting* boardMarkerLightBackgroundColors;
        UBColorListSetting* boardMarkerLightBackgroundSelectedColors;

        UBColorListSetting* boardMarkerDarkBackgroundColors;
        UBColorListSetting* boardMarkerDarkBackgroundSelectedColors;

        UBSetting* showEraserPreviewCircle;
        UBSetting* showMarkerPreviewCircle;
        UBSetting* showPenPreviewCircle;
        UBSetting* penPreviewFromSize;

        UBSetting* webUseExternalBrowser;
        UBSetting* webShowPageImmediatelyOnMirroredScreen;

        UBSetting* webHomePage;
        UBSetting* webSearchEngineUrl;
        UBSetting* alternativeUserAgent;
        UBSetting* alternativeUserAgentDomains;
        UBSetting* webCookieAutoDelete;
        UBSetting* webCookieKeepDomains;
        UBSetting* webCookiePolicy;
        UBSetting* webPrivateBrowsing;

        UBSetting* pageCacheSize;

        UBSetting* boardZoomBase;
        UBSetting* boardZoomFactor;

        UBSetting* mirroringRefreshRateInFps;

        UBSetting* lastImportFilePath;
        UBSetting* lastImportFolderPath;

        UBSetting* lastExportFilePath;
        UBSetting* lastExportDirPath;

        UBSetting* lastImportToLibraryPath;

        UBSetting* lastPicturePath;
        UBSetting* lastWidgetPath;
        UBSetting* lastVideoPath;

        UBSetting* boardShowToolsPalette;

        QMap<DocumentSizeRatio::Enum, QSize> documentSizes;

        UBSetting* svgViewBoxMargin;
        UBSetting* pdfMargin;
        UBSetting* pdfPageFormat;
        UBSetting* pdfUsePDFMerger;
        UBSetting* pdfResolution;

        UBSetting* exportBackgroundGrid;
        UBSetting* exportBackgroundColor;

        UBSetting* podcastFramesPerSecond;
        UBSetting* podcastVideoSize;
        UBSetting* podcastWindowsMediaBitsPerSecond;
        UBSetting* podcastAudioRecordingDevice;
        UBSetting* podcastQuickTimeQuality;

        UBSetting* podcastPublishToYoutube;
        UBSetting* youTubeUserEMail;
        UBSetting* youTubeCredentialsPersistence;

        UBSetting* podcastPublishToIntranet;
        UBSetting* intranetPodcastPublishingUrl;
        UBSetting* intranetPodcastAuthor;

        UBSetting* favoritesNativeToolUris;

        UBSetting* documentThumbnailWidth;
        UBSetting* documentSortKind;
        UBSetting* documentSortOrder;
        UBSetting* documentSplitterLeftSize;
        UBSetting* documentSplitterRightSize;
        UBSetting* imageThumbnailWidth;
        UBSetting* videoThumbnailWidth;
        UBSetting* shapeThumbnailWidth;
        UBSetting* gipThumbnailWidth;
        UBSetting* soundThumbnailWidth;

        UBSetting* libraryShowDetailsForLocalItems;

        UBSetting* rightLibPaletteBoardModeWidth;
        UBSetting* rightLibPaletteBoardModeIsCollapsed;
        UBSetting* rightLibPaletteDesktopModeWidth;
        UBSetting* rightLibPaletteDesktopModeIsCollapsed;
        UBSetting* leftLibPaletteBoardModeWidth;
        UBSetting* leftLibPaletteBoardModeIsCollapsed;
        UBSetting* leftLibPaletteDesktopModeWidth;
        UBSetting* leftLibPaletteDesktopModeIsCollapsed;

        UBSetting* communityUser;
        UBSetting* communityPsw;
        UBSetting* communityCredentialsPersistence;

        UBSetting* pageSize;

        UBSetting* KeyboardLocale;
        UBSetting* swapControlAndDisplayScreens;

        UBSetting* rotationAngleStep;
        UBSetting* historyLimit;

        UBSetting* libIconSize;

        UBSetting* useSystemOnScreenKeyboard;

        UBSetting* showDateColumnOnAlphabeticalSort;

        UBSetting* emptyTrashForOlderDocuments;
        UBSetting* emptyTrashDaysValue;

        UBSetting* magnifierDrawingMode;
        UBSetting* autoSaveInterval;

    public slots:

        void setPenWidthIndex(int index);
        void setPenColorIndex(int index);

        void setMarkerWidthIndex(int index);
        void setMarkerColorIndex(int index);

        void setEraserWidthIndex(int index);
        void setEraserFineWidth(qreal width);
        void setEraserMediumWidth(qreal width);
        void setEraserStrongWidth(qreal width);

         void setStylusPaletteVisible(bool visible);

        void setPenPressureSensitive(bool sensitive);
        void setPenPreviewCircle(bool sensitive);
        void setPenPreviewFromSize(int size);
        void setMarkerPressureSensitive(bool sensitive);

        QVariant value ( const QString & key, const QVariant & defaultValue = QVariant() );
        void setValue (const QString & key,const QVariant & value);

        void colorChanged() { emit colorContextChanged(); }

    signals:
        void colorContextChanged();

    private:

        QSettings* mAppSettings;
        QSettings* mUserSettings;

        QHash<QString, QVariant> mSettingsQueue;

        static const int sDefaultFontPixelSize;
        static const char *sDefaultFontFamily;
        static const char *sDefaultFontStyleName;

        static QSettings* getAppSettings();

        static QPointer<QSettings> sAppSettings;
        static QPointer<UBSettings> sSingleton;

        static bool checkDirectory(QString& dirPath);
        static QString replaceWildcard(QString& path);

        void removeSetting(const QString& setting);
        void checkNewSettings();

};


#endif /* UBSETTINGS_H_ */
