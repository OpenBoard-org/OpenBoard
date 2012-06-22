
#include "MacUtils.h"
#include "UBPlatformUtils.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"
#include "frameworks/UBFileSystemUtils.h"

#include <QWidget>

#import <Foundation/NSAutoreleasePool.h>
#import <Carbon/Carbon.h>
#import <APELite.h>

/*
// commented because Sankore crashes on Java Script. It seems to backends dependencies.
#import <WebKit/WebKit.h>
#import <AppKit/AppKit.h>
*/


NSString* bundleShortVersion(NSBundle *bundle)
{
    return [bundle objectForInfoDictionaryKey:@"CFBundleShortVersionString"];
}

OSStatus emptySetSystemUIMode (
   SystemUIMode inMode,
   SystemUIOptions inOptions)
{
    Q_UNUSED(inMode);
    Q_UNUSED(inOptions);
    // NOOP
    return noErr;
}

void *originalSetSystemUIMode = 0;

void UBPlatformUtils::init()
{
        initializeKeyboardLayouts();

    // qwidget_mac.mm qt_mac_set_fullscreen_mode uses kUIModeAllSuppressed which is unfortunate in our case
    //
    // http://developer.apple.com/mac/library/documentation/Carbon/Reference/Dock_Manager/Reference/reference.html#//apple_ref/c/func/SetSystemUIMode
    //

    originalSetSystemUIMode = APEPatchCreate((const void *)SetSystemUIMode, (const void *)emptySetSystemUIMode);

    setDesktopMode(false);

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSString *currentPath      = [[NSBundle mainBundle] pathForResource:@"Save PDF to Open-Sankore" ofType:@"workflow"];
    NSString *installedPath    = [[[@"~/Library/PDF Services" stringByExpandingTildeInPath] stringByAppendingPathComponent:@"Save PDF to Open-Sankore"] stringByAppendingPathExtension:@"workflow"];
    NSString *currentVersion   = bundleShortVersion([NSBundle bundleWithPath:currentPath]);
    NSString *installedVersion = bundleShortVersion([NSBundle bundleWithPath:installedPath]);

    if (![installedVersion isEqualToString:currentVersion])
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        [fileManager removeFileAtPath:installedPath handler:nil];
        // removing the old version of the script named Save PDF to Uniboard
        [fileManager removeFileAtPath:[[[@"~/Library/PDF Services" stringByExpandingTildeInPath] stringByAppendingPathComponent:@"Save PDF to Uniboard"] stringByAppendingPathExtension:@"workflow"] handler:nil];
        [fileManager createDirectoryAtPath:[installedPath stringByDeletingLastPathComponent] attributes:nil];
        BOOL copyOK = [fileManager copyPath:currentPath toPath:installedPath handler:nil];
        if (!copyOK)
        {
            qWarning("Could not install the 'Save PDF to Open-Sankoré workflow");
        }
    }

    [pool drain];
}


void UBPlatformUtils::setDesktopMode(bool desktop)
{
    OSStatus (*functor)(SystemUIMode, SystemUIOptions) = (OSStatus (*)(SystemUIMode, SystemUIOptions))originalSetSystemUIMode;

    if (desktop)
    {
        functor(kUIModeNormal, 0);
    }
    else
    {
        functor(kUIModeAllHidden, 0);
    }

}


QString UBPlatformUtils::applicationResourcesDirectory()
{
    QString path;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *resourcePath = [[NSBundle mainBundle] resourcePath];

    path = QString::fromUtf8([resourcePath fileSystemRepresentation], strlen([resourcePath fileSystemRepresentation]));
    [pool drain];

    return path;
}

