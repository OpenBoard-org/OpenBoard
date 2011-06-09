#include "UniboardSankoreTransition.h"
#include "core/UBSettings.h"
#include "frameworks/UBDesktopServices.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

UniboardSankoreTransition::UniboardSankoreTransition(QObject *parent) :
    QObject(parent)
  , mTransitionDlg(NULL)
{
    mOldSankoreDirectory = UBFileSystemUtils::normalizeFilePath(UBDesktopServices::storageLocation(QDesktopServices::DataLocation));
    qDebug() << mOldSankoreDirectory;

    mUniboardSourceDirectory = UBFileSystemUtils::normalizeFilePath(UBDesktopServices::storageLocation(QDesktopServices::DataLocation));
#if defined(Q_WS_MACX)
    mOldSankoreDirectory.replace("Sankore/Sankore 3.1", "Sankore 3.1");
    mUniboardSourceDirectory.replace("Sankore/Sankore 3.1", "Uniboard");
#else
    mUniboardSourceDirectory.replace("Sankore/Sankore 3.1", "Mnemis/Uniboard");
#endif
    connect(this, SIGNAL(docAdded(UBDocumentProxy*)), UBPersistenceManager::persistenceManager(), SIGNAL(documentCreated(UBDocumentProxy*)));
}
UniboardSankoreTransition::~UniboardSankoreTransition()
{
    if(NULL != mTransitionDlg)
    {
        delete mTransitionDlg;
        mTransitionDlg = NULL;
    }
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
    if (QFileInfo(mUniboardSourceDirectory).exists() || QFileInfo(mOldSankoreDirectory).exists()){
        QString uniboardDocumentDirectory = mUniboardSourceDirectory + "/document";

        QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(uniboardDocumentDirectory);
        fileInfoList << UBFileSystemUtils::allElementsInDirectory(mOldSankoreDirectory + "/document");

        QString backupDirectoryPath = UBFileSystemUtils::normalizeFilePath(UBDesktopServices::storageLocation(QDesktopServices::DesktopLocation));

        if (fileInfoList.count() != 0){
            mTransitionDlg = new UBUpdateDlg(0, fileInfoList.count(), backupDirectoryPath);
            connect(mTransitionDlg, SIGNAL(updateFiles()), this, SLOT(startDocumentTransition()));
            connect(this, SIGNAL(transitionFinished(bool)), mTransitionDlg, SLOT(onFilesUpdated(bool)));
            mTransitionDlg->show();
        }
    }
}

void UniboardSankoreTransition::startDocumentTransition()
{
    bool result = false;
    QString backupDestinationPath = mTransitionDlg->backupPath() + "/OldSankoreAndUniboardVersionsBackup";
    result = UBFileSystemUtils::copyDir(mUniboardSourceDirectory + "/document", backupDestinationPath);
    result &= UBFileSystemUtils::copyDir(mOldSankoreDirectory + "/document", backupDestinationPath);

    QString uniboardDocumentDirectory = mUniboardSourceDirectory + "/document";
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(uniboardDocumentDirectory);
    fileInfoList.append(UBFileSystemUtils::allElementsInDirectory(mOldSankoreDirectory + "/document"));

    QFileInfoList::iterator fileInfo;
    QString sankoreDocumentDirectory = UBSettings::uniboardDocumentDirectory();

    QStringList qslNewDocs;

    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end() && result; fileInfo += 1) {
        if (fileInfo->isDir() && (fileInfo->fileName().startsWith("Uniboard Document ") || fileInfo->fileName().startsWith("Sankore Document "))){
            QString sankoreDocumentName = fileInfo->fileName();
            sankoreDocumentName.replace("Uniboard","Sankore");
            result = UBFileSystemUtils::copyDir(fileInfo->filePath(),sankoreDocumentDirectory + "/" + sankoreDocumentName);
            qslNewDocs << sankoreDocumentName;
        }
    }

    if (!result){
        qWarning() << "The transaction has failed";
        rollbackDocumentsTransition(fileInfoList);
        UBFileSystemUtils::deleteDir(backupDestinationPath);
    }
    else{
        UBFileSystemUtils::deleteDir(mOldSankoreDirectory);
        UBFileSystemUtils::deleteDir(mUniboardSourceDirectory);
    }

    emit transitionFinished(result);

    mTransitionDlg->hide();
}
