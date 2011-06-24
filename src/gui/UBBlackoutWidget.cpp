
#include <QtGui>

#include "UBBlackoutWidget.h"

#include "core/memcheck.h"

UBBlackoutWidget::UBBlackoutWidget(QWidget *parent)
    : QWidget(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
}


void UBBlackoutWidget::mousePressEvent(QMouseEvent *event)
{
        Q_UNUSED(event);
    doActivity();
}


void UBBlackoutWidget::keyPressEvent(QKeyEvent *event)
{
    if (!event->isAccepted())
    {
        if (event->key() == Qt::Key_B)
        {
            doActivity();
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
}


void UBBlackoutWidget::doActivity()
{
    emit activity();
}
