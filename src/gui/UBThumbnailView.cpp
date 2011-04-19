
#include "UBThumbnailView.h"
#include "domain/UBGraphicsScene.h"

#include "core/UBMimeData.h"


UBThumbnailView::UBThumbnailView()
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
}

UBThumbnailView::~UBThumbnailView()
{
    // NOOP
}

void UBThumbnailView::drawBackground(QPainter *painter, const QRectF &rect)
{
    // Do not draw crossed background in thumbnails
    if (qobject_cast<UBGraphicsScene*>(scene())->isDarkBackground())
    {
        painter->fillRect(rect, QBrush(QColor(Qt::black)));
    }
    else
    {
        painter->fillRect(rect, QBrush(QColor(Qt::white)));
    }
}

void UBThumbnailView::mouseDoubleClickEvent ( QMouseEvent * event )
{
    Q_UNUSED(event);
    emit doubleClicked();
}

