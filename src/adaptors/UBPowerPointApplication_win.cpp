/*
 * UBPowerPointApplication.cpp
 *
 *  Created on: Dec 4, 2008
 *      Author: Luc
 */

#include "UBPowerPointApplication_win.h"

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBDocumentManager.h"
#include "core/UBPersistenceManager.h"

#include "UBImportVirtualPrinter.h"
#include "msppt.h"
#include "mso.h"
#include "windows.h"

UBPowerPointApplication::UBPowerPointApplication(QObject* parent)
    : UBImportAdaptor(parent)
    , mInit(false)
        , mHasException(false)
        , mSupportPpt(false)
        , mSupportPptX(false)
{
    // NOOP
}


UBPowerPointApplication::~UBPowerPointApplication()
{
    // NOOP
}


void UBPowerPointApplication::init()
{
    PowerPoint::Application ppt;
    qDebug() << "PPT version :" << ppt.Version().toFloat();
    mSupportPpt = !ppt.isNull();
    mSupportPptX = ppt.Version().toFloat() >= 12;
    mInit = true;
}


bool UBPowerPointApplication::isPowerPointInstalled()
{
    if (!mInit)
    {
        init();
    }
    return mSupportPpt;
}


bool UBPowerPointApplication::supportPptx()
{
    if (!mInit)
    {
        init();
    }

    return mSupportPptX;
}


bool UBPowerPointApplication::generatePdfFromPptFile(const QString& pptFile, const QString& outputDir)
{
    Q_UNUSED(pptFile);
    Q_UNUSED(outputDir);
    return false;
}


bool UBPowerPointApplication::generateImagesFromPptFile(const QString& pptFile, const QString& outputDir, const QString& imageFormat, const QSize& imageSize)
{
    mHasException = false;

    PowerPoint::Application ppt;

    connect(&ppt, SIGNAL(exception ( int , const QString & , const QString & , const QString & ))
        , this, SLOT(exception ( int , const QString & , const QString & , const QString & )));

    if (ppt.isNull())
        return false;

    ppt.Activate();
    ppt.SetWindowState(PowerPoint::ppWindowMinimized);

    UBApplication::processEvents();

    int previouslyOpenPresentations = ppt.Presentations()->Count();

    PowerPoint::Presentation *presentation =
        ppt.Presentations()->Open(QDir::toNativeSeparators(pptFile));

    int currentOpenPresentations = ppt.Presentations()->Count();

    if(!presentation)
        return false;

    if (ppt.Version().toFloat() >= 12) // PPT 2007 is broken with high res exports : https://trac.assembla.com/uniboard/ticket/297#comment:16
    {
        presentation->Export(QDir::toNativeSeparators(outputDir), imageFormat);
    }
    else
    {
        presentation->Export(QDir::toNativeSeparators(outputDir), imageFormat, imageSize.width(), imageSize.height());
    }

    if(mHasException)
        return false;

    if (currentOpenPresentations != previouslyOpenPresentations)
        presentation->Close();

    if (ppt.Presentations()->Count() == 0)
        ppt.Quit();

    return true;
}

QStringList UBPowerPointApplication::supportedExtentions()
{
    QStringList result;
    if (UBPowerPointApplication::isPowerPointInstalled())
    {
        result << QStringList("ppt") << "pps";
        if (UBPowerPointApplication::supportPptx())
        {
            result << "pptx" << "pptm" << "ppsx" << "ppsm";
        }
    }

    return result;
}

QString UBPowerPointApplication::importFileFilter()
{
    if (UBPowerPointApplication::isPowerPointInstalled())
    {
        if (UBPowerPointApplication::supportPptx())
        {
            return "PowerPoint (*.ppt *.pptx *.pptm *.pps *.ppsx *.ppsm)";
        }
        else
        {
            return "PowerPoint (*.ppt *.pps)";
        }
    }
    else
    {
        return "";
    }
}

