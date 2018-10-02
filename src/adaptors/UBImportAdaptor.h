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




#ifndef UBIMPORTADAPTOR_H_
#define UBIMPORTADAPTOR_H_

#include <QtGui>

class UBGraphicsItem;
class UBGraphicsScene;
class UBDocumentProxy;

class UBImportAdaptor : public QObject
{
    Q_OBJECT;

    protected:
        UBImportAdaptor(bool _documentBased, QObject *parent = 0);
        virtual ~UBImportAdaptor();

    public:

        virtual QStringList supportedExtentions() = 0;
        virtual QString importFileFilter() = 0;

        bool isDocumentBased(){return documentBased;}
    private:
        bool documentBased;
        
};

class UBPageBasedImportAdaptor : public UBImportAdaptor
{
protected:
        UBPageBasedImportAdaptor(QObject *parent = 0);

public:
        virtual QList<UBGraphicsItem*> import(const QUuid& uuid, const QString& filePath) = 0;
        virtual void placeImportedItemToScene(UBGraphicsScene* scene, UBGraphicsItem* item) = 0;
        virtual const QString& folderToCopy() = 0;
};

class UBDocumentBasedImportAdaptor : public UBImportAdaptor
{
protected:
        UBDocumentBasedImportAdaptor(QObject *parent = 0);
public:
    virtual UBDocumentProxy* importFile(const QFile& pFile, const QString& pGroup) = 0;
    virtual bool addFileToDocument(UBDocumentProxy* pDocument, const QFile& pFile) = 0;
};


#endif /* UBIMPORTADAPTOR_H_ */
