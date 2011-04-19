/*
 * UBCoreGraphicsScene.h
 *
 *  Created on: 28 mai 2009
 *      Author: Luc
 */

#ifndef UBCOREGRAPHICSSCENE_H_
#define UBCOREGRAPHICSSCENE_H_

#include <QtGui>

class UBCoreGraphicsScene : public QGraphicsScene
{
    public:
        UBCoreGraphicsScene(QObject * parent = 0);
        virtual ~UBCoreGraphicsScene();

        virtual void addItem(QGraphicsItem* item);

        virtual void removeItem(QGraphicsItem* item, bool forceDelete = false);

    private:
        QSet<QGraphicsItem*> mItemsToDelete;
};

#endif /* UBCOREGRAPHICSSCENE_H_ */
