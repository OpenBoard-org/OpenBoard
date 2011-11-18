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
#include "UBDownloadManager.h"
#include "core/UBApplication.h"
#include "gui/UBMainWindow.h"
#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "core/memcheck.h"

/** The unique instance of the download manager */
static UBDownloadManager* pInstance = NULL;

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBDownloadManager::UBDownloadManager(QObject *parent, const char *name):QObject(parent)
{
    setObjectName(name);
    init();

    connect(this, SIGNAL(fileAddedToDownload()), this, SLOT(onUpdateDownloadLists()));
}

/**
 * \brief Destructor
 */
UBDownloadManager::~UBDownloadManager()
{

}

/**
 * \brief Get the download manager
 * @return a pointer on the download manager
 */
UBDownloadManager* UBDownloadManager::downloadManager()
{
    if(NULL == pInstance)
    {
        pInstance = new UBDownloadManager();
    }
    return pInstance;
}

void UBDownloadManager::destroy()
{
    if(pInstance)
    {
        delete pInstance;
    }
    pInstance = NULL;
}

/**
 * \brief Add a file to the download list
 * @param desc as the given file description
 */
void UBDownloadManager::addFileToDownload(sDownloadFileDesc desc)
{
    // Set the ID for this download
    desc.id = mLastID;
    mLastID++;

    // Add the file to the pending download list
    mPendingDL.append(desc);

    // If the download is modal, show the download dialog
    if(desc.modal)
    {
        // Update the download order (priority to modal files)
        updateDownloadOrder();
        UBApplication::mainWindow->showDownloadWidget();
    }
    UBApplication::boardController->paletteManager()->startDownloads();

    emit fileAddedToDownload();
}

/**
 * \brief Initialize the download manager
 */
void UBDownloadManager::init()
{
    mCrntDL.clear();
    mPendingDL.clear();
    mReplies.clear();
    mLastID = 1;
    mDLAvailability.clear();
    for(int i=0; i<SIMULTANEOUS_DOWNLOAD; i++)
    {
        mDLAvailability.append(-1);
    }
}

/**
 * \brief Update the download order. The modal downloads will be put in priority.
 */
void UBDownloadManager::updateDownloadOrder()
{
    QVector<sDownloadFileDesc> modalFiles;
    QVector<sDownloadFileDesc> nonModalfiles;

    for(int i=0; i<mPendingDL.size(); i++)
    {
        sDownloadFileDesc crnt = mPendingDL.at(i);
        if(crnt.modal)
        {
            modalFiles.append(crnt);
        }
        else
        {
            nonModalfiles.append(crnt);
        }
    }

    mPendingDL = modalFiles + nonModalfiles;
}

/**
 * \brief Update the download list. If a current download is finished, we take a
 *  file from the pending download list and add it to the download list.
 */
void UBDownloadManager::onUpdateDownloadLists()
{
    for(int i=0; i<SIMULTANEOUS_DOWNLOAD; i++)
    {
        if(mPendingDL.empty())
        {
            // If we fall here that means that there is no pending download
            break;
        }
        if(-1 == mDLAvailability.at(i))
        {
            // Pending downloads exist and a download 'slot' is available
            // Let's move the first pending download to the current download
            // list and fill the slot
            sDownloadFileDesc desc = mPendingDL.at(0);
            mCrntDL.append(desc);
            mPendingDL.remove(0);
            mDLAvailability.remove(i);
            mDLAvailability.insert(i, desc.id);

            // Start the download of this file
            startFileDownload(desc);
        }
    }
}

/**
 * \brief Get the list of the current downloads
 * @return a QVector of current downloads
 */
QVector<sDownloadFileDesc> UBDownloadManager::currentDownloads()
{
    return mCrntDL;
}

/**
 * \brief Get the list of the pending downloads
 * @return a QVector of pending downloads
 */
QVector<sDownloadFileDesc> UBDownloadManager::pendingDownloads()
{
    return mPendingDL;
}

/**
 * \brief Update the file transfer information
 * @param desc as the current downloaded file description
 */
