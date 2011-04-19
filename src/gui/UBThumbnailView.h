
#ifndef UBTHUMBNAILVIEW_H_
#define UBTHUMBNAILVIEW_H_

#include <QGraphicsView>

class UBGraphicsScene;

class UBThumbnailView : public QGraphicsView
{
    Q_OBJECT;

    public:
        UBThumbnailView();
        virtual ~UBThumbnailView();

    signals:
        void doubleClicked();

    protected:
        virtual void drawBackground(QPainter *painter, const QRectF &rect);
        virtual void mouseDoubleClickEvent ( QMouseEvent * event );

};

#endif /* UBTHUMBNAILVIEW_H_ */
