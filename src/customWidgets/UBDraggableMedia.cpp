#include <QApplication>
#include <QUrl>

#include "UBDraggableMedia.h"

UBDraggableMedia::UBDraggableMedia(eMediaType type, QWidget *parent, const char *name):UBMediaWidget(type, parent, name)
{

}

UBDraggableMedia::~UBDraggableMedia()
{

}

void UBDraggableMedia::mousePressEvent(QMouseEvent* ev)
{
    if(Qt::LeftButton == ev->button()){
        mDragStartPos = ev->pos();
    }
}

void UBDraggableMedia::mouseMoveEvent(QMouseEvent* ev)
{
    if(!(ev->buttons() & Qt::LeftButton)){
        return;
    }
    if((ev->pos() - mDragStartPos).manhattanLength() < QApplication::startDragDistance()){
        return;
    }
    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData;

    QList<QUrl> urls;
    urls << QUrl(mFilePath);
    mimeData->setText(mFilePath);
    mimeData->setUrls(urls);

    drag->setMimeData(mimeData);

    drag->exec(Qt::CopyAction | Qt::MoveAction);
}