void UBPlatformUtils::hideFile(const QString &filePath)
{
    FSRef ref;
    CFStringRef path = CFStringCreateWithCharacters(0, reinterpret_cast<const UniChar *>(filePath.unicode()), filePath.length());
    const CFIndex maxSize = CFStringGetMaximumSizeOfFileSystemRepresentation(path);
    UInt8 fileSystemRepresentation[maxSize];
    CFRelease(path);
    if (!CFStringGetFileSystemRepresentation(path, (char*)fileSystemRepresentation, maxSize))
    {
        return;
    }

    OSStatus status = FSPathMakeRefWithOptions(fileSystemRepresentation, kFSPathMakeRefDefaultOptions, &ref, NULL);
    if (status != noErr)
    {
        return;
    }

    FSCatalogInfo catalogInfo;
    FSCatalogInfoBitmap whichInfo = kFSCatInfoFinderInfo;

    OSErr err = FSGetCatalogInfo(&ref, whichInfo, &catalogInfo, NULL, NULL, NULL);
    if (err != noErr)
    {
        return;
    }

    ((FileInfo*)(catalogInfo.finderInfo))->finderFlags |= kIsInvisible;

    FSSetCatalogInfo(&ref, whichInfo, &catalogInfo);
}

void UBPlatformUtils::setFileType(const QString &filePath, unsigned long fileType)
{
    FSRef ref;
    CFStringRef path = CFStringCreateWithCharacters(0, reinterpret_cast<const UniChar *>(filePath.unicode()), filePath.length());
    const CFIndex maxSize = CFStringGetMaximumSizeOfFileSystemRepresentation(path);
    UInt8 fileSystemRepresentation[maxSize];
    CFRelease(path);
    if (!CFStringGetFileSystemRepresentation(path, (char*)fileSystemRepresentation, maxSize))
    {
        return;
    }

    OSStatus status = FSPathMakeRefWithOptions(fileSystemRepresentation, kFSPathMakeRefDefaultOptions, &ref, NULL);
    if (status != noErr)
    {
        return;
    }

    FSCatalogInfo catalogInfo;
    FSCatalogInfoBitmap whichInfo = kFSCatInfoFinderInfo;

    OSErr err = FSGetCatalogInfo(&ref, whichInfo, &catalogInfo, NULL, NULL, NULL);
    if (err != noErr)
    {
        return;
    }

    ((FileInfo*)(catalogInfo.finderInfo))->fileType = fileType;
    ((FileInfo*)(catalogInfo.finderInfo))->fileCreator = 'UniB';

    FSSetCatalogInfo(&ref, whichInfo, &catalogInfo);
}

static CGDisplayFadeReservationToken token = NULL;

void UBPlatformUtils::fadeDisplayOut()
{
    if (CGAcquireDisplayFadeReservation(1.2, &token) == kCGErrorSuccess)
    {
        CGDisplayFade(token, 1.0, kCGDisplayBlendNormal, kCGDisplayBlendSolidColor, 0.0, 0.0, 0.0, true);
    }
}

void UBPlatformUtils::fadeDisplayIn()
{
    if (CGAcquireDisplayFadeReservation(0.6, &token) == kCGErrorSuccess)
    {
        CGDisplayFade(token, 0.5, kCGDisplayBlendSolidColor,  kCGDisplayBlendNormal, 0.0, 0.0, 0.0, true);
    }
}


//QString UBPlatformUtils::preferredTranslation(QString pFilePrefix)
//{
//    QString qmPath;
//    NSString* filePrefix = [[NSString alloc] initWithUTF8String:(const char*)(pFilePrefix.toUtf8())];

//    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

//    NSString *lprojPath = [[[NSBundle mainBundle] pathForResource:@"Localizable" ofType:@"strings"] stringByDeletingLastPathComponent];
//    if (lprojPath)
//    {
//        NSString *lang = [[lprojPath lastPathComponent] stringByDeletingPathExtension];
//        NSString *translationFilePath = [lprojPath stringByAppendingPathComponent:[[filePrefix stringByAppendingString:lang] stringByAppendingPathExtension:@"qm"]];
//        qmPath = QString::fromUtf8([translationFilePath UTF8String], strlen([translationFilePath UTF8String]));
//    }

