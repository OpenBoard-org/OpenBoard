#include "UniboardSankoreTransition.h"
#include "core/UBSettings.h"
#include "frameworks/UBDesktopServices.h"
#include "frameworks/UBFileSystemUtils.h"

UniboardSankoreTransition::UniboardSankoreTransition(QObject *parent) :
    QObject(parent)
{
    mUniboardSourceDirectory = UBDesktopServices::storageLocation(QDesktopServices::DataLocation);
    mUniboardSourceDirectory.replace("Sankore/Sankore 3.1", "Mnemis/Uniboard");
}

bool UniboardSankoreTransition::backupUniboardDirectory()
{
    bool result = false;
    QString destinationDirectory =  UBDesktopServices::storageLocation(QDesktopServices::DesktopLocation);
    if(QFileInfo(destinationDirectory).exists() && QFileInfo(mUniboardSourceDirectory).exists()){
         if(!destinationDirectory.endsWith("/")) destinationDirectory += "/";
         result = UBFileSystemUtils::copyDir(mUniboardSourceDirectory, destinationDirectory + QFileInfo(mUniboardSourceDirectory).fileName() + "BackupData/");
         if(result) documentTransition();
    }

    return result;
}


void UniboardSankoreTransition::rollbackDocumentsTransition(QFileInfoList& fileInfoList)
{
    QFileInfoList::iterator fileInfo;
    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end(); fileInfo += 1) {
        if (fileInfo->isDir() && fileInfo->fileName().startsWith("Uniboard Document ")){
            QString sankoreDocumentName = fileInfo->fileName();
            sankoreDocumentName.replace("Uniboard","Sankore");
            QString sankoreDocumentDirectoryPath = UBSettings::uniboardDocumentDirectory() + "/" + sankoreDocumentName;
            if (QFileInfo(sankoreDocumentDirectoryPath).exists()){
                UBFileSystemUtils::deleteDir(sankoreDocumentDirectoryPath);
            }
        }
    }
}

void UniboardSankoreTransition::documentTransition()
{
    QString uniboardDocumentDirectory = mUniboardSourceDirectory + "/document";
    QString sankoreDocumentDirectory = UBSettings::uniboardDocumentDirectory();

    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(uniboardDocumentDirectory);

    QFileInfoList::iterator fileInfo;
    bool result = true;
    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end() && result; fileInfo += 1) {
        if (fileInfo->isDir() && fileInfo->fileName().startsWith("Uniboard Document ")){
            QString sankoreDocumentName = fileInfo->fileName();
            sankoreDocumentName.replace("Uniboard","Sankore");
            result = UBFileSystemUtils::copyDir(fileInfo->filePath(),sankoreDocumentDirectory + "/" + sankoreDocumentName);
        }
    }

    if (!result){
        qWarning() << "The transaction has failed during the copy of the " + fileInfo->filePath() + " document.";
        rollbackDocumentsTransition(fileInfoList);
    }
    else {
        UBFileSystemUtils::deleteDir(mUniboardSourceDirectory);
    }
}