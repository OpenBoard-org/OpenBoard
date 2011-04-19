/*
 * UBMimeData.h
 *
 *  Created on: Dec 8, 2008
 *      Author: Luc
 */

#ifndef UBMIMEDATA_H_
#define UBMIMEDATA_H_

#include <QtGui>

class UBDocumentProxy;
class UBItem;

struct UBMimeDataItem
{
    public:
        UBMimeDataItem(UBDocumentProxy* proxy, int sceneIndex);
        virtual ~UBMimeDataItem();

        UBDocumentProxy* documentProxy() const { return mProxy; }
        int sceneIndex() const { return mSceneIndex; }

    private:
        UBDocumentProxy* mProxy;
        int mSceneIndex;
};


class UBMimeDataGraphicsItem : public QMimeData
{
    Q_OBJECT;

    public:
            UBMimeDataGraphicsItem(QList<UBItem*> pItems);
        virtual ~UBMimeDataGraphicsItem();

        QList<UBItem*> items() const { return mItems; }

    private:
        QList<UBItem*> mItems;

};




class UBMimeData : public QMimeData
{
    Q_OBJECT;

    public:
        UBMimeData(const QList<UBMimeDataItem> &items);
        virtual ~UBMimeData();

        QList<UBMimeDataItem> items() const { return mItems; }

    private:
        QList<UBMimeDataItem> mItems;
};

#endif /* UBMIMEDATA_H_ */
