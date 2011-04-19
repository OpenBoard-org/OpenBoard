/*
 * UBLibraryTreeWidget.h
 *
 *  Created on: 28 juil. 2009
 *      Author: Luc
 */

#ifndef UBLIBRARYTREEWIDGET_H_
#define UBLIBRARYTREEWIDGET_H_

#include <QtGui>

class UBLibraryFolderItem;

class UBLibraryTreeWidget : public QTreeWidget
{
    Q_OBJECT;

    public:
        UBLibraryTreeWidget(QWidget * parent = 0);
        virtual ~UBLibraryTreeWidget();

        void setMimeTypes(const QStringList& pMimeTypes)
        {
            mMimeTypes = pMimeTypes;
        }

        void setFileExtensions(const QStringList& pFileExtensions)
        {
            mFileExtensions = pFileExtensions;
        }

        void setProtocoles(const QStringList& pProtocoles)
        {
            mProtocoles = pProtocoles;
        }

        bool supportMimeData(const QMimeData* pMimeData);

    signals:
        void contentChanged();

    protected:

        virtual void dragEnterEvent(QDragEnterEvent * event);
        virtual void dragMoveEvent(QDragMoveEvent * event);
        virtual void dropEvent(QDropEvent * event);

    private slots:
        void downloadFinished(bool pSuccess, QUrl sourceUrl, QString pContentTypeHeader, QByteArray pData);

    private:

       UBLibraryFolderItem* mDropFolderItem;

       QStringList mMimeTypes;
       QStringList mFileExtensions;
       QStringList mProtocoles;

};

#endif /* UBLIBRARYTREEWIDGET_H_ */
