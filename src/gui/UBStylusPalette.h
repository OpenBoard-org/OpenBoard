
#ifndef UBSTYLUSPALLETTE_H_
#define UBSTYLUSPALLETTE_H_

#include <QButtonGroup>

#include "UBActionPalette.h"


class UBStylusPalette : public UBActionPalette
{
    Q_OBJECT;

    public:
        UBStylusPalette(QWidget *parent = 0, Qt::Orientation orient = Qt::Vertical);
        virtual ~UBStylusPalette();

        void initPosition();

    private slots:

        void stylusToolDoubleClicked();

    private:
        int mLastSelectedId;

    signals:
        void stylusToolDoubleClicked(int tool);
};

#endif /* UBSTYLUSPALLETTE_H_ */
