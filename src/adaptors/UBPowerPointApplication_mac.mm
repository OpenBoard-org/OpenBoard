
#include "UBPowerPointApplication_mac.h"

#include "core/UBApplication.h"
#include "core/UBDocumentManager.h"
#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#import <Foundation/NSAutoreleasePool.h>
#import <Carbon/Carbon.h>



UBPowerPointApplication::UBPowerPointApplication(QObject* parent)
: UBImportAdaptor(parent)
{
    // NOOP
}

UBPowerPointApplication::~UBPowerPointApplication()
{
    // NOOP
}

class AppleScriptThread : public QThread
{
    public:
        AppleScriptThread(NSAppleScript *appleScript, QObject *parent = 0)
            : QThread(parent)
            , mAppleScript(appleScript)
            , mError(nil)
        {
            // NOOP
        }

        ~AppleScriptThread()
        {
            if (mError)
            {
                [mError release];
            }
        }

        void run()
        {
            NSDictionary *error = nil;
            NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
            [mAppleScript executeAndReturnError:&error];
            mError = error ? [[NSDictionary alloc] initWithDictionary:error] : nil;
            [pool release];
        }

        NSDictionary* error()
        {
            return mError;
        }

    private:
        NSAppleScript *mAppleScript;
        NSDictionary *mError;
};


NSString* escapePath(const QString &filePath)
{
    QString escapedFilePath(filePath);
    escapedFilePath.replace("\"", "\\\"");
    QByteArray pathRepresentation = QFile::encodeName(escapedFilePath);
    return [[NSFileManager defaultManager] stringWithFileSystemRepresentation:pathRepresentation.constData() length:pathRepresentation.length()];
}


bool UBPowerPointApplication::generatePdfFromPptFile(const QString& pptFile, const QString& pOutputFile)
{
    bool result = true;
    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    QResource scriptResource(":/PowerPointImport.applescript");
    NSString *script = [[NSString alloc] initWithBytes:scriptResource.data() length:scriptResource.size() encoding:NSUTF8StringEncoding];
    QFileInfo pptFileInfo(pptFile);
    NSString *scriptSource = [NSString stringWithFormat:script, escapePath(pptFileInfo.fileName()), escapePath(pptFile), escapePath(pOutputFile)];
    [script release];
    NSAppleScript *pdfExportScript = [[NSAppleScript alloc] initWithSource:scriptSource];
    AppleScriptThread appleScriptThread(pdfExportScript);
    appleScriptThread.start();
    while (appleScriptThread.isRunning())
    {
        qApp->processEvents();
    }
    if (appleScriptThread.error())
    {
        const char* errorDescription = [[appleScriptThread.error() description] UTF8String];
        qWarning() << "PowerPoint import error:" << QString::fromUtf8(errorDescription, strlen(errorDescription));
        result = false;
    }
    [pdfExportScript release];
    [pool drain];
    return result;
}


QStringList UBPowerPointApplication::supportedExtentions()
{
    QStringList result;
    CFURLRef powerPointURL = NULL;

    if (LSFindApplicationForInfo(kLSUnknownCreator, CFSTR("com.microsoft.Powerpoint"), NULL, NULL, &powerPointURL) == noErr)
    {
        CFBundleRef powerPointBundle = CFBundleCreate(kCFAllocatorDefault, powerPointURL);
        if (powerPointBundle)
        {
            CFStringRef buildNumber = (CFStringRef)CFBundleGetValueForInfoDictionaryKey(powerPointBundle, CFSTR("MicrosoftBuildNumber"));
            if (buildNumber && (CFGetTypeID(buildNumber) == CFStringGetTypeID()))
            {
                int buildValue = CFStringGetIntValue(buildNumber);
                if (buildValue >= 80409)
                {
                    // PowerPoint 2008
                    result << "ppt" << "pptx" << "pptm" << "pps" << "ppsx" << "ppsm";
                }
                /*
                 else if (buildValue >= `Office 2004 MicrosoftBuildNumber`)
                 {
                 result << "ppt" << "pptm" << "pps" << "ppsm";
                 }
                 */
                else
                {
                    qWarning("Unsupported Microsoft PowerPoint version: %d", buildValue);
                }
            }
            else
            {
                qWarning("Invalid PowerPoint MicrosoftBuildNumber");
            }
            CFRelease(powerPointBundle);
        }
        else
        {
            qWarning("Microsoft PowerPoint bundle was not found");
        }
    }
    else
    {
        qWarning("Microsoft PowerPoint was not found");
    }

    if (powerPointURL)
    {
        CFRelease(powerPointURL);
    }

    return result;
}


QString UBPowerPointApplication::importFileFilter()
{
    QStringList extentions = supportedExtentions();
    if (extentions.count() > 0)
    {
        QString filter = "PowerPoint (";
        foreach (const QString ext, extentions)
        {
            filter.append("*.");
            filter.append(ext);
            filter.append(" ");
        }
        filter = filter.trimmed();
        filter.append(")");
        return filter;
    }
    else
    {
        return 0;
    }
}


UBDocumentProxy* UBPowerPointApplication::importFile(const QFile& pFile, const QString& pGroup)
{
    UBApplication::showMessage(tr("Converting PowerPoint file ..."), true);
    UBDocumentProxy* result = 0;

    QString tempDir = UBFileSystemUtils::createTempDir();
    QFileInfo sourceFileInfo(pFile);
    QString tempFile = tempDir + "/" + sourceFileInfo.baseName() + ".pdf";

    if (generatePdfFromPptFile(pFile.fileName(), tempFile))
    {
        UBApplication::showMessage(tr("PowerPoint import successful."));
        QFile tmp(tempFile);
        result = UBDocumentManager::documentManager()->importFile(tmp, pGroup);
    }
    else
    {
        UBApplication::showMessage(tr("PowerPoint import failed."));
    }
    
    UBFileSystemUtils::deleteDir(tempDir);
    return result;
}


bool UBPowerPointApplication::addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile)
{
    UBApplication::showMessage(tr("Converting PowerPoint file ..."), true);
    bool result = false;

    QString tempDir = UBFileSystemUtils::createTempDir();
    QFileInfo sourceFileInfo(pFile);
    QString tempFile = tempDir + "/" + sourceFileInfo.baseName() + ".pdf";

    if (generatePdfFromPptFile(pFile.fileName(), tempFile))
    {
        UBApplication::showMessage(tr("PowerPoint import successful."));
        QFile tmp(tempFile);
        result = UBDocumentManager::documentManager()->addFileToDocument(pDocument, tmp);
    }
    else
    {
        UBApplication::showMessage(tr("PowerPoint import failed."));
    }
    
    UBFileSystemUtils::deleteDir(tempDir);
    return result;
}