//    [pool drain];
//    return qmPath;
//}

//QString UBPlatformUtils::preferredLanguage()
//{
//    QFileInfo qmFileInfo = QFileInfo(preferredTranslation("sankore_"));
//    QDir lprojPath = qmFileInfo.dir();
//    QFileInfo lprojFileInfo = QFileInfo(lprojPath.absolutePath());
//    return lprojFileInfo.baseName();
//}

QStringList UBPlatformUtils::availableTranslations()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *lprojPath = [[NSBundle mainBundle] resourcePath];
    QString translationsPath =  QString::fromUtf8([lprojPath UTF8String], strlen([lprojPath UTF8String]));
    QStringList translationsList = UBFileSystemUtils::allFiles(translationsPath, false);
    QRegExp sankoreTranslationFiles(".*lproj");
    translationsList=translationsList.filter(sankoreTranslationFiles);
    [pool drain];
    return translationsList.replaceInStrings(QRegExp("(.*)/(.*).lproj"),"\\2");
}

QString UBPlatformUtils::translationPath(QString pFilePrefix, QString pLanguage)
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *lprojPath = [[NSBundle mainBundle] resourcePath];
    QString translationsPath =  QString::fromUtf8([lprojPath UTF8String], strlen([lprojPath UTF8String]));
    [pool drain];
    return translationsPath + "/" + pLanguage + ".lproj/" + pFilePrefix + pLanguage + ".qm";
}

QString UBPlatformUtils::systemLanguage()
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    NSUserDefaults* defs = [NSUserDefaults standardUserDefaults];
    NSArray* languages = [defs objectForKey:@"AppleLanguages"];
    NSString* preferredLang = [languages objectAtIndex:0];
    QString result = QString::fromUtf8([preferredLang UTF8String], strlen([preferredLang UTF8String]));
    [pool drain];
    return result;
}

void UBPlatformUtils::runInstaller(const QString &installerFilePath)
{
    UBApplication::setDisabled(true);

    // Save app config file to temp directory (will be restored at launch)
    QString appSettings = UBPlatformUtils::applicationResourcesDirectory() + "/etc/Uniboard.config";
    QString tmpSettings = QDir::tempPath() + "/Uniboard.config";
    QFile::remove(tmpSettings);
    QFile::copy(appSettings, tmpSettings);

    QString updateFilePath = QDir::tempPath() + "/upgrade.sh";

    QFile file(":/macx/upgrade.sh");
    QFile updateFile(updateFilePath);
    if (file.open(QIODevice::ReadOnly) && updateFile.open(QIODevice::WriteOnly))
    {
        QByteArray payload = file.readAll();

        updateFile.write(payload);
        updateFile.close();

        QString uniboardAndVersion = QApplication::applicationName()  + QString(" ") + QApplication::applicationVersion();

        QFileInfo fi(installerFilePath);
        uniboardAndVersion = fi.fileName().remove(".dmg");

        QString bundlePath;
        NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
        NSString *nssBundlePath = [[NSBundle mainBundle] bundlePath];

        bundlePath = QString::fromUtf8([nssBundlePath fileSystemRepresentation], strlen([nssBundlePath fileSystemRepresentation]));
        [pool drain];

        QString escaped = QString("/bin/sh \"%1\" \"%2\" \"%3\" \"%4\"")
                .arg(updateFilePath)
                .arg(uniboardAndVersion)
                .arg(installerFilePath)
                .arg(bundlePath);

        qDebug() << "Installing New Version" << escaped;

        QProcess process;
        bool success = process.startDetached(escaped);

        if(success)
           return;
    }

    // did not work .. lets load the dmg ...
    QDesktopServices::openUrl(QUrl::fromLocalFile(installerFilePath));

}


