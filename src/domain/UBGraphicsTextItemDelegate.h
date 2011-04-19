/*
 * UBGraphicsTextItemDelegate.h
 *
 *  Created on: June 15, 2009
 *      Author: Patrick
 */

#ifndef UBGRAPHICSTEXTITEMDELEGATE_H_
#define UBGRAPHICSTEXTITEMDELEGATE_H_

#include <QtGui>

#include <QtSvg>

#include "core/UB.h"
#include "UBGraphicsItemDelegate.h"

class UBGraphicsTextItem;

class UBGraphicsTextItemDelegate : public UBGraphicsItemDelegate
{
    Q_OBJECT;

    public:
        UBGraphicsTextItemDelegate(UBGraphicsTextItem* pDelegated, QObject * parent = 0);
        virtual ~UBGraphicsTextItemDelegate();

    public slots:
        void contentsChanged();

    protected:
        virtual void buildButtons();

    private:

        UBGraphicsTextItem* delegated();

        DelegateButton* mFontButton;
        DelegateButton* mColorButton;
        DelegateButton* mDecreaseSizeButton;
        DelegateButton* mIncreaseSizeButton;

        int mLastFontPixelSize;

        static const int sMinPixelSize;

    private:
        void customize(QFontDialog &fontDialog);

    private slots:

        void pickFont();
        void pickColor();

        void decreaseSize();
        void increaseSize();

};

#endif /* UBGRAPHICSTEXTITEMDELEGATE_H_ */
