/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
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



#include "MacUtils.h"
#include "UBPlatformUtils.h"
#include "core/UBApplication.h"
#include "core/UBDisplayManager.h"
#include "core/UBSettings.h"
#include "frameworks/UBFileSystemUtils.h"
#include "gui/UBMainWindow.h"

#include <QWidget>
#include <QRegularExpression>

#import <Foundation/NSAutoreleasePool.h>
#import <Cocoa/Cocoa.h>
#import <Carbon/Carbon.h>

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

//void *originalSetSystemUIMode = 0;

void UBPlatformUtils::init()
{
    initializeKeyboardLayouts();

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];

    NSString *currentPath      = [[NSBundle mainBundle] pathForResource:@"Save PDF to OpenBoard" ofType:@"workflow"];
    NSString *installedPath    = [[[@"~/Library/PDF Services" stringByExpandingTildeInPath] stringByAppendingPathComponent:@"Save PDF to OpenBoard"] stringByAppendingPathExtension:@"workflow"];
    NSString *currentVersion   = bundleShortVersion([NSBundle bundleWithPath:currentPath]);
    NSString *installedVersion = bundleShortVersion([NSBundle bundleWithPath:installedPath]);

    if (![installedVersion isEqualToString:currentVersion])
    {
        NSFileManager *fileManager = [NSFileManager defaultManager];
        [fileManager removeFileAtPath:installedPath handler:nil];
        // removing the old version of the script named Save PDF to Uniboard
        [fileManager removeFileAtPath:[[[@"~/Library/PDF Services" stringByExpandingTildeInPath] stringByAppendingPathComponent:@"Save PDF to OpenBoard"] stringByAppendingPathExtension:@"workflow"] handler:nil];
        [fileManager createDirectoryAtPath:[installedPath stringByDeletingLastPathComponent] attributes:nil];
        BOOL copyOK = [fileManager copyPath:currentPath toPath:installedPath handler:nil];
        if (!copyOK)
        {
            qWarning("Could not install the 'Save PDF to OpenBoard workflow");
        }
    }

    [pool drain];
}


