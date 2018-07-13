#ifndef UBSTYLUSPALLETTE_H_
#define UBSTYLUSPALLETTE_H_

#include <QButtonGroup>

#include "UBActionPalette.h"


class UBDrawingPalette_ : public UBActionPalette
{
    Q_OBJECT

    public:
        UBDrawingPalette_(QWidget *parent = 0, Qt::Orientation orient = Qt::Horizontal);
        virtual ~UBDrawingPalette_();

        void initPosition();

    private slots:

        void drawingToolDoubleClicked();

    private:
        int mLastSelectedId;

    signals:
        void drawingToolDoubleClicked(int tool);
};

#endif /* UBSTYLUSPALLETTE_H_ */
