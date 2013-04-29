/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#ifndef UBIMPORTIMAGE_H_
#define UBIMPORTIMAGE_H_

#include <QtGui>
#include "UBImportAdaptor.h"

class UBDocumentProxy;

class UBImportImage : public UBPageBasedImportAdaptor
{
    Q_OBJECT;

    public:
        UBImportImage(QObject *parent = 0);
        virtual ~UBImportImage();

        virtual QStringList supportedExtentions();
        virtual QString importFileFilter();

        virtual QList<UBGraphicsItem*> import(const QUuid& uuid, const QString& filePath);
        virtual void placeImportedItemToScene(UBGraphicsScene* scene, UBGraphicsItem* item);
        virtual const QString& folderToCopy();
};

#endif /* UBIMPORTIMAGE_H_ */
