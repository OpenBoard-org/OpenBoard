#ifndef UBDESKTOPPENPALETTE_H
#define UBDESKTOPPENPALETTE_H

#include <QtGui>
#include <QResizeEvent>

#include "gui/UBPropertyPalette.h"

class UBDesktopPenPalette : public UBPropertyPalette
{
    Q_OBJECT
    public:
        UBDesktopPenPalette(QWidget *parent = 0);
        virtual ~UBDesktopPenPalette(){}

    public slots:
        void onParentMinimized();
        void onParentMaximized();

    protected:
        void resizeEvent(QResizeEvent *);

    private slots:
        void onButtonReleased();
};

#endif // UBDESKTOPPENPALETTE_H
