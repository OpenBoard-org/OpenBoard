#include <QMimeData>
#include <QDrag>
#include <QUrl>

#include "UBDraggableLabel.h"


UBDraggableLabel::UBDraggableLabel(QWidget *parent) :
    QLabel(parent)
{
}

UBDraggableLabel::~UBDraggableLabel()
{
    //NOOP
}

void UBDraggableLabel::loadImage(QString imagePath)
{
    mSourcePath = imagePath;
    QPixmap pix = QPixmap(mSourcePath);
    setPixmap(pix);
    setScaledContents(true);
}

void UBDraggableLabel::mousePressEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    QMimeData *mimeData = new QMimeData;
    QList<QUrl> urls;
    urls << QUrl::fromLocalFile(mSourcePath);
    mimeData->setUrls(urls);
    mimeData->setText(mSourcePath);


    QDrag *drag = new QDrag(this);
    drag->setMimeData(mimeData);
    drag->start();
}
