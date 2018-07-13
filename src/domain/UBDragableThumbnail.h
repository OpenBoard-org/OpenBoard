#ifndef UBDRAGABLELABEL_H
#define UBDRAGABLELABEL_H

#include <QFrame>
#include <QLabel>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QHBoxLayout>
#include <QPixmap>

class UBDraggableThumbnail : public QFrame
{
    public:
        UBDraggableThumbnail(QWidget* parent =0, const QPixmap& pixmap = QPixmap(":images/libpalette/notFound.png"));

        void setThumbnail(const QPixmap &pixmap);
        void setPixmap(const QPixmap & pixmap);

    protected:
        void dragEnterEvent(QDragEnterEvent *event);
        void dragMoveEvent(QDragMoveEvent *event);
        void dropEvent(QDropEvent *event);
        void mousePressEvent(QMouseEvent *event);

    private:
        QLabel* mThumbnail;
        QHBoxLayout* mHBoxLayout;
};

#endif // UBDRAGABLELABEL_H