void UBPlatformUtils::bringPreviousProcessToFront()
{
    ProcessSerialNumber previousProcessPSN = {0, kNoProcess};

    CFArrayRef apps = CopyLaunchedApplicationsInFrontToBackOrder();
    if (apps != NULL)
    {
        if (CFArrayGetCount(apps) > 1)
        {
            SInt64 psn64;
            CFDictionaryRef appInfo = (CFDictionaryRef)CFArrayGetValueAtIndex(apps, 1);
            CFNumberRef psn = (CFNumberRef)CFDictionaryGetValue(appInfo, CFSTR("PSN"));
            if (psn != NULL)
            {
                CFNumberGetValue(psn, kCFNumberSInt64Type, &psn64);
                previousProcessPSN.highLongOfPSN = psn64 >> 32;
                previousProcessPSN.lowLongOfPSN  = psn64 & 0xFFFFFFFF;
            }
        }
        CFRelease(apps);
    }
    else
    {
        // On 10.4, we can't get the apps in front to back order, so we default to Finder
        OSStatus status;
        ProcessSerialNumber psn = {0, kNoProcess};
        while ((status = GetNextProcess(&psn)) == noErr)
        {
            CFDictionaryRef processInfo = ProcessInformationCopyDictionary(&psn, kProcessDictionaryIncludeAllInformationMask);
            if (processInfo != NULL)
            {
                CFStringRef bundleIdentifier = (CFStringRef)CFDictionaryGetValue(processInfo, kCFBundleIdentifierKey);
                if (bundleIdentifier && CFStringCompare(CFSTR("com.apple.finder"), bundleIdentifier, kCFCompareCaseInsensitive) == kCFCompareEqualTo)
                {
                    previousProcessPSN.highLongOfPSN = psn.highLongOfPSN;
                    previousProcessPSN.lowLongOfPSN  = psn.lowLongOfPSN;
                }
                CFRelease(processInfo);
            }
        }
    }

    SetFrontProcess(&previousProcessPSN);
}


QString UBPlatformUtils::osUserLoginName()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSString *nsUserName = NSUserName();
    QString userName = QString::fromUtf8([nsUserName UTF8String], strlen([nsUserName UTF8String]));

    [pool drain];
    return userName;
}

QString UBPlatformUtils::computerName()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSString *nsComputerName = [[NSHost currentHost] name];
    QString computerName = QString::fromUtf8([nsComputerName UTF8String], strlen([nsComputerName UTF8String]));

    [pool drain];
    return computerName;
}

void UBPlatformUtils::setWindowNonActivableFlag(QWidget* widget, bool nonAcivable)
{
    Q_UNUSED(widget);
    Q_UNUSED(nonAcivable);
}

QPixmap qpixmapFromIconRef(IconRef iconRef, int size) {
      OSErr result;
      int iconSize;
      OSType elementType;

      // Determine elementType and iconSize
      if (size <= 16) {
            elementType = kSmall32BitData;
            iconSize = 16;
      } else if (size <= 32) {
            elementType = kLarge32BitData;
            iconSize = 32;
      } else {
            elementType = kThumbnail32BitData;
            iconSize = 128;
      }

      // Get icon into an IconFamily
      IconFamilyHandle hIconFamily = 0;
      IconRefToIconFamily(iconRef, kSelectorAllAvailableData, &hIconFamily);

      // Extract data
      Handle hRawBitmapData = NewHandle(iconSize * iconSize * 4);
      result = GetIconFamilyData( hIconFamily, elementType, hRawBitmapData );
      if (result != noErr) {
            DisposeHandle(hRawBitmapData);
            return QPixmap();
      }

      // Convert data to QImage
      QImage image(iconSize, iconSize, QImage::Format_ARGB32);
      HLock(hRawBitmapData);
      unsigned long* data = (unsigned long*) *hRawBitmapData;
      for (int posy=0; posy<iconSize; ++posy, data+=iconSize) {
      #ifdef __BIG_ENDIAN__
            uchar* line = image.scanLine(posy);
            memcpy(line, data, iconSize * 4);
      #else
            uchar* src = (uchar*) data;
            uchar* dst = image.scanLine(posy);
            for (int posx=0; posx<iconSize; src+=4, dst+=4, ++posx) {
                  dst[0] = src[3];
                  dst[1] = src[2];
                  dst[2] = src[1];
                  dst[3] = src[0];
            }
      #endif
      }
      HUnlock(hRawBitmapData);
      DisposeHandle( hRawBitmapData );

      // Scale to wanted size
      image = image.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
      return QPixmap::fromImage(image);
}

