/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "UniboardSankoreTransition.h"
#include "core/UBSettings.h"
#include "frameworks/UBDesktopServices.h"
#include "frameworks/UBFileSystemUtils.h"
#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

UniboardSankoreTransition::UniboardSankoreTransition(QObject *parent) :
    QObject(parent)
    , mTransitionDlg(NULL)
    , mThread(NULL)
{
    mOldSankoreDirectory = UBFileSystemUtils::normalizeFilePath(UBDesktopServices::storageLocation(QDesktopServices::DataLocation));

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

    if(mThread){
        delete mThread;
        mThread = NULL;
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
            mTransitionDlg = new UBUpdateDlg(NULL, fileInfoList.count(), backupDirectoryPath);
            connect(mTransitionDlg, SIGNAL(updateFiles()), this, SLOT(startDocumentTransition()));
            connect(this, SIGNAL(transitionFinished(bool)), mTransitionDlg, SLOT(onFilesUpdated(bool)));
            mTransitionDlg->show();
        }
    }
}

void UniboardSankoreTransition::startDocumentTransition()
{
    mThread = new UniboardSankoreThread(this);
    mThread->start();
    connect(this,SIGNAL(transitioningFile(QString)),mTransitionDlg,SLOT(transitioningFile(QString)));
}


bool UniboardSankoreTransition::checkPage(QString& sankorePagePath)
{
    QFile file(sankorePagePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray documentByteArray = file.readAll();
    file.close();

    QString sankoreDirectory = sankorePagePath.left(sankorePagePath.indexOf("/page"));
    ;
    sankoreDirectory = QUrl::fromLocalFile(sankoreDirectory).toString();
    QString documentString(documentByteArray);

    documentString.replace("xlink:href=\"videos/","xlink:href=\"" + sankoreDirectory + "/videos/");

    documentString.replace("xlink:href=\"objects/","xlink:href=\"" + sankoreDirectory + "/objects/");

    documentString.replace("xlink:href=\"audios/","xlink:href=\"" + sankoreDirectory + "/audios/");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    file.write(documentString.toAscii());
    file.close();

    return true;
}


bool UniboardSankoreTransition::checkWidget(QString& sankoreWidgetIndexPath)
{
    QFile file(sankoreWidgetIndexPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return false;

    QByteArray documentByteArray = file.readAll();
    file.close();

    QString documentString(documentByteArray);

    documentString.replace("/Uniboard/interactive content","/Sankore/interactive content");

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;

    file.write(documentString.toAscii());
    file.close();

    return true;
}


bool UniboardSankoreTransition::updateIndexWidget(QString& sankoreWidgetPath)
{
    bool result = true;
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(sankoreWidgetPath);

    QFileInfoList::iterator fileInfo;
    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end() && result; fileInfo += 1) {
        if (fileInfo->fileName().endsWith("wgt")){
            QString path = fileInfo->absolutePath() + "/" + fileInfo->fileName() + "/index.html";
            if (QFile(path).exists())
                result = checkWidget(path);

            path = fileInfo->absolutePath() + "/" + fileInfo->fileName() + "/index.htm";
            if (QFile(path).exists())
                result &= checkWidget(path);
        }
    }

    return result;
}

bool UniboardSankoreTransition::updateSankoreHRef(QString& sankoreDocumentPath)
{
    bool result = true;
    QFileInfoList fileInfoList = UBFileSystemUtils::allElementsInDirectory(sankoreDocumentPath);

    QFileInfoList::iterator fileInfo;
    QString sankoreDocumentDirectory = UBSettings::uniboardDocumentDirectory();

    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end() && result; fileInfo += 1) {
        if (fileInfo->fileName().endsWith("svg")){
            QString path = fileInfo->absolutePath() + "/" + fileInfo->fileName();
            result = checkPage(path);
        }
    }

    return result;
}

void UniboardSankoreTransition::executeTransition()
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

    for (fileInfo = fileInfoList.begin(); fileInfo != fileInfoList.end() && result; fileInfo += 1) {
        if (fileInfo->isDir() && (fileInfo->fileName().startsWith("Uniboard Document ") || fileInfo->fileName().startsWith("Sankore Document "))){
            QString sankoreDocumentName = fileInfo->fileName();
            emit transitioningFile(sankoreDocumentName);
            sankoreDocumentName.replace("Uniboard","Sankore");
            QString sankoreDocumentPath = sankoreDocumentDirectory + "/" + sankoreDocumentName;
            result = UBFileSystemUtils::copyDir(fileInfo->filePath(),sankoreDocumentPath);
            result &= updateSankoreHRef(sankoreDocumentPath);
            QString sankoreWidgetPath = sankoreDocumentDirectory +  "/" + sankoreDocumentName + "/widgets";
            result &= updateIndexWidget(sankoreWidgetPath);
        }
    }

    if (!result){
        qWarning() << "The transaction has failed";
        rollbackDocumentsTransition(fileInfoList);
        UBFileSystemUtils::deleteDir(backupDestinationPath);
    }
    else{
        QString sankoreInteractiveAppPath = sankoreDocumentDirectory;
        sankoreInteractiveAppPath = sankoreInteractiveAppPath.replace("document","") + "interactive content/";
        UBFileSystemUtils::copyDir(mOldSankoreDirectory + "/interactive content", sankoreInteractiveAppPath);
        UBFileSystemUtils::copyDir(mUniboardSourceDirectory + "/interactive content", sankoreInteractiveAppPath);
        UBFileSystemUtils::deleteDir(mOldSankoreDirectory);
        UBFileSystemUtils::deleteDir(mUniboardSourceDirectory);
    }

    emit transitionFinished(result);
}


UniboardSankoreThread::UniboardSankoreThread(QObject* parent):QThread(parent)
{

}

UniboardSankoreThread::~UniboardSankoreThread()
{

}

void UniboardSankoreThread::run()
{
    UniboardSankoreTransition* pTransition = dynamic_cast<UniboardSankoreTransition*>(parent());

    pTransition->executeTransition();
}

