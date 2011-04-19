
#include <QFile>

#include "PDFRenderer.h"

#include "XPDFRenderer.h"


QMap< QUuid, QPointer<PDFRenderer> > PDFRenderer::sRenderers;


PDFRenderer::~PDFRenderer()
{
    // NOOP
}

PDFRenderer* PDFRenderer::rendererForUuid(const QUuid &uuid, const QString &filename)
{
    if (sRenderers.contains(uuid))
    {
        return sRenderers.value(uuid);
    }
    else
    {
        PDFRenderer *newRenderer = new XPDFRenderer(filename);

        newRenderer->setRefCount(0);
        newRenderer->setFileUuid(uuid);

        QFile file(filename);
        file.open(QIODevice::ReadOnly);
        newRenderer->setFileData(file.readAll());
        file.close();

        sRenderers.insert(newRenderer->fileUuid(), newRenderer);

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
    if (mRefCount == 0)
    {
        sRenderers.remove(mFileUuid);
        delete this;
    }
}