void UBDownloadManager::onDownloadProgress(int id, qint64 received, qint64 total)
{
    updateFileCurrentSize(id, received, total);
}

/**
 * \brief Called when the download of the given file is finished
 * @param desc as the current downloaded file description
 */
void UBDownloadManager::onDownloadFinished(int id, bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground)
{
    for(int i=0; i<mCrntDL.size(); i++)
    {
        sDownloadFileDesc desc = mCrntDL.at(i);
        if(id == desc.id)
        {
            if(desc.modal)
            {
                // The downloaded file is modal so we must put it on the board
                emit addDownloadedFileToBoard(pSuccess, sourceUrl, pContentTypeHeader, pData, pPos, pSize, isBackground);
            }
            else
            {
                emit addDownloadedFileToLibrary(pSuccess, sourceUrl, pContentTypeHeader, pData);
            }
            break;
        }
    }

    // Then do this
    updateFileCurrentSize(id);
}

/**
 * \brief Update the description of the given current downloaded file
 * @param desc as the current downloaded file description
 */
void UBDownloadManager::updateFileCurrentSize(int id, qint64 received, qint64 total)
{
    for(int i=0; i<mCrntDL.size();i++)
    {
        if(mCrntDL.at(i).id == id)
        {
            sDownloadFileDesc desc = mCrntDL.at(i);
            if(received >= 0 && total >= 0)
            {
                // -------------------------------------
                // [=============== x% ====            ]
                // -------------------------------------
                desc.currentSize = received;
                desc.totalSize = total;

                emit downloadUpdated(id, received, total);
            }
            else
            {
                // -------------------------------------
                // [=============== 100% ==============]
                // -------------------------------------
                // received and total are negative. That means that the download is finished
                desc.currentSize = mCrntDL.at(i).totalSize;

                // Remove the finished file from the current download list
                mCrntDL.remove(i);

                // Here we don't forget to remove the reply related to the finished download
                mReplies.remove(id);

                // Free the download slot used by the finished file
                for(int j=0; j<mDLAvailability.size();j++)
                {
                    if(id == mDLAvailability.at(j))
                    {
                        mDLAvailability.remove(j);
                        mDLAvailability.insert(j, -1);
                        break;
                    }
                }

                // Here we check if some modal downloads remain
                checkIfModalRemains();

                // Then we update the list of downloads
                onUpdateDownloadLists();

                emit downloadFinished(id);

                // Verify if all downloads are finished
                if(mCrntDL.empty() && mPendingDL.empty())
                {
                    finishDownloads();
                }

                break;
            }

            mCrntDL.remove(i);
            mCrntDL.insert(i,desc);
            break;
        }
    }
}

/**
 * \brief Start the download of a file
 * @param desc as the given file description
 */
void UBDownloadManager::startFileDownload(sDownloadFileDesc desc)
{
    UBDownloadHttpFile* http = new UBDownloadHttpFile(desc.id, this);
    connect(http, SIGNAL(downloadProgress(int, qint64,qint64)), this, SLOT(onDownloadProgress(int,qint64,qint64)));
    connect(http, SIGNAL(downloadFinished(int, bool, QUrl, QString, QByteArray, QPointF, QSize, bool)), this, SLOT(onDownloadFinished(int, bool, QUrl, QString, QByteArray, QPointF, QSize, bool)));

    // We send here the request and store its reply in order to be able to cancel it if needed
    mReplies[desc.id] = http->get(QUrl(desc.url));
}

/**
 * \brief Verify if modal downloads remains and notify everyone if it is not the case.
 */
void UBDownloadManager::checkIfModalRemains()
{
    bool bModal = false;
    for(int i=0; i<mCrntDL.size();i++)
    {
        if(mCrntDL.at(i).modal)
        {
            bModal = true;
            break;
        }
    }

    if(!bModal)
    {
        for(int j=0; j<mPendingDL.size(); j++)
        {
            if(mPendingDL.at(j).modal)
            {
                bModal = true;
                break;
            }
        }
    }

    if(bModal || (mCrntDL.empty() && mPendingDL.empty()))
    {
        // Close the modal window
        UBApplication::mainWindow->hideDownloadWidget();

        // Notify that no modal downloads are pending
        emit downloadModalFinished();
    }
}

