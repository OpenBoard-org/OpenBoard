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




/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "WBDownloadManager.h"

#include <QtGui>
#include <QFileDialog>
#include <QWebSettings>

#include "network/UBAutoSaver.h"
#include "network/UBNetworkAccessManager.h"
#include "frameworks/UBFileSystemUtils.h"

#include <math.h>

#include "core/memcheck.h"

/*!
    DownloadItem is a widget that is displayed in the download manager list.
    It moves the data from the QNetworkReply into the QFile as well
    as update the information/progressbar and report errors.
 */
WBDownloadItem::WBDownloadItem(QNetworkReply *reply, bool requestFileName, QWidget *parent, QString customDownloadPath)
    : QWidget(parent)
    , m_reply(reply)
    , mRequestFileName(requestFileName)
    , mBytesReceived(0)
    , mCustomDownloadPath(customDownloadPath)
{

    setupUi(this);
    QPalette p = downloadInfoLabel->palette();
    p.setColor(QPalette::Text, Qt::darkGray);
    downloadInfoLabel->setPalette(p);
    progressBar->setMaximum(0);
    tryAgainButton->hide();
    connect(stopButton, SIGNAL(clicked()), this, SLOT(stop()));
    connect(openButton, SIGNAL(clicked()), this, SLOT(open()));
    connect(tryAgainButton, SIGNAL(clicked()), this, SLOT(tryAgain()));

    init();
}


void WBDownloadItem::init()
{
    if (!m_reply)
        return;

    // attach to the m_reply
    m_url = m_reply->url();

    qDebug() << "DownloadItem::init() with url:" << m_url;

    m_reply->setParent(this);
    connect(m_reply, SIGNAL(readyRead()), this, SLOT(downloadReadyRead()));
    connect(m_reply, SIGNAL(error(QNetworkReply::NetworkError)),
            this, SLOT(error(QNetworkReply::NetworkError)));
    connect(m_reply, SIGNAL(downloadProgress(qint64, qint64)),
            this, SLOT(downloadProgress(qint64, qint64)));
    connect(m_reply, SIGNAL(metaDataChanged()),
            this, SLOT(metaDataChanged()));
    connect(m_reply, SIGNAL(finished()),
            this, SLOT(finished()));

    // reset info
    downloadInfoLabel->clear();
    progressBar->setValue(0);
    getFileName();

    // start timer for the download estimation
    mDownloadTime.start();

    if (m_reply->error() != QNetworkReply::NoError) {
        error(m_reply->error());
        finished();
    }
}

void WBDownloadItem::getFileName()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));
    QString defaultLocation = !mCustomDownloadPath.isEmpty() ? mCustomDownloadPath :  QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    QString downloadDirectory = settings.value(QLatin1String("downloadDirectory"), defaultLocation).toString();
    if (!downloadDirectory.isEmpty())
        downloadDirectory += QLatin1Char('/');

    QString defaultFileName = saveFileName(downloadDirectory);

    QString fileName = defaultFileName;
    if (mRequestFileName)
    {
        fileName = QFileDialog::getSaveFileName(this, tr("Save File"), defaultFileName);
        if (fileName.isEmpty())
        {
            m_reply->close();
            fileNameLabel->setText(tr("Download canceled: %1").arg(QFileInfo(defaultFileName).fileName()));
            return;
        }
    }


    m_output.setFileName(fileName);
    fileNameLabel->setText(QFileInfo(m_output.fileName()).fileName());
    if (mRequestFileName)
        downloadReadyRead();

}


QString WBDownloadItem::saveFileName(const QString &directory) const
{
    // Move this function into QNetworkReply to also get file name sent from the server
    QString path = m_url.path();
    QFileInfo info(path);
    QString baseName = info.completeBaseName();
    QString endName = info.suffix();

    if (baseName.isEmpty())
    {
        baseName = QLatin1String("unnamed_download");
        qDebug() << "DownloadManager:: downloading unknown file:" << m_url;
    }
    QString name = directory + baseName + QLatin1Char('.') + endName;
    if (QFile::exists(name))
    {
        // already exists, don't overwrite
        int i = 1;
        do {
            name = directory + baseName + QLatin1Char('-') + QString::number(i++) + QLatin1Char('.') + endName;
        } while (QFile::exists(name));
    }
    return name;
}


