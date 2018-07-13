#ifndef UBGRAPHICSPROXYWIDGET_H
#define UBGRAPHICSPROXYWIDGET_H

#include <QGraphicsProxyWidget>

class UBGraphicsProxyWidget : public QGraphicsProxyWidget
{
    Q_OBJECT
public:
    explicit UBGraphicsProxyWidget(QGraphicsItem *parent = 0, Qt::WindowFlags wFlags = 0);
    ~UBGraphicsProxyWidget();

};

#endif // UBGRAPHICSPROXYWIDGET_H
