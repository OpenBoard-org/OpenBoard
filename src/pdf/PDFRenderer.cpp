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




#include <QFile>
#include <QDesktopWidget>

#include "PDFRenderer.h"

#include "XPDFRenderer.h"

#include "core/UBApplication.h"
#include "core/memcheck.h"


QMap< QUuid, QPointer<PDFRenderer> > PDFRenderer::sRenderers;

PDFRenderer::PDFRenderer() : dpiForRendering(96)
{
}

PDFRenderer::~PDFRenderer()
{
    // NOOP
}

PDFRenderer* PDFRenderer::rendererForUuid(const QUuid &uuid, const QString &filename, bool importingFile)
{
    if (sRenderers.contains(uuid))
    {
        return sRenderers.value(uuid);
    }
    else
    {
        PDFRenderer *newRenderer = new XPDFRenderer(filename,importingFile);

        newRenderer->setRefCount(0);
        newRenderer->setFileUuid(uuid);

        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        newRenderer->setFileData(file.readAll());
        file.close();

        sRenderers.insert(newRenderer->fileUuid(), newRenderer);

        QDesktopWidget* desktop = UBApplication::desktop();
        int dpiCommon = (desktop->physicalDpiX() + desktop->physicalDpiY()) / 2;
        newRenderer->setDPI(dpiCommon);

        return newRenderer;
    }
}

void PDFRenderer::setRefCount(const QAtomicInt &refCount)
{
    mRefCount = refCount;
}

void PDFRenderer::setFileData(const QByteArray &fileData)
{
    mFileData = fileData;
}

void PDFRenderer::setFileUuid(const QUuid &fileUuid)
{
    mFileUuid = fileUuid;
}

void PDFRenderer::attach()
{
    mRefCount.ref();
}

void PDFRenderer::detach()
{
    mRefCount.deref();
    if (mRefCount.loadAcquire() == 0)
    {
        sRenderers.remove(mFileUuid);
        delete this;
    }
}
