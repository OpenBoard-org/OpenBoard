#ifndef UBPROPERTYPALETTE_H
#define UBPROPERTYPALETTE_H

#include <QMouseEvent>
#include <QPoint>

#include "UBActionPalette.h"

class UBPropertyPalette : public UBActionPalette
{
    Q_OBJECT
public:
    UBPropertyPalette(QWidget* parent=0, const char* name="propertyPalette");
    UBPropertyPalette(Qt::Orientation orientation, QWidget* parent = 0);
    ~UBPropertyPalette();

private slots:
    void onMouseRelease();
};

#endif // UBPROPERTYPALETTE_H
