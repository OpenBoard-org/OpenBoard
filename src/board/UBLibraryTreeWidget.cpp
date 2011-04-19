/*
 * UBLibraryTreeWidget.cpp
 *
 *  Created on: 28 juil. 2009
 *      Author: Luc
 */

#include "UBLibraryTreeWidget.h"

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "board/UBBoardController.h"

#include "network/UBHttpGet.h"

#include "domain/UBW3CWidget.h"

#include "UBLibraryController.h"


UBLibraryTreeWidget::UBLibraryTreeWidget(QWidget * parent)
    : QTreeWidget(parent)
    , mDropFolderItem(0)
{
    setDragDropMode(QAbstractItemView::DropOnly);
    setAcceptDrops(true);
    viewport()->setAcceptDrops(true);
    setDropIndicatorShown(true);
}


UBLibraryTreeWidget::~UBLibraryTreeWidget()
{
    // NOOP
}

bool UBLibraryTreeWidget::supportMimeData(const QMimeData* pMimeData)
{
    foreach(QString mimeType, mMimeTypes)
    {
        foreach(QString format, pMimeData->formats())
        {
            if (format.startsWith(mimeType))
            {
                return true;
            }
        }
    }

    foreach (QUrl url, pMimeData->urls())
    {
        foreach(QString ext, mFileExtensions)
        {
            if (url.toString().endsWith(ext))
            {
                return true;
            }
        }
    }

    foreach (QUrl url, pMimeData->urls())
    {
        foreach(QString protocole, mProtocoles)
        {
            if (url.toString().startsWith(protocole))
            {
                return true;
            }
        }
    }

    return false;
}


void UBLibraryTreeWidget::dragEnterEvent(QDragEnterEvent * event)
{
    mDropFolderItem = 0;

    if (supportMimeData(event->mimeData()))
    {
        event->acceptProposedAction();
        return;
    }
    else
    {
        QTreeWidget::dragEnterEvent(event);
    }
}


void UBLibraryTreeWidget::dragMoveEvent(QDragMoveEvent * event)
{
    QTreeWidgetItem * ti = itemAt(event->pos());
    UBLibraryFolderItem * lfi = dynamic_cast<UBLibraryFolderItem*>(ti);

    if (lfi && lfi->canWrite() && !(lfi == currentItem()))
    {
        event->acceptProposedAction();
        return;
    }

    QTreeWidget::dragMoveEvent(event);
}


void UBLibraryTreeWidget::dropEvent(QDropEvent * event)
{
    QTreeWidgetItem * ti = itemAt(event->pos());
    UBLibraryFolderItem * lfi = dynamic_cast<UBLibraryFolderItem*>(ti);

    if (lfi)
    {
        UBLibraryFolderItem * sourceLfi = dynamic_cast<UBLibraryFolderItem*>(currentItem());

        mDropFolderItem = lfi;

        QDir dir = lfi->dir();

        foreach(QUrl url, event->mimeData()->urls())
        {
            // is it local ?
            QString localFile = url.toLocalFile();

            if (localFile.length() > 0)
            {
                QFileInfo fileInfo(localFile);

                //is it a file ?
                if (fileInfo.isFile())
                {
                    // copy file to dir
                    QFile file(localFile);

                    if (sourceLfi && sourceLfi->canWrite())
                    {
                        file.rename(dir.filePath(fileInfo.fileName()));
                        emit contentChanged();
                    }
                    else
                    {
                        file.copy(dir.filePath(fileInfo.fileName()));
                    }
                }
                else if (fileInfo.isDir())
                {
                    if (sourceLfi && sourceLfi->canWrite())
                    {
                        UBFileSystemUtils::moveDir(localFile, dir.filePath(fileInfo.fileName()));
                        emit contentChanged();
                    }
                    else
                    {
                        UBFileSystemUtils::copyDir(localFile, dir.filePath(fileInfo.fileName()));
                    }
                }
            }
            else if (url.toString().startsWith("uniboardTool://"))
            {
                UBApplication::boardController->libraryController()->addNativeToolToFavorites(url);
            }
            else
            {
                UBHttpGet* httpGet = new UBHttpGet(this); // TODO UB 4.6 we are leaking here, tree widget is never destroyed

                connect(httpGet, SIGNAL(downloadFinished(bool, QUrl, QString, QByteArray, QPointF, QSize, bool)),
                        this, SLOT(downloadFinished(bool, QUrl, QString, QByteArray)));

                UBApplication::setOverrideCursor(Qt::WaitCursor);
                UBApplication::showMessage(tr("Downloading content from %1").arg(url.toString()), true);

                httpGet->get(url);
            }
        }

        event->acceptProposedAction();
        return;
    }
    else
    {
        QTreeWidget::dropEvent(event);
    }
}


void UBLibraryTreeWidget::downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData)
{
    if (!pSuccess)
    {
        UBApplication::showMessage(tr("Downloading content %1 failed").arg(sourceUrl.toString()));
    }
    else
    {
        UBApplication::showMessage(tr("Download finished"));
        QStringList urlPathParts = sourceUrl.path().split("/");

        if (urlPathParts.length() > 0 && mDropFolderItem)
        {
            QString fileName = urlPathParts.last();

            QString ext = UBFileSystemUtils::fileExtensionFromMimeType(pContentTypeHeader);

            if (!fileName.endsWith(ext))
            {
                fileName += "." + ext;
            }

            QString filePath = mDropFolderItem->dir().filePath(fileName);
            QString nextItemPath  = UBFileSystemUtils::nextAvailableFileName(filePath);

            if (UBSettings::widgetFileExtensions.contains(ext))
            {
                QTemporaryFile tempFile;

                if (tempFile.open())
                {
                    tempFile.write(pData);
                    tempFile.close();

                    QDir widgetDir(nextItemPath);

                    UBFileSystemUtils::expandZipToDir(tempFile, widgetDir);
                }
            }
            else if(UBW3CWidget::hasNPAPIWrapper(pContentTypeHeader))
            {
                QString widgetPath = UBW3CWidget::createNPAPIWrapperInDir(sourceUrl.toString()
                        , mDropFolderItem->dir(), pContentTypeHeader);
            }
            else
            {
                QFile itemFile(nextItemPath);
                if (itemFile.open(QIODevice::WriteOnly))
                {
                    itemFile.write(pData);
                    itemFile.close();
                }
            }
        }
    }

    UBApplication::restoreOverrideCursor();
}

