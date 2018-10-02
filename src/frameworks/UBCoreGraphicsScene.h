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




#ifndef UBCOREGRAPHICSSCENE_H_
#define UBCOREGRAPHICSSCENE_H_

#include <QtGui>
#include <QGraphicsScene>
#include <QGraphicsItem>

class UBCoreGraphicsScene : public QGraphicsScene
{
    public:
        UBCoreGraphicsScene(QObject * parent = 0);
        virtual ~UBCoreGraphicsScene();

        virtual void addItem(QGraphicsItem* item);

        virtual void removeItem(QGraphicsItem* item, bool forceDelete = false);

        virtual bool deleteItem(QGraphicsItem* item);

        void removeItemFromDeletion(QGraphicsItem* item);
        void addItemToDeletion(QGraphicsItem *item);

        bool isModified() const
        {
            return mIsModified;
        }

        void setModified(bool pModified)
        {
            mIsModified = pModified;
        }


    private:
        QSet<QGraphicsItem*> mItemsToDelete;

        bool mIsModified;
};

#endif /* UBCOREGRAPHICSSCENE_H_ */