UBDocumentProxy* UBPowerPointApplication::importFile(const QFile& pFile, const QString& pGroup)
{

    UBDocumentProxy* document = 0;
    // print by changing default printer and use shell execute to print
    LPTSTR wDefaultPrinterName = new TCHAR[255];
    LPTSTR virtualPrinter = new TCHAR[255];

    int i = QString("Uniboard").toWCharArray(virtualPrinter); // TODO UB 4.x make me configurable ....
    virtualPrinter[i] = 0;
    DWORD bufferSize = 1000;
    GetDefaultPrinter(wDefaultPrinterName, &bufferSize);
    UBImportVirtualPrinter::sOriginalDefaultPrintername = QString::fromWCharArray(wDefaultPrinterName);

    if (!SetDefaultPrinter(virtualPrinter))
    {
        QMessageBox msgBox(0);
        msgBox.setText(tr("Uniboard printer is not installed. Import will be done in jpg format."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        QString tempLocation = UBFileSystemUtils::createTempDir();
        QDir tempDir(tempLocation);

        bool pptSuccess = generateImagesFromPptFile(pFile.fileName(), tempLocation, "jpg", QSize(3000, 2250));

        if (pptSuccess)
        {
            document = UBDocumentManager::documentManager()->importDir(tempDir, pGroup);

            if (document)
            {
                UBFileSystemUtils::deleteDir(tempLocation);
            }
        }
    }
    else
    {
        document = UBPersistenceManager::persistenceManager()->createDocument(pGroup);

        QFileInfo sourceFileInfo(pFile);

        document->setMetaData(UBSettings::documentName, sourceFileInfo.baseName());
        UBPersistenceManager::persistenceManager()->persistDocumentMetadata(document);

        UBImportVirtualPrinter::pendingDocument = document;

        int result = (int)ShellExecute(NULL, QString("print").utf16() , pFile.fileName().utf16(), NULL, NULL, SW_HIDE);

        qDebug() << "PPT shellexec print result" << result;
    }

    delete[] wDefaultPrinterName;
    delete[] virtualPrinter;

    return document;
}

bool UBPowerPointApplication::addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile)
{
    bool result = false;
    // print by changing default printer and use shell execute to print
    LPTSTR wDefaultPrinterName = new TCHAR[255];
    LPTSTR virtualPrinter = new TCHAR[255];

    int i = QString("Uniboard").toWCharArray(virtualPrinter); // TODO UB 4.x make me configurable ....
    virtualPrinter[i] = 0;
    DWORD bufferSize = 1000;
    GetDefaultPrinter(wDefaultPrinterName, &bufferSize);
    UBImportVirtualPrinter::sOriginalDefaultPrintername = QString::fromWCharArray(wDefaultPrinterName);

    if (!SetDefaultPrinter(virtualPrinter))
    {
        QMessageBox msgBox(0);
        msgBox.setText(tr("Uniboard printer is not installed. Import will be done in jpg format."));
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        msgBox.exec();
        QString tempLocation = UBFileSystemUtils::createTempDir();
        QDir tempDir(tempLocation);

        bool pptSuccess = generateImagesFromPptFile(pFile.fileName(), tempLocation, "jpg", QSize(3000, 2250));

        if (pptSuccess)
        {
                        if (UBDocumentManager::documentManager()->addImageDirToDocument(tempDir, pDocument))
            {
                UBFileSystemUtils::deleteDir(tempLocation);
                result = true;
            }
        }
    }
    else
    {
        UBImportVirtualPrinter::pendingDocument = pDocument;
        result = ShellExecute(NULL, QString("print").utf16() , pFile.fileName().utf16(), NULL, NULL, SW_HIDE);
    }

    delete[] wDefaultPrinterName;
    delete[] virtualPrinter;
    return result;

}

void UBPowerPointApplication::exception ( int code, const QString & source, const QString & desc, const QString & help )
{
    Q_UNUSED(help);
    mHasException = true;
    qCritical() << source << desc << code;
}

