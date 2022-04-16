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

        virtual void render(QPainter *p, int pageNumber, bool const cacheAllowed, const QRectF &bounds = QRectF()) = 0;

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