void WBDownloadItem::stop()
{
    setUpdatesEnabled(false);
    stopButton->setEnabled(false);
    stopButton->hide();
    tryAgainButton->setEnabled(true);
    tryAgainButton->show();
    setUpdatesEnabled(true);
    m_reply->abort();
}


void WBDownloadItem::open()
{

    QUrl url = QUrl::fromLocalFile(m_output.fileName());
    QDesktopServices::openUrl(url);
}


void WBDownloadItem::tryAgain()
{
    if (!tryAgainButton->isEnabled())
        return;

    tryAgainButton->setEnabled(false);
    tryAgainButton->setVisible(false);
    stopButton->setEnabled(true);
    stopButton->setVisible(true);
    progressBar->setVisible(true);

    QNetworkReply *r = UBNetworkAccessManager::defaultAccessManager()->get(QNetworkRequest(m_url));
    if (m_reply)
        m_reply->deleteLater();
    if (m_output.exists())
        m_output.remove();
    m_reply = r;
    init();
    emit statusChanged();
}


void WBDownloadItem::downloadReadyRead()
{
    if (mRequestFileName && m_output.fileName().isEmpty())
        return;

    if (!m_output.isOpen())
    {
        // in case someone else has already put a file there
        if (!mRequestFileName)
        {
            getFileName();
        }

        if (!m_output.open(QIODevice::WriteOnly))
        {
            downloadInfoLabel->setText(tr("Error opening saved file: %1")
                    .arg(m_output.errorString()));
            stopButton->click();
            emit statusChanged();
            return;
        }

        emit statusChanged();
    }

    if (-1 == m_output.write(m_reply->readAll())) {
        downloadInfoLabel->setText(tr("Error saving: %1")
                .arg(m_output.errorString()));
        stopButton->click();
    }
}


void WBDownloadItem::error(QNetworkReply::NetworkError)
{
    qDebug() << "DownloadItem::error" << m_reply->errorString() << m_url;
    downloadInfoLabel->setText(tr("Network Error: %1").arg(m_reply->errorString()));
    tryAgainButton->setEnabled(true);
    tryAgainButton->setVisible(true);
}


void WBDownloadItem::metaDataChanged()
{
    qDebug() << "DownloadItem::metaDataChanged: not handled.";
}


void WBDownloadItem::downloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    mBytesReceived = bytesReceived;
    if (bytesTotal == -1)
    {
        progressBar->setValue(0);
        progressBar->setMaximum(0);
    } else {
        progressBar->setValue(bytesReceived);
        progressBar->setMaximum(bytesTotal);
    }
    updateInfoLabel();
}


void WBDownloadItem::updateInfoLabel()
{
    if (m_reply->error() == QNetworkReply::NoError)
        return;

    qint64 bytesTotal = progressBar->maximum();
    bool running = !downloadedSuccessfully();

    // update info label
    double speed = mBytesReceived * 1000.0 / mDownloadTime.elapsed();
    double timeRemaining = ((double)(bytesTotal - mBytesReceived)) / speed;
    QString timeRemainingString = tr("seconds");
    if (timeRemaining > 60)
    {
        timeRemaining = timeRemaining / 60;
        timeRemainingString = tr("minutes");
    }
    timeRemaining = floor(timeRemaining);

    // When downloading the eta should never be 0
    if (timeRemaining == 0)
        timeRemaining = 1;

    QString info;
    if (running)
    {
        QString remaining;
        if (bytesTotal != 0)
            remaining = tr("- %4 %5 remaining")
            .arg(timeRemaining)
            .arg(timeRemainingString);
        info = QString(tr("%1 of %2 (%3/sec) %4"))
            .arg(dataString(mBytesReceived))
            .arg(bytesTotal == 0 ? tr("?", "unknown file size") : dataString(bytesTotal))
            .arg(dataString((int)speed))
            .arg(remaining);
    }
    else
    {
        if (mBytesReceived == bytesTotal)
            info = dataString(m_output.size());
        else
            info = tr("%1 of %2 - Stopped")
                .arg(dataString(mBytesReceived))
                .arg(dataString(bytesTotal));
    }
    downloadInfoLabel->setText(info);
}


