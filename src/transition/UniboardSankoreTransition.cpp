#include "UniboardSankoreTransition.h"
#include "core/UBSettings.h"
#include "frameworks/UBDesktopServices.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBApplication.h"

UniboardSankoreTransition::UniboardSankoreTransition(QObject *parent) :
    QObject(parent)
{
    mUniboardSourceDirectory = UBFileSystemUtils::normalizeFilePath(UBDesktopServices::storageLocation(QDesktopServices::DataLocation));
#if defined(Q_WS_MACX)
    mUniboardSourceDirectory.replace("Sankore/Sankore 3.1", "Uniboard");
#else
    mUniboardSourceDirectory.replace("Sankore/Sankore 3.1", "Mnemis/Uniboard");
#endif
}
UniboardSankoreTransition::~UniboardSankoreTransition()
{
    delete mTransitionDlg;
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
    if (QFileInfo(mUniboardSourceDirectory).exists()){
        QString uniboardDocumentDirectory = mUniboardSourceDirectory + "/document";

        QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(uniboardDocumentDirectory);

        QString backupDirectoryPath = UBFileSystemUtils::normalizeFilePath(UBDesktopServices::storageLocation(QDesktopServices::DesktopLocation));

        mTransitionDlg = new UBUpdateDlg(0, fileInfoList.count(), backupDirectoryPath);
        connect(mTransitionDlg, SIGNAL(updateFiles()), this, SLOT(startDocumentTransition()));
        connect(this, SIGNAL(transitionFinished(bool)), mTransitionDlg, SLOT(onFilesUpdated(bool)));
        mTransitionDlg->show();
    }
}

void UniboardSankoreTransition::startDocumentTransition()
{
    bool result = false;
    QString backupDestinationPath = mTransitionDlg->backupPath() + "/UniboardBackup";
    result = UBFileSystemUtils::copyDir(mUniboardSourceDirectory, backupDestinationPath);

    QString uniboardDocumentDirectory = mUniboardSourceDirectory + "/document";
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(uniboardDocumentDirectory);

    QFileInfoList::iterator fileInfo;
    QString sankoreDocumentDirectory = UBSettings::uniboardDocumentDirectory();

    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end() && result; fileInfo += 1) {
        if (fileInfo->isDir() && fileInfo->fileName().startsWith("Uniboard Document ")){
            QString sankoreDocumentName = fileInfo->fileName();
            sankoreDocumentName.replace("Uniboard","Sankore");
            result = UBFileSystemUtils::copyDir(fileInfo->filePath(),sankoreDocumentDirectory + "/" + sankoreDocumentName);
        }
    }

    if (!result){
        qWarning() << "The transaction has failed";
        rollbackDocumentsTransition(fileInfoList);
        UBFileSystemUtils::deleteDir(backupDestinationPath);
    }
    else {
        UBFileSystemUtils::deleteDir(mUniboardSourceDirectory);
    }

    emit transitionFinished(result);

    mTransitionDlg->hide();
}