QString QStringFromStringRef(CFStringRef stringRef)
{
   if (stringRef!=NULL)
        {
		char tmp[1024];
		CFStringGetCString(stringRef, tmp, 1024, 0);
		return QString(tmp);
	}
	else
		return QString();
}


KEYBT* createKeyBt(const UCKeyboardLayout* keyLayout, int vkk)
{
	UInt32 deadKeyState = 0L;
	UInt32 kbdType = kKeyboardISO;

	UniCharCount cnt1, cnt2;
	UniChar unicodeString1[100], unicodeString2[100];

	UCKeyTranslate(keyLayout, vkk, kUCKeyActionDisplay, 0, kbdType,  kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 100, &cnt1, unicodeString1);
	UCKeyTranslate(keyLayout, vkk, kUCKeyActionDisplay, (shiftKey >> 8) & 0xff, kbdType,  kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 100, &cnt2, unicodeString2);

	return new KEYBT(unicodeString1[0], vkk, unicodeString2[0], vkk);
}


void UBPlatformUtils::initializeKeyboardLayouts()
{
	CFStringRef keys[] = { kTISPropertyInputSourceCategory, kTISPropertyInputSourceIsEnableCapable, kTISPropertyInputSourceIsSelectCapable };
	const void* values[] = { kTISCategoryKeyboardInputSource, kCFBooleanTrue, kCFBooleanTrue };
	CFDictionaryRef dict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 3, NULL, NULL);
	CFArrayRef kbds = TISCreateInputSourceList(dict, false);

	int count = CFArrayGetCount(kbds);
	QList<UBKeyboardLocale*> result;

	for(int i=0; i<count; i++)
	{
		TISInputSourceRef keyLayoutRef =  (TISInputSourceRef)CFArrayGetValueAtIndex(kbds, i);
		if (keyLayoutRef==NULL)
			continue;

		CFDataRef ref = (CFDataRef) TISGetInputSourceProperty(keyLayoutRef,
					 kTISPropertyUnicodeKeyLayoutData);
		if (ref==NULL)
			continue;
		const UCKeyboardLayout* keyLayout = (const UCKeyboardLayout*) CFDataGetBytePtr(ref);
		if (keyLayoutRef==NULL)
			continue;

		KEYBT** keybt = new KEYBT*[SYMBOL_KEYS_COUNT];

		keybt[0] = createKeyBt(keyLayout, 10);
		keybt[1] = createKeyBt(keyLayout, 18);
		keybt[2] = createKeyBt(keyLayout, 19);
		keybt[3] = createKeyBt(keyLayout, 20);
		keybt[4] = createKeyBt(keyLayout, 21);
		keybt[5] = createKeyBt(keyLayout, 23);
		keybt[6] = createKeyBt(keyLayout, 22);
		keybt[7] = createKeyBt(keyLayout, 26);
		keybt[8] = createKeyBt(keyLayout, 28);
		keybt[9] = createKeyBt(keyLayout, 25);
		keybt[10] = createKeyBt(keyLayout, 29);
		keybt[11] = createKeyBt(keyLayout, 27);
		keybt[12] = createKeyBt(keyLayout, 24);

		keybt[13] = createKeyBt(keyLayout, 12);
		keybt[14] = createKeyBt(keyLayout, 13);
		keybt[15] = createKeyBt(keyLayout, 14);
		keybt[16] = createKeyBt(keyLayout, 15);
		keybt[17] = createKeyBt(keyLayout, 17);
		keybt[18] = createKeyBt(keyLayout, 16);
		keybt[19] = createKeyBt(keyLayout, 32);
		keybt[20] = createKeyBt(keyLayout, 34);
		keybt[21] = createKeyBt(keyLayout, 31);
		keybt[22] = createKeyBt(keyLayout, 35);
		keybt[23] = createKeyBt(keyLayout, 33);
		keybt[24] = createKeyBt(keyLayout, 30);

		keybt[25] = createKeyBt(keyLayout, 0);
		keybt[26] = createKeyBt(keyLayout, 1);
		keybt[27] = createKeyBt(keyLayout, 2);
		keybt[28] = createKeyBt(keyLayout, 3);
		keybt[29] = createKeyBt(keyLayout, 5);
		keybt[30] = createKeyBt(keyLayout, 4);
		keybt[31] = createKeyBt(keyLayout, 38);
		keybt[32] = createKeyBt(keyLayout, 40);
		keybt[33] = createKeyBt(keyLayout, 37);
		keybt[34] = createKeyBt(keyLayout, 41);
		keybt[35] = createKeyBt(keyLayout, 39);
		keybt[36] = createKeyBt(keyLayout, 42);

		keybt[37] = createKeyBt(keyLayout, 6);
		keybt[38] = createKeyBt(keyLayout, 7);
		keybt[39] = createKeyBt(keyLayout, 8);
		keybt[40] = createKeyBt(keyLayout, 9);
		keybt[41] = createKeyBt(keyLayout, 11);
		keybt[42] = createKeyBt(keyLayout, 45);
		keybt[43] = createKeyBt(keyLayout, 46);
		keybt[44] = createKeyBt(keyLayout, 43);
		keybt[45] = createKeyBt(keyLayout, 47);
		keybt[46] = createKeyBt(keyLayout, 44);


		CFStringRef sr = (CFStringRef) TISGetInputSourceProperty(keyLayoutRef, kTISPropertyInputSourceID);
		QString ID = QStringFromStringRef(sr);

		sr = (CFStringRef) TISGetInputSourceProperty(keyLayoutRef, kTISPropertyLocalizedName);
		QString fullName = QStringFromStringRef(sr);

                CFArrayRef langs = (CFArrayRef) TISGetInputSourceProperty(keyLayoutRef, kTISPropertyInputSourceLanguages);

                QString name = "??";
		if (CFArrayGetCount(langs)>0)
		{
			CFStringRef langRef = (CFStringRef)CFArrayGetValueAtIndex(langs, 0);
			name = QStringFromStringRef(langRef);
                        qDebug() << "name is " + name;

		}

                //IconRef iconRef = (IconRef)TISGetInputSourceProperty(kTISPropertyIconRef, kTISPropertyInputSourceLanguages);

                const QString resName = ":/images/flags/" + name + ".png";
                QIcon *iconLang = new QIcon(resName);

                result.append(new UBKeyboardLocale(fullName, name, ID, iconLang, keybt));
	}

	if (result.size()==0)
	{
		nKeyboardLayouts = 0;
		keyboardLayouts = NULL;
	}
	else
	{
		nKeyboardLayouts = result.size();
		keyboardLayouts = new UBKeyboardLocale*[nKeyboardLayouts];
		for(int i=0; i<nKeyboardLayouts; i++)
			keyboardLayouts[i] = result[i];
	}

}

void UBPlatformUtils::destroyKeyboardLayouts()
{}

QString UBPlatformUtils::urlFromClipboard()
{
    QString qsRet;
/*  
    // commented because Sankore crashes on Java Script. It seems to backends dependencies.
    NSPasteboard* pPasteboard = [NSPasteboard pasteboardWithName:@"Apple CFPasteboard drag"];
    WebArchive* pArchive = [[WebArchive alloc] initWithData:[pPasteboard dataForType:@"com.apple.webarchive"]];

    qsRet = [[[[pArchive mainResource] URL] absoluteString] UTF8String];

    [pArchive release];
*/
    return qsRet;
}