QString WBDownloadItem::dataString(int size) const
{
    QString unit;
    if (size < 1024)
    {
        unit = tr("bytes");
    } else if (size < 1024*1024)
    {
        size /= 1024;
        unit = tr("KB");
    } else {
        size /= 1024*1024;
        unit = tr("MB");
    }
    return QString(QLatin1String("%1 %2")).arg(size).arg(unit);
}


bool WBDownloadItem::downloading() const
{
    return (progressBar->isVisible());
}


bool WBDownloadItem::downloadedSuccessfully() const
{
    return (stopButton->isHidden() && tryAgainButton->isHidden());
}


void WBDownloadItem::finished()
{
    progressBar->hide();
    stopButton->setEnabled(false);
    stopButton->hide();
    m_output.close();
    updateInfoLabel();
    if(m_output.fileName().toLower().endsWith(".wgt") || m_output.fileName().toLower().endsWith(".wdgt")){
        QString destPath = QFileInfo(m_output.fileName()).absolutePath() + "/Dir" + QFileInfo(m_output.fileName()).fileName();
        if (!QFileInfo(m_output.fileName()).isDir()){
            UBFileSystemUtils::expandZipToDir(m_output.fileName(), destPath);
            m_output.remove();
        }
    }
    emit statusChanged();
}


/*!
    DownloadManager is a Dialog that contains a list of DownloadItems

    It is a basic download manager.  It only downloads the file, doesn't do BitTorrent,
    extract zipped files or anything fancy.
  */
WBDownloadManager::WBDownloadManager(QWidget *parent)
    : QDialog(parent)
    , mAutoSaver(new UBAutoSaver(this))
    , mManager(UBNetworkAccessManager::defaultAccessManager())
    , mIconProvider(0)
    , m_RemovePolicy(Never)
{
    setupUi(this);
    hide();

    downloadsView->setShowGrid(false);
    downloadsView->verticalHeader()->hide();
    downloadsView->horizontalHeader()->hide();
    downloadsView->setAlternatingRowColors(true);
    downloadsView->horizontalHeader()->setStretchLastSection(true);
    mModel = new WBDownloadModel(this);
    downloadsView->setModel(mModel);
    connect(cleanupButton, SIGNAL(clicked()), this, SLOT(cleanup()));
    load();
}


WBDownloadManager::~WBDownloadManager()
{
    mAutoSaver->changeOccurred();
    mAutoSaver->saveIfNeccessary();
    if (mIconProvider)
        delete mIconProvider;
}


int WBDownloadManager::activeDownloads() const
{
    int count = 0;
    for (int i = 0; i < mDownloads.count(); ++i)
    {
        if (mDownloads.at(i)->stopButton->isEnabled())
            ++count;
    }
    return count;
}


void WBDownloadManager::download(const QNetworkRequest &request, bool requestFileName)
{
    if (request.url().isEmpty())
        return;
    processDownloadedContent(mManager->get(request), requestFileName);
}


void WBDownloadManager::handleUnsupportedContent(QNetworkReply *reply, bool requestFileName, QString customDownloadPath)
{
    QUrl originalUrl = reply->request().url();

    if (originalUrl.isEmpty())
        return;

    processDownloadedContent(mManager->get(QNetworkRequest(originalUrl)), requestFileName, customDownloadPath);

}


