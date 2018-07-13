#include "UBGraphicsProxyWidget.h"

UBGraphicsProxyWidget::UBGraphicsProxyWidget(QGraphicsItem *parent, Qt::WindowFlags wFlags) :
    QGraphicsProxyWidget(parent, wFlags)
{
}

UBGraphicsProxyWidget::~UBGraphicsProxyWidget()
{
    int i = 0;
    if (i == 0)
        i++;
}
