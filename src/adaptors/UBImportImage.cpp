/*
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




#include "UBImportImage.h"

#include "document/UBDocumentProxy.h"

#include "board/UBBoardController.h"

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

QList<UBGraphicsItem*> UBImportImage::import(const QUuid& uuid, const QString& filePath)
{
    Q_UNUSED(uuid);
    QList<UBGraphicsItem*> result;

    QPixmap pix(filePath);
    if (pix.isNull())
        return result;

    UBGraphicsPixmapItem* pixmapItem = new UBGraphicsPixmapItem();
    pixmapItem->setPixmap(pix);
    result << pixmapItem;

    QString documentPath = UBApplication::boardController->selectedDocument()->persistencePath();
    QString fileName = UBPersistenceManager::imageDirectory + "/" + pixmapItem->uuid().toString() + ".png";
    QString path = documentPath + "/" + fileName;
    if (!QFile::exists(path))
    {
        QDir dir;
        dir.mkdir(documentPath + "/" + UBPersistenceManager::imageDirectory);

        pixmapItem->pixmap().toImage().save(path, "PNG");
    }

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