void WBDownloadManager::processDownloadedContent(QNetworkReply *reply, bool requestFileName, QString customDownloadPath)
{
    if (!reply || reply->url().isEmpty())
        return;
    QVariant header = reply->header(QNetworkRequest::ContentLengthHeader);

    bool ok;
    int size = header.toInt(&ok);
    if (ok && size == 0)
        return;

    //redirect ?

    qDebug() << "redirect" << reply->header(QNetworkRequest::LocationHeader);
    qDebug() << "ContentTypeHeader" << reply->header(QNetworkRequest::ContentTypeHeader);
    qDebug() << "DownloadManager::handleUnsupportedContent" << reply->url() << "requestFileName" << requestFileName;

    WBDownloadItem *item = new WBDownloadItem(reply, requestFileName, this, customDownloadPath);
    addItem(item);
}


void WBDownloadManager::addItem(WBDownloadItem *item)
{
    connect(item, SIGNAL(statusChanged()), this, SLOT(updateRow()));
    int row = mDownloads.count();
    mModel->beginInsertRows(QModelIndex(), row, row);
    mDownloads.append(item);
    mModel->endInsertRows();
    updateItemCount();

    if (!mIsLoading)
        show();

    downloadsView->setIndexWidget(mModel->index(row, 0), item);

    if (!mIconProvider)
        mIconProvider = new QFileIconProvider();
    QIcon icon = mIconProvider->icon(item->m_output.fileName());
    if (icon.isNull())
        icon = style()->standardIcon(QStyle::SP_FileIcon);
    item->fileIcon->setPixmap(icon.pixmap(48, 48));
    downloadsView->setRowHeight(row, item->sizeHint().height());
}


void WBDownloadManager::updateRow()
{
    WBDownloadItem *item = qobject_cast<WBDownloadItem*>(sender());
    int row = mDownloads.indexOf(item);
    if (-1 == row)
        return;

    if (!mIconProvider)
        mIconProvider = new QFileIconProvider();

    QIcon icon = mIconProvider->icon(item->m_output.fileName());
    if (icon.isNull())
        icon = style()->standardIcon(QStyle::SP_FileIcon);

    item->fileIcon->setPixmap(icon.pixmap(48, 48));
    downloadsView->setRowHeight(row, item->minimumSizeHint().height());

    bool remove = false;
    QWebSettings *globalSettings = QWebSettings::globalSettings();

    if (!item->downloading() && globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        remove = true;

    if (item->downloadedSuccessfully() && removePolicy() == WBDownloadManager::SuccessFullDownload)
        remove = true;

    if (remove)
        mModel->removeRow(row);

    cleanupButton->setEnabled(mDownloads.count() - activeDownloads() > 0);
}


WBDownloadManager::RemovePolicy WBDownloadManager::removePolicy() const
{
    return m_RemovePolicy;
}


void WBDownloadManager::setRemovePolicy(RemovePolicy policy)
{
    if (policy == m_RemovePolicy)
        return;

    m_RemovePolicy = policy;
    mAutoSaver->changeOccurred();
}


void WBDownloadManager::save() const
{
    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));
    QMetaEnum removePolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("RemovePolicy"));
    settings.setValue(QLatin1String("removeDownloadsPolicy"), QLatin1String(removePolicyEnum.valueToKey(m_RemovePolicy)));

    if (m_RemovePolicy == Exit)
        return;

    for (int i = 0; i < mDownloads.count(); ++i)
    {
        QString key = QString(QLatin1String("download_%1_")).arg(i);
        settings.setValue(key + QLatin1String("url"), mDownloads[i]->m_url);
        settings.setValue(key + QLatin1String("location"), QFileInfo(mDownloads[i]->m_output).filePath());
        settings.setValue(key + QLatin1String("done"), mDownloads[i]->downloadedSuccessfully());
    }

    int i = mDownloads.count();
    QString key = QString(QLatin1String("download_%1_")).arg(i);

    while (settings.contains(key + QLatin1String("url")))
    {
        settings.remove(key + QLatin1String("url"));
        settings.remove(key + QLatin1String("location"));
        settings.remove(key + QLatin1String("done"));
        key = QString(QLatin1String("download_%1_")).arg(++i);
    }
}