void UBPlatformUtils::hideMenuBarAndDock()
{

    @try {
#if defined(Q_OS_OSX) && (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
        // temporarily disabled due to bug: when switching to desktop mode (and calling this),
        // openboard switches right back to the board mode. clicking again on desktop mode works.
        /*if (desktop) {
            [NSApp setPresentationOptions:NSApplicationPresentationAutoHideMenuBar | NSApplicationPresentationAutoHideDock];
        }
        else*/
            [NSApp setPresentationOptions:NSApplicationPresentationHideMenuBar | NSApplicationPresentationHideDock];
#else // QT_VERSION_CHECK(5, 10, 0)
        [NSApp setPresentationOptions:NSApplicationPresentationHideMenuBar | NSApplicationPresentationHideDock];
#endif // QT_VERSION_CHECK(5, 10, 0)
    }

    @catch(NSException * exception) {
        qDebug() << "Error setting presentation options";
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

QString UBPlatformUtils::applicationEtcDirectory()
{
    return applicationResourcesDirectory() + "/etc";
}

QString UBPlatformUtils::applicationTemplateDirectory()
{
    return applicationResourcesDirectory() + "/etc";
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
    if (!CFStringGetFileSystemRepresentation(path, (char*)fileSystemRepresentation, maxSize))
    {
        return;
    }

    CFRelease(path);
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

bool UBPlatformUtils::hasSystemOnScreenKeyboard()
{
    return true;
}

QStringList UBPlatformUtils::availableTranslations()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSString *lprojPath = [[NSBundle mainBundle] resourcePath];
    QString translationsPath =  QString::fromUtf8([lprojPath UTF8String], strlen([lprojPath UTF8String]));
    QStringList translationsList = UBFileSystemUtils::allFiles(translationsPath, false);
    QRegularExpression sankoreTranslationFiles(".*lproj");
    translationsList=translationsList.filter(sankoreTranslationFiles);
    [pool drain];
    return translationsList.replaceInStrings(QRegularExpression("(.*)/(.*).lproj"),"\\2");
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
    UniChar unicodeString1[100], unicodeString2[100], unicodeString3[100];

    UCKeyTranslate(keyLayout, vkk, kUCKeyActionDisplay, 0, kbdType,  kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 100, &cnt1, unicodeString1);
    UCKeyTranslate(keyLayout, vkk, kUCKeyActionDisplay, (shiftKey >> 8) & 0xff, kbdType,  kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 100, &cnt2, unicodeString2);
    UCKeyTranslate(keyLayout, vkk, kUCKeyActionDisplay, (alphaLock >> 8) & 0xff, kbdType,  kUCKeyTranslateNoDeadKeysBit, &deadKeyState, 100, &cnt2, unicodeString3);

    return new KEYBT(unicodeString1[0], unicodeString2[0], unicodeString1[0] != unicodeString3[0], 0,0, KEYCODE(0, vkk, 0), KEYCODE(0, vkk, 1));
}


void UBPlatformUtils::initializeKeyboardLayouts()
{
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    CFStringRef keys[] = { kTISPropertyInputSourceCategory, kTISPropertyInputSourceIsEnableCapable, kTISPropertyInputSourceIsSelectCapable };
    const void* values[] = { kTISCategoryKeyboardInputSource, kCFBooleanTrue, kCFBooleanTrue };
    CFDictionaryRef dict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 3, NULL, NULL);
    CFArrayRef kbds = TISCreateInputSourceList(dict, false);

    int count = CFArrayGetCount(kbds);
    QList<UBKeyboardLocale*> result;

    qDebug() << "initializeKeyboardLayouts";
    qDebug() << "Found system locales: " << count;

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
        QString fullName = QString::fromUtf8([sr UTF8String], strlen([sr UTF8String]));

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

        qDebug() << "Locale: " << ID << ", name: " << name;

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
    [pool drain];
}

void UBPlatformUtils::destroyKeyboardLayouts()
{}

QString UBPlatformUtils::urlFromClipboard()
{
    QString qsRet;

    return qsRet;
}

void UBPlatformUtils::toggleFinder(const bool on)
{
#if defined(Q_OS_OSX) && (QT_VERSION >= QT_VERSION_CHECK(5, 10, 0))
    if (on)
    {
        [NSApp setPresentationOptions:NSApplicationPresentationDefault];
    }
    else
    {
        [NSApp setPresentationOptions:NSApplicationPresentationHideMenuBar | NSApplicationPresentationHideDock];
    }
#else // QT_VERSION_CHECK(5, 10, 0)
    Q_UNUSED(on);
#endif //QT_VERSION_CHECK(5, 10, 0)
}


void UBPlatformUtils::SetMacLocaleByIdentifier(const QString& id)
{
    @autoreleasepool {
        // convert id from QString to CFString
        // TODO: clean this up
        const QByteArray utf8 = id.toUtf8();
        const char* cString = utf8.constData();
        NSString * ns = [[NSString alloc] initWithUTF8String:cString];

        CFStringRef iName = (__bridge CFStringRef)ns;



        CFStringRef keys[] = { kTISPropertyInputSourceID };
        CFStringRef values[] = { iName };
        CFDictionaryRef dict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 1, NULL, NULL);

        // get list of current enabled keyboard layouts. dict filters the list
        // false specifies that we search only through the active input sources
        CFArrayRef kbds = TISCreateInputSourceList(dict, false);

        if (kbds && CFArrayGetCount(kbds) == 0)
            // if not found in the active sources, we search again through all sources installed
            kbds = TISCreateInputSourceList(dict, true);

        if (kbds && CFArrayGetCount(kbds)!=0) {
            TISInputSourceRef klRef =  (TISInputSourceRef)CFArrayGetValueAtIndex(kbds, 0);
            if (klRef!=NULL)
                TISSelectInputSource(klRef);
        }
    }
}

/**
 * @brief Activate the current application
 */
void UBPlatformUtils::setFrontProcess()
{
    NSRunningApplication* app = [NSRunningApplication currentApplication];

    // activate the application, forcing focus on it
    [app activateWithOptions: NSApplicationActivateIgnoringOtherApps];

    // other option: NSApplicationActivateAllWindows. This won't steal focus from another app, e.g
    // if the user is doing something else while waiting for OpenBoard to load
}


/**
 * @brief Full-screen a QWidget. Specific behaviour is platform-dependent.
 * @param pWidget the QWidget to maximize
 */
