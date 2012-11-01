/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef PDFRENDERER_H
#define PDFRENDERER_H

#include <QObject>
#include <QSizeF>
#include <QRect>
#include <QByteArray>
#include <QUuid>
#include <QMap>
#include <QPointer>

class QPainter;

class PDFRenderer : public QObject
{
    Q_OBJECT

    public:
        static PDFRenderer* rendererForUuid(const QUuid &uuid, const QString &filename, bool importingFile = false);
        virtual ~PDFRenderer();

        virtual bool isValid() const = 0;

        virtual int pageCount() const = 0;

        virtual QSizeF pageSizeF(int pageNumber) const = 0;

        virtual int pageRotation(int pageNumber) const = 0;

        virtual QString title() const = 0;

        void attach();
        void detach();

        QUuid fileUuid() const { return mFileUuid; }
        QByteArray fileData() const { return mFileData; }

		void setDPI(int desiredDPI) { this->dpiForRendering = desiredDPI; }

    public slots:
        virtual void render(QPainter *p, int pageNumber, const QRectF &bounds = QRectF()) = 0;

    private:
        QAtomicInt mRefCount;
        QByteArray mFileData;
        QUuid mFileUuid;

        void setRefCount(const QAtomicInt &refCount);
        void setFileData(const QByteArray &fileData);
        void setFileUuid(const QUuid &fileUuid);

        static QMap< QUuid, QPointer<PDFRenderer> > sRenderers;

	protected:
		int dpiForRendering;
		PDFRenderer();
};

#endif // PDFRENDERER_H
