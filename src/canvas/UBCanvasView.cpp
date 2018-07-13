#include "UBCanvasView.h"
#include "gui/UBResources.h"
#include "core/UBApplication.h"

#include <QtWidgets>

UBCanvasView::UBCanvasView(int numberOfIndepedentBoards, int toolBarHeight, QColor background, QList<QPen*> _pens, QWidget *parent)
    : QWidget(parent)
{
    bgColor = background;

    discardNextTouch = false;
    counterNextTouch = 0;

    setAttribute(Qt::WA_AcceptTouchEvents);
    setAttribute(Qt::WA_StaticContents);
    modified = false;
    scribbling = false;

    qWarning()<<"SCREEN PHYSICAL SIZE:" << UBApplication::desktop()->widthMM() << "  " << UBApplication::desktop()->heightMM();
    qWarning()<<"PIXEL: "<< UBApplication::desktop()->logicalDpiX() << "  " << UBApplication::desktop()->logicalDpiY();

    QPixmap px (36,36);
    px.fill(Qt::transparent);
    QPainter qp(&px);
    qp.setRenderHint( QPainter::Antialiasing );
    qp.setPen(QPen(Qt::black, 1, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
    qp.drawEllipse(QRect(0,0,36,36));
    cursorEraser = new QCursor(px,18,18);

    pens = _pens;
    /*foreach(QPen* p, _pens)
        qWarning()<<p->brush()<<" "<<p->width()<<" "<<p->style()<<" "<<p->capStyle()<<" "<<p->joinStyle();*/

    TBHeight = toolBarHeight;
    setupBoards(numberOfIndepedentBoards);
}

void UBCanvasView::setupBoards(int numberOfIndependentBoards){

    QRect g = QApplication::desktop()->screenGeometry();
    this->setGeometry(g.x(),g.y()+TBHeight,g.width(),g.height()-TBHeight);

    independentBoards.clear();
    eraserMode.clear();
    eraserGesture.clear();

    int i = 0;
    int incWidth = this->geometry().width() / numberOfIndependentBoards;
    int h = this->geometry().height();
    int x = 0;

    for (i=0; i<numberOfIndependentBoards; i++){
        QRect *r = new QRect();
        r->setRect(x, 0, incWidth, h);
        x = (i+1)*incWidth;
        independentBoards.append(r);
        eraserMode.append(false);
        eraserGesture.append(false);
        //qWarning()<<r->x()<<"  "<<r->y()<<"  "<<r->width()<<"  "<<r->height();
    }
    update();
}

QRect UBCanvasView::getRegion(int pos){
    int x, y, w, h;
    independentBoards.at(pos)->getRect(&x,&y,&w,&h);
    //qWarning()<<"*** "<<x<<" "<<y<<" "<<w<<" "<<h;
    return QRect(x,y,w,h);
}

int UBCanvasView::getNumberRegion(QPoint p)
{
    int i=0;
    foreach (QRect* r, independentBoards){
        if(r->contains(p,false) == true){
            return i;
        }
        i++;
    }
    return 0;
}

void UBCanvasView::setEraserMode(int region, bool isEraser)
{
    eraserMode.replace(region, isEraser);
}

bool UBCanvasView::openImage(const QString &fileName)
{
    QImage loadedImage;
    if (!loadedImage.load(fileName))
        return false;

    QSize newSize = loadedImage.size().expandedTo(size());
    resizeImage(&loadedImage, newSize);
    image = loadedImage;
    modified = false;
    update();
    return true;
}

QImage UBCanvasView::createSubImage(QRect *rect) {
    size_t offset = rect->x() * image.depth() / 8
                    + rect->y() * image.bytesPerLine();
    return QImage(image.bits() + offset, rect->width(), rect->height(),
                  image.bytesPerLine(), image.format());
}

bool UBCanvasView::saveRegionImage(int region)
{
    QString fileName;
    QString fileFormat;
    fileFormat=".png";
    fileName = QFileDialog::getSaveFileName(this,"Save image",fileFormat);

    QImage visibleImage = this->createSubImage(independentBoards.at(region));
    resizeImage(&visibleImage, independentBoards.at(region)->size());

    if (visibleImage.save(fileName,"PNG")) {
        modified = false;
        return true;
    } else {
        return false;
    }
}

bool UBCanvasView::saveImage()
{
    QString fileName;
    QString fileFormat;
    fileFormat=".png";
    fileName = QFileDialog::getSaveFileName(this,"Save image",fileFormat);

    QImage visibleImage = image;
    resizeImage(&visibleImage, size());

    if (visibleImage.save(fileName,"PNG")) {
        modified = false;
        return true;
    } else {
        return false;
    }
}

void UBCanvasView::clearImage()
{
    image.fill(bgColor);//(qRgb(255, 255, 255));
    modified = true;
    update();
}

void UBCanvasView::clearRegion(int region)
{
    QPainter painter(&image);
    painter.setPen(Qt::NoPen);
    painter.setBrush(bgColor);
    const QRect rect = *independentBoards.at(region);
    painter.drawRect(rect);
    modified = true;
    update();
}


void UBCanvasView::paintEvent(QPaintEvent *event)
{

    QPainter painter(this);
    const QRect rect = event->rect();
    painter.drawImage(rect.topLeft(), image, rect);

    int i=0;
    QRect* r;
    foreach (r, independentBoards){
        i++;
        if(i < independentBoards.count()){
          QPainter painter(this);
          painter.setRenderHint(QPainter::Antialiasing);
          painter.setPen(QPen(Qt::gray, 2,Qt::DashLine, Qt::RoundCap, Qt::RoundJoin));
          painter.drawLine(r->topRight(),r->bottomRight());
        }
        //painter.drawRect(*r);
    }
}

void UBCanvasView::resizeEvent(QResizeEvent *event)
{
    if (width() > image.width() || height() > image.height()) {
        int newWidth = qMax(width() + 128, image.width());
        int newHeight = qMax(height() + 128, image.height());
        resizeImage(&image, QSize(newWidth, newHeight));
        update();
    }
    QWidget::resizeEvent(event);
}

void UBCanvasView::resizeImage(QImage *image, const QSize &newSize)
{
    if (image->size() == newSize)
        return;

    QImage newImage(newSize, QImage::Format_RGB32);
    newImage.fill(bgColor);//qRgb(255, 255, 255));
    QPainter painter(&newImage);
    painter.drawImage(QPoint(0, 0), *image);
    *image = newImage;
}

void UBCanvasView::print()
{
}

void UBCanvasView::drawLineTo(const QPoint &endPoint){
    qWarning()<<"drawLineTo";
    QRect rect(lastPoint,endPoint);
    //int penWidth = 5;
    QPainter painter(&image);
    painter.setRenderHints (QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    painter.setPen(QPen(bgColor, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    int i=0;
    foreach (QRect* r, independentBoards){
        if(r->intersects(rect) == true){
            if(eraserMode.at(i) == false)
                painter.setPen(*pens.at(i));//myPenColors.at(i), penWidth,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            else
                painter.setPen(QPen(bgColor, 36, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));//myPenColors.at(i), penWidth,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            break;
        }
        i++;
    }
    painter.drawLine(lastPoint, endPoint);
    modified = true;
    int rad = 50;
    update(rect.adjusted(-rad,-rad, +rad, +rad));
    //update();
    lastPoint=endPoint;
}

void UBCanvasView::drawLineToTouch(int id, const QPoint &endPoint){

    // Avoid senseless lines.   
    /*if( ( (lastPointHash[id].x() == endPoint.x()) &&
          (lastPointHash[id].y() == endPoint.y()) )
        ||*/
       if ( euclideanDistance(lastPointHash[id].x(), lastPointHash[id].y(), endPoint.x(), endPoint.y()) > 100 )
      //)
      return;

    QRect rect(lastPointHash[id],endPoint);
    //int penWidth = 5;
    QPainter painter(&image);
    painter.setRenderHints (QPainter::Antialiasing | QPainter::SmoothPixmapTransform | QPainter::TextAntialiasing);
    painter.setPen(QPen(bgColor, 5, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    int i=0;
    foreach (QRect* r, independentBoards){
        if(r->intersects(rect) == true){
            if( (eraserMode.at(i) == false) && (eraserGesture.at(i) == false) )
                painter.setPen(*pens.at(i));//myPenColors.at(i), penWidth,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            else
                painter.setPen(QPen(bgColor, 36, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));//myPenColors.at(i), penWidth,Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            break;
        }
        i++;
    }
    painter.drawLine(lastPointHash[id], endPoint);
    modified = true;
    int rad = 50;
    update(rect.adjusted(-rad,-rad, +rad, +rad));
    //update();
    lastPointHash[id]=endPoint;
}

void UBCanvasView::enterEvent(QEvent *event)
{
    QWidget::enterEvent(event);
    QApplication::setOverrideCursor(Qt::CrossCursor);
}

void UBCanvasView::leaveEvent(QEvent *event)
{
    QWidget::leaveEvent(event);
    QApplication::restoreOverrideCursor();
}

QList<QRect*> UBCanvasView::maxEuclideanDistance(QList<QTouchEvent::TouchPoint> touchPoints)
{
    QList<QRect*> listRect;
    foreach (const QTouchEvent::TouchPoint &touchPoint1, touchPoints) {
        //if (touchPoint1.state() == Qt::TouchPointPressed){
          foreach (const QTouchEvent::TouchPoint &touchPoint2, touchPoints) {
            int x1, y1, x2, y2;
            x1 = touchPoint1.rect().x();
            y1 = touchPoint1.rect().y();
            x2 = touchPoint2.rect().x();
            y2 = touchPoint2.rect().y();
            if( (x1 != x2) && (y1 != y2) )// && (touchPoint2.state() == Qt::TouchPointPressed) )
            {
              qreal d = euclideanDistance(x1,y1,x2,y2);
              qreal dcm = 2.54*(d/UBApplication::desktop()->logicalDpiX());
              qWarning()<<x1<<","<<y1<<" "<<x2<<","<<y2<<"   "<<d<<"px  "<<dcm<<"cm ";
              int r1 = getNumberRegion(QPoint(x1,y1));
              int r2 = getNumberRegion(QPoint(x2,y2));
              if( (dcm < 2.0) && (r1 == r2) ) listRect.append(new QRect(QPoint(x1,y1),QPoint(x2,y2)));
              //if (r1 == r2) listRect.append(new QRect(QPoint(x1,y1),QPoint(x2,y2)));
            }
          }
        //}
    }
    return listRect;
}
/*QRect* UBCanvasView::maxEuclideanDistance(QList<QTouchEvent::TouchPoint> touchPoints)
{
     if(touchPoints.count() < 2) return NULL;
     qreal maxDistance = 0;
     int x1, y1, x2, y2;
     foreach (const QTouchEvent::TouchPoint &touchPoint1, touchPoints) {
         foreach (const QTouchEvent::TouchPoint &touchPoint2, touchPoints) {
             qreal d = euclideanDistance(touchPoint1.rect().x(), touchPoint1.rect().y(), touchPoint2.rect().x(), touchPoint2.rect().y());
             if ( d > maxDistance )
             {
                   maxDistance = d;
                   x1 = touchPoint1.rect().x();
                   y1 = touchPoint1.rect().y();
                   x2 = touchPoint2.rect().x();
                   y2 = touchPoint2.rect().y();
             }
         }
     }
     int r1 = getNumberRegion(QPoint(x1,y1));
     int r2 = getNumberRegion(QPoint(x2,y2));
     qWarning()<<touchPoints.count()<<"  --> MAX ED: "<<maxDistance <<r1<<" ## "<<r2;
     if( (maxDistance < 220) && (r1 == r2) )
         return (new QRect(QPoint(x1,y1),QPoint(x2,y2)));
     else
         return NULL;
     //return maxDistance;
}*/

bool UBCanvasView::event(QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonPress:
    {
        qWarning()<<"MOUSE BUTTON PRESS";
        //qWarning()<<event->type();
        const QMouseEvent* me = static_cast<const QMouseEvent *>(event);
        qWarning()<<eraserMode.at(getNumberRegion(me->pos()));
        if(eraserMode.at(getNumberRegion(me->pos())) == true){
            QApplication::setOverrideCursor(*cursorEraser);
        }
        else{
            QApplication::setOverrideCursor(Qt::BlankCursor);
        }
        if(me->button() == Qt::LeftButton){
          scribbling=true;
          lastPoint=me->pos();
        }
        break;
    }
    case QEvent::MouseMove:
    {
        //qWarning()<<event->type();
        qWarning()<<"MOUSE MOVE";
        const QMouseEvent* me = static_cast<const QMouseEvent *>(event);

        drawLineTo(me->pos());
        if( (me->button() == Qt::LeftButton) && (scribbling==true) ){
          qWarning()<<"movement";
            drawLineTo(me->pos());
        }
        break;
    }
    case QEvent::MouseButtonRelease:
    {
        qWarning()<<"MOUSE BUTTON RELEASE";
        QApplication::restoreOverrideCursor();
        const QMouseEvent* me = static_cast<const QMouseEvent *>(event);
        if( (me->button() == Qt::LeftButton) && (scribbling == true) ){
          drawLineTo(me->pos());
          scribbling=false;
        }
        update();
        break;
    }
    case QEvent::TouchBegin:
    case QEvent::TouchUpdate:
    case QEvent::TouchEnd:
    {
        // discard a few touch after eraser-gesture to make it smoother
        /*if ( (discardNextTouch == true) && (counterNextTouch < 8) )
        {
            qWarning()<<"  --> discarding "<<counterNextTouch;
            counterNextTouch++;
            if(counterNextTouch > 8) discardNextTouch = false;
            // It is bad-practice but make the eraser mode smoother
            for (int i=0; i<100000; i++);
            return true;
        }*/

        //qWarning()<<"e: "<<event->type();
        QList<QTouchEvent::TouchPoint> touchPoints = static_cast<QTouchEvent *>(event)->touchPoints();
        QList<QRect*> listRectED = maxEuclideanDistance(touchPoints);
        foreach(QRect* rectED, listRectED)
        /*QRect* rectED = maxEuclideanDistance(touchPoints);
        if(rectED != NULL)*/
        {
          int region = getNumberRegion(rectED->center());
          if (eraserMode.at(region) == false)
          {
            qWarning()<<" ***************** ERASE GESTURE *****************";
            QPainter painter(&image);
            painter.setPen(Qt::NoPen);
            painter.setBrush(Qt::white);
            painter.drawEllipse(*rectED);
            painter.end();
            int rad = 10;
            update(rectED->adjusted(-rad,-rad, +rad, +rad));
            if(eraserGesture.at(region) == false)
            {
              eraserMode.replace(region,true);
              eraserGesture.replace(region,true);
              emit gestureErase(region);
            }
          }
        }
        //else{
            foreach (const QTouchEvent::TouchPoint &touchPoint, touchPoints) {
                switch (touchPoint.state()) {
                 case Qt::TouchPointPressed:
                    //qWarning()<<"Pressed: "<<touchPoint.rect().x()<<", "<<touchPoint.rect().y()<<", "<<touchPoint.pressure();
                    lastPointHash[touchPoint.id()]=QPoint(touchPoint.pos().x(), touchPoint.pos().y());
                    if(eraserMode.at(getNumberRegion(QPoint(touchPoint.pos().x(), touchPoint.pos().y()))) == true){
                        QApplication::setOverrideCursor(*cursorEraser);
                    }
                    else{
                        QApplication::setOverrideCursor(Qt::BlankCursor);
                    }
                    break;
                 case Qt::TouchPointReleased:
                 {
                    int region = getNumberRegion(QPoint(touchPoint.pos().x(), touchPoint.pos().y()));
                    QApplication::restoreOverrideCursor();
                    drawLineToTouch(touchPoint.id(),QPoint(touchPoint.pos().x(),touchPoint.pos().y()));
                    update();
                    if(eraserGesture.at(region) == true)
                    {
                        discardNextTouch = true;
                        counterNextTouch = 0;
                        eraserGesture.replace(region,false);
                        emit endGestureErase(region); // to switch back to pen style.
                    }

                    break;
                 }
                 case Qt::TouchPointMoved:
                 {
                    //qWarning()<<"Move: "<<touchPoint.rect().x()<<", "<<touchPoint.rect().y()<<", "<<touchPoint.pressure();
                    drawLineToTouch(touchPoint.id(),QPoint(touchPoint.pos().x(),touchPoint.pos().y()));
                    break;
                 }
                 case Qt::TouchPointStationary:
                    // don't do anything if this touch point hasn't moved
                    continue;
                }
            }
        /*}*/
        break;
    }
    default:
        return QWidget::event(event);
    }
    return true;
}
