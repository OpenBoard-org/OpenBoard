/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#include "UBImportImage.h"

#include "document/UBDocumentProxy.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBDocumentManager.h"

#include "domain/UBGraphicsPixmapItem.h"

#include "pdf/PDFRenderer.h"

#include "core/memcheck.h"

UBImportImage::UBImportImage(QObject *parent)
    : UBPageBasedImportAdaptor(parent)
{
    // NOOP
}


UBImportImage::~UBImportImage()
{
    // NOOP
}


QStringList UBImportImage::supportedExtentions()
{
    QStringList formats;

    for ( int i = 0; i < QImageReader::supportedImageFormats().count(); ++i )
    {
            formats << QString(QImageReader::supportedImageFormats().at(i)).toLower();
    }

    return formats;
}


QString UBImportImage::importFileFilter()
{
    QString filter = tr("Image Format (");
    QStringList formats = supportedExtentions();
    bool isFirst = true;

    foreach(QString format, formats)
    {
            if(isFirst)
                    isFirst = false;
            else
                    filter.append(" ");

        filter.append("*."+format);
    }

    filter.append(")");

    return filter;
}

/*
bool UBImportImage::addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile)
{
    int res = UBDocumentManager::documentManager()->addImageAsPageToDocument(QStringList(QFileInfo(pFile).absoluteFilePath()), pDocument);
    if (res == 0)
    {
        UBApplication::showMessage(tr("Image import failed."));
        return false;
    }
    else
    {
        UBApplication::showMessage(tr("Image import successful."));
        return true;
    }
}
*/


QList<UBGraphicsItem*> UBImportImage::import(const QUuid& uuid, const QString& filePath)
{
    QList<UBGraphicsItem*> result;

    QPixmap pix(filePath);
    if (pix.isNull())
        return result;

    UBGraphicsPixmapItem* pixmapItem = new UBGraphicsPixmapItem();
    pixmapItem->setPixmap(pix);
    result << pixmapItem;
    return result;
}

void UBImportImage::placeImportedItemToScene(UBGraphicsScene* scene, UBGraphicsItem* item)
{
    UBGraphicsPixmapItem* pixmapItem = (UBGraphicsPixmapItem*)item;
    
     UBGraphicsPixmapItem* sceneItem = scene->addPixmap(pixmapItem->pixmap(), NULL, QPointF(0, 0));
     scene->setAsBackgroundObject(sceneItem, true);

     // Only stored pixmap, should be deleted now
     delete pixmapItem;
}

const QString& UBImportImage::folderToCopy()
{
    static QString f("");
    return f;
}