/**
 * \brief Cancel all downloads
 */
void UBDownloadManager::cancelDownloads()
{
    // Stop the current downloads
    QMap<int, QNetworkReply*>::iterator it = mReplies.begin();
    for(; it!=mReplies.end();it++)
    {
        dynamic_cast<QNetworkReply*>(it.value())->abort();
    }

    // Clear all the lists
    init();

    checkIfModalRemains();

    finishDownloads(true);
}

void UBDownloadManager::onDownloadError(int id)
{
    QNetworkReply* pReply = mReplies.value(id);
    if(NULL != pReply)
    {
        // Check which error occured:
        switch(pReply->error())
        {
            case QNetworkReply::OperationCanceledError:
                // For futur developments: do something in case of download aborting (message? remove the download?)
            break;

            default:
                // Check the documentation of QNetworkReply in Qt Assistant for the different error cases
            break;
        }
    }
}

void UBDownloadManager::finishDownloads(bool cancel)
{
    UBApplication::boardController->paletteManager()->stopDownloads();
    if(cancel){
        emit cancelAllDownloads();
    }
    else{
        emit allDownloadsFinished();
    }
}

void UBDownloadManager::cancelDownload(int id)
{
    // Stop the download
    mReplies[id]->abort();
    mReplies.remove(id);

    // Remove the canceled download from the download lists
    bool bFound = false;
    for(int i=0; i<mCrntDL.size(); i++){
        if(id == mCrntDL.at(i).id){
            mCrntDL.remove(i);
            bFound = true;
            break;
        }
    }
    if(!bFound){
        for(int j=0; j<mPendingDL.size(); j++){
            if(id == mPendingDL.at(j).id){
                mPendingDL.remove(j);
                bFound = true;
                break;
            }
        }
    }

    // Free the download slot used by the finished file
    for(int h=0; h<mDLAvailability.size();h++){
        if(id == mDLAvailability.at(h)){
            mDLAvailability.remove(h);
            mDLAvailability.insert(h, -1);
            break;
        }
    }

    // Here we check if some modal downloads remain
    checkIfModalRemains();

    // Then we update the list of downloads
    onUpdateDownloadLists();

    // Verify if all downloads are finished
    if(mCrntDL.empty() && mPendingDL.empty())
    {
        finishDownloads();
    }
}

// ------------------------------------------------------------------------------
/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBDownloadHttpFile::UBDownloadHttpFile(int fileId, QObject *parent):UBHttpGet(parent)
{
    mId = fileId;

    connect(this, SIGNAL(downloadFinished(bool,QUrl,QString,QByteArray,QPointF,QSize,bool)), this, SLOT(onDownloadFinished(bool,QUrl,QString,QByteArray,QPointF,QSize,bool)));
    connect(this, SIGNAL(downloadProgress(qint64,qint64)), this, SLOT(onDownloadProgress(qint64,qint64)));
}

/**
 * \brief Destructor
 */
UBDownloadHttpFile::~UBDownloadHttpFile()
{

}

/**
 * \brief Handles the download progress notification
 * @param bytesReceived as the number of received bytes
 * @param bytesTotal as the total number of bytes
 */
void UBDownloadHttpFile::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    emit downloadProgress(mId, bytesReceived, bytesTotal);
}

/**
 * \brief Handles the download finished notification
 * @param pSuccess as the success indicator
 * @param sourceUrl as the source URL
 * @param pContentTypeHeader as the response content type header
 * @param pData as the packet data
 * @param pPos as the item position in the board
 * @param psize as the item size (GUI)
 * @param isBackground as the background mdoe indicator
 */
void UBDownloadHttpFile::onDownloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData, QPointF pPos, QSize pSize, bool isBackground)
{
    if(pSuccess)
    {
        // Notify the end of the download
        emit downloadFinished(mId, pSuccess, sourceUrl, pContentTypeHeader, pData, pPos, pSize, isBackground);
    }
    else
    {
        // Notify the fact that and error occured during the download
        emit downloadError(mId);
    }
}

