#include <QDir>
#include "UBImportCFF.h"
#include "document/UBDocumentProxy.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBDocumentManager.h"

#include "domain/UBGraphicsPDFItem.h"

#include "pdf/PDFRenderer.h"

#include "core/memcheck.h"

#include "core/UBPersistenceManager.h"

#include "UBCFFSubsetAdaptor.h"

#include "quazip.h"
#include "quazipfile.h"
#include "quazipfileinfo.h"

UBImportCFF::UBImportCFF(QObject *parent)
    : UBImportAdaptor(parent)
{
    // NOOP
}


UBImportCFF::~UBImportCFF()
{
    // NOOP
}


QStringList UBImportCFF::supportedExtentions()
{
    return QStringList("iwb");
}


QString UBImportCFF::importFileFilter()
{
    QString filter = tr("Common File Format (");
    QStringList formats = supportedExtentions();
    bool isFirst = true;

    foreach(QString format, formats)
    {
            if(isFirst)
                    isFirst = false;
            else
                    filter.append(" ");

        filter.append("*."+format);
    }

    filter.append(")");

    return filter;
}


bool UBImportCFF::addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile)
{
	//TODO add cff file import here
/*    int res = UBDocumentManager::documentManager()->addImageAsPageToDocument(QStringList(QFileInfo(pFile).absoluteFilePath()), pDocument);
    if (res == 0)
    {
        UBApplication::showMessage(tr("Image import failed."));
        return false;
    }
    else
    {
        UBApplication::showMessage(tr("Image import successful."));
        return true;
    }*/
    return true;
}

QString UBImportCFF::expandFileToDir(const QFile& pZipFile, const QString& pDir)
{

    QDir rootDir(pDir);
    QuaZip zip(pZipFile.fileName());

    if(!zip.open(QuaZip::mdUnzip))
    {
        qWarning() << "Import failed. Cause zip.open(): " << zip.getZipError();
        return "";
    }

    zip.setFileNameCodec("UTF-8");
    QuaZipFileInfo info;
    QuaZipFile file(&zip);

    // TODO UB 4.x  implement a mechanism that can replace an existing
    // document based on the UID of the document.
    bool createNewDocument = true;
    QString documentRootFolder;

    // first we search the metadata.rdf to check the document properties
    for(bool more = zip.goToFirstFile(); more; more = zip.goToNextFile())
    {
        if(!zip.getCurrentFileInfo(&info))
        {
            qWarning() << "Import failed. Cause: getCurrentFileInfo(): " << zip.getZipError();
            return "";
        }

        QFileInfo currentFileInfo(pDir + "/" + file.getActualFileName());
    }

    if (createNewDocument)
        documentRootFolder = UBPersistenceManager::persistenceManager()->generateUniqueDocumentPath();


    QFile out;
    char c;
    for(bool more=zip.goToFirstFile(); more; more=zip.goToNextFile())
    {
        if(!zip.getCurrentFileInfo(&info))
        {
            //TOD UB 4.3 O display error to user or use crash reporter
            qWarning() << "Import failed. Cause: getCurrentFileInfo(): " << zip.getZipError();
            return "";
        }

        if(!file.open(QIODevice::ReadOnly))
        {
            qWarning() << "Import failed. Cause: file.open(): " << zip.getZipError();
            return "";
        }

        if(file.getZipError()!= UNZ_OK)
        {
            qWarning() << "Import failed. Cause: file.getFileName(): " << zip.getZipError();
            return "";
        }

        QString newFileName = documentRootFolder + "/" + file.getActualFileName();
        QFileInfo newFileInfo(newFileName);
        rootDir.mkpath(newFileInfo.absolutePath());

        out.setFileName(newFileName);
        out.open(QIODevice::WriteOnly);

        // Slow like hell (on GNU/Linux at least), but it is not my fault.
        // Not ZIP/UNZIP package's fault either.
        // The slowest thing here is out.putChar(c).
        QByteArray outFileContent = file.readAll();
        if (out.write(outFileContent) == -1)
        {
            qWarning() << "Import failed. Cause: Unable to write file";
            out.close();
            return "";
        }

        while(file.getChar(&c))
            out.putChar(c);

        out.close();

        if(file.getZipError()!=UNZ_OK)
        {
            qWarning() << "Import failed. Cause: " << zip.getZipError();
            return "";
        }

        if(!file.atEnd())
        {
            qWarning() << "Import failed. Cause: read all but not EOF";
            return "";
        }

        file.close();

        if(file.getZipError()!=UNZ_OK)
        {
            qWarning() << "Import failed. Cause: file.close(): " <<  file.getZipError();
            return "";
        }

    }

    zip.close();

    if(zip.getZipError()!=UNZ_OK)
    {
      qWarning() << "Import failed. Cause: zip.close(): " << zip.getZipError();
      return "";
    }


    return documentRootFolder;
}


UBDocumentProxy* UBImportCFF::importFile(const QFile& pFile, const QString& pGroup)
{
    Q_UNUSED(pGroup); // group is defined in the imported file

    QFileInfo fi(pFile);
    UBApplication::showMessage(tr("Importing file %1...").arg(fi.baseName()), true);

    // first unzip the file to the correct place
    //TODO create temporary path for iwb file content
    QString path = QDir::tempPath();

    QString documentRootFolder = expandFileToDir(pFile, path);
    if (documentRootFolder.isEmpty())
        documentRootFolder = pFile.fileName();
    else
        documentRootFolder = path.append("\\content.xml");

    if(!documentRootFolder.length()){
            UBApplication::showMessage(tr("Import of file %1 failed.").arg(fi.baseName()));
            return 0;
    }
    else{
        //TODO convert expanded CFF file content to ubz destination folder
        QString temporaryFolder = QDir::tempPath();
        UBCFFSubsetAdaptor::ConvertCFFFileToUbz(documentRootFolder, temporaryFolder);
        // create document proxy for destination folder and return
        UBDocumentProxy* newDocument = UBPersistenceManager::persistenceManager()->createDocumentFromDir(temporaryFolder);
        UBApplication::showMessage(tr("Import successful."));
        return newDocument;
    }
}