void WBDownloadManager::load()
{
    mIsLoading = true;

    QSettings settings;
    settings.beginGroup(QLatin1String("downloadmanager"));

    QByteArray value = settings.value(QLatin1String("removeDownloadsPolicy"), QLatin1String("Never")).toByteArray();
    QMetaEnum removePolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("RemovePolicy"));
    m_RemovePolicy = removePolicyEnum.keyToValue(value) == -1 ?
                        Never :
                        static_cast<RemovePolicy>(removePolicyEnum.keyToValue(value));

    int i = 0;
    QString key = QString(QLatin1String("download_%1_")).arg(i);

    while (settings.contains(key + QLatin1String("url")))
    {
        QUrl url = settings.value(key + QLatin1String("url")).toUrl();
        QString fileName = settings.value(key + QLatin1String("location")).toString();
        bool done = settings.value(key + QLatin1String("done"), true).toBool();
        if (!url.isEmpty() && !fileName.isEmpty())
        {
            WBDownloadItem *item = new WBDownloadItem(0, this);
            item->m_output.setFileName(fileName);
            item->fileNameLabel->setText(QFileInfo(item->m_output.fileName()).fileName());
            item->m_url = url;
            item->stopButton->setVisible(false);
            item->stopButton->setEnabled(false);
            item->tryAgainButton->setVisible(!done);
            item->tryAgainButton->setEnabled(!done);
            item->progressBar->setVisible(!done);
            addItem(item);
        }
        key = QString(QLatin1String("download_%1_")).arg(++i);
    }

    cleanupButton->setEnabled(mDownloads.count() - activeDownloads() > 0);

    mIsLoading = false;
}


void WBDownloadManager::cleanup()
{
    if (mDownloads.isEmpty())
        return;

    mModel->removeRows(0, mDownloads.count());
    updateItemCount();

    if (mDownloads.isEmpty() && mIconProvider)
    {
        delete mIconProvider;
        mIconProvider = 0;
    }

    mAutoSaver->changeOccurred();
}


void WBDownloadManager::updateItemCount()
{
    int count = mDownloads.count();
    itemCount->setText(count == 1 ? tr("1 Download") : tr("%1 Downloads", "always >= 2").arg(count));
}


WBDownloadModel::WBDownloadModel(WBDownloadManager *downloadManager, QObject *parent)
    : QAbstractListModel(parent)
    , mDownloadManager(downloadManager)
{
    // NOOP
}

QVariant WBDownloadModel::data(const QModelIndex &index, int role) const
{
    if (index.row() < 0 || index.row() >= rowCount(index.parent()))
        return QVariant();

    if (role == Qt::ToolTipRole)
        if (!mDownloadManager->mDownloads.at(index.row())->downloadedSuccessfully())
            return mDownloadManager->mDownloads.at(index.row())->downloadInfoLabel->text();

    return QVariant();
}


int WBDownloadModel::rowCount(const QModelIndex &parent) const
{
    return (parent.isValid()) ? 0 : mDownloadManager->mDownloads.count();
}


bool WBDownloadModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent.isValid())
        return false;

    int lastRow = row + count - 1;

    for (int i = lastRow; i >= row; --i)
    {
        if (mDownloadManager->mDownloads.at(i)->downloadedSuccessfully()
            || mDownloadManager->mDownloads.at(i)->tryAgainButton->isEnabled())
        {
            beginRemoveRows(parent, i, i);
            mDownloadManager->mDownloads.takeAt(i)->deleteLater();
            endRemoveRows();
        }
    }

    mDownloadManager->mAutoSaver->changeOccurred();
    return true;
}