void UBPlatformUtils::showFullScreen(QWidget *pWidget)
{
    /* OpenBoard is designed to be run in "kiosk mode", i.e full screen. 
     * On OS X, we want to maximize the application while hiding the dock and menu bar, 
     * rather than use OS X's native full screen mode (which places the window on a new desktop). 
     * However, Qt's default behaviour when full-screening a QWidget is to set the dock and menu bar to auto-hide.
     * Since it is impossible to later set different presentation options (i.e Hide dock & menu bar)
     * to NSApplication, we have to avoid calling QWidget::showFullScreen on OSX.
    */

    if (UBSettings::settings()->appRunInWindow->get().toBool() &&
            pWidget == UBApplication::displayManager->widget(ScreenRole::Control)) {
        pWidget->show();
    } else {
        pWidget->showMaximized();

        /* Bit of a hack. On OS X 10.10, showMaximized() resizes the widget to full screen (if the dock and
         * menu bar are hidden); but on 10.9, it is placed in the "available" screen area (i.e the
         * screen area minus the menu bar and dock area). So we have to manually resize it to the
         * total screen height, and move it up to the top of the screen (y=0 position). */

        QRect currentScreenRect = QGuiApplication::screenAt(pWidget->geometry().topLeft())->geometry();
        pWidget->resize(currentScreenRect.width(), currentScreenRect.height());
        pWidget->move(currentScreenRect.left(), currentScreenRect.top());
    }
}


void UBPlatformUtils::showOSK(bool show)
{
    if (QOperatingSystemVersion::current().majorVersion() == 10 && QOperatingSystemVersion::current().minorVersion() < 15) /* < Catalina */
    {
        @autoreleasepool {
            CFDictionaryRef properties = (CFDictionaryRef)[NSDictionary
                          dictionaryWithObject: @"com.apple.KeyboardViewer"
                          forKey: (NSString *)kTISPropertyInputSourceID];

            NSArray *sources = (NSArray *)TISCreateInputSourceList(properties, true);

            if ([sources count] > 0) {
                TISInputSourceRef osk = (TISInputSourceRef)[sources objectAtIndex: 0];

                OSStatus result;
                if (show) {
                    TISEnableInputSource(osk);
                    result = TISSelectInputSource(osk);
                }
                else {
                    TISDisableInputSource(osk);
                    result = TISDeselectInputSource(osk);
                }

                if (result == paramErr) {
                    qWarning() << "Unable to select input source";
                    UBApplication::showMessage(tr("Unable to activate system on-screen keyboard"));
                }
            }

            else {
                qWarning() << "System OSK not found";
                UBApplication::showMessage(tr("System on-screen keyboard not found"));
            }
        }
    }
    else
    {
        NSString *source =
            @"tell application \"System Events\"\n\
                if application process \"TextInputMenuAgent\" exists then\n\
                    tell application process \"TextInputMenuAgent\"\n\
                        tell menu bar item 1 of menu bar 2\n\
                            ignoring application responses\n\
                                click\n\
                                delay 0.5\n\
                            end ignoring\n\
                        end tell\n\
                    end tell\n\
                end if\n\
            end tell\n\
            do shell script \"killall 'System Events'\"\n";

            source = [source stringByAppendingString:@"if application \"Assistive Control\" is"];

            if (show)
            {
                source = [source stringByAppendingString:@" not"];
            }

            source = [source stringByAppendingString:@" running then\n\
                tell application \"System Events\"\n\
                    tell application process \"TextInputMenuAgent\"\n\
                        tell menu 1 of menu bar item 1 of menu bar 2\n\
                            set nbItems to count menu items\n\
                            if (nbItems = 4)\n\
                                -- only one language so items are\n\
                                -- 1. emojis&symbols n-2. keyboard n-1. separator n.preferences\n\
                                click menu item (nbItems-2)\n\
                            else\n\
                                -- items are ... n-4. access keyboard n-3. separator n-2 display names n-1. separator n. preferences\n\
                                -- target is in fourth position from bottom\n\
                                click menu item (nbItems - 4)\n\
                            end if\n\
                        end tell\n\
                    end tell\n\
                end tell\n\
            end if"];

        NSAppleScript *script = [[[NSAppleScript alloc] initWithSource:source] autorelease];
        NSDictionary  *errorInfo   = nil;
        [script executeAndReturnError:&errorInfo];

        if(errorInfo!=nil)
        {
            errorOpeningVirtualKeyboard = true;

            NSAlert *alert = [[NSAlert alloc] init];

            if (alert != nil)
            {
                alert.messageText = errorInfo.allValues[0];
                [alert runModal];
                [alert release];

                //restore action state to previous one as it failed
                if (show)
                    UBApplication::mainWindow->actionVirtualKeyboard->setChecked(false);
                else
                    UBApplication::mainWindow->actionVirtualKeyboard->setChecked(true);
            }
        }
        else
        {
            errorOpeningVirtualKeyboard = false;
        }
    }
}

void UBPlatformUtils::grabScreen(QScreen* screen, std::function<void (QPixmap)> callback, QRect rect)
{
    QPixmap pixmap = screen->grabWindow(0, rect.x(), rect.y(), rect.width(), rect.height());
    callback(pixmap);
}
