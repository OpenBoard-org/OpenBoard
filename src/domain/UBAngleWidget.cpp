#include "UBAngleWidget.h"
#include <QPainter>

#include "core/memcheck.h"

UBAngleWidget::UBAngleWidget(QWidget *parent)
    : QWidget(parent)
{
    this->setFixedSize(45,30);
    this->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    QImage mask_img(width(), height(), QImage::Format_Mono);
    mask_img.fill(0xff);
    QPainter mask_ptr(&mask_img);
    mask_ptr.setBrush( QBrush( QColor(0, 0, 0) ) );
    mask_ptr.drawRoundedRect(0,0,this->geometry().width() - 6,this->geometry().height() - 6,1,1);
    bmpMask = QBitmap::fromImage(mask_img);
    this->setMask(bmpMask);
}

UBAngleWidget::~UBAngleWidget()
{

}

void UBAngleWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    this->move(this->cursor().pos().x(), this->cursor().pos().y() - 30);

    QPainter painter(this);
    painter.setRenderHints(QPainter::Antialiasing | QPainter::SmoothPixmapTransform);
    QBrush brush(Qt::white);
    painter.setBrush(brush);
    painter.drawRoundedRect(1,1,this->geometry().width() - 10,this->geometry().height() - 10,1,1);

    painter.setPen(QColor(85,50,127));
    painter.setFont(QFont("Arial", 10));
    painter.drawText(1,1,this->geometry().width() - 10,this->geometry().height() - 10, Qt::AlignCenter, text);
}

void UBAngleWidget::setText(QString newText)
{
    text = newText;
    text.append(QChar(176));
}