/*
 * UNCustomCaptureWindow.cpp
 *
 *  Created on: Feb 2, 2009
 *      Author: julienbachmann
 */

#include "UBCustomCaptureWindow.h"

#include "gui/UBRubberBand.h"

#include "core/memcheck.h"

UBCustomCaptureWindow::UBCustomCaptureWindow(QWidget *parent)
    : QDialog(parent, Qt::FramelessWindowHint  | Qt::Window)
    , mSelectionBand(0)
    , mRubberBandStyle(0)
    , mOrigin(0,0)
    , mIsSelecting(false)
{
    setCursor(Qt::CrossCursor);
    setWindowOpacity(0.0);
}


UBCustomCaptureWindow::~UBCustomCaptureWindow()
{
    delete mSelectionBand;
    delete mRubberBandStyle;
}


QPixmap UBCustomCaptureWindow::getSelectedPixmap()
{
    if (mSelectionBand)
    {
        return mWholeScreenPixmap.copy(mSelectionBand->geometry());
    }
    else
    {
        return QPixmap();
    }
}


int UBCustomCaptureWindow::execute(const QPixmap &pScreenPixmap)
{
    mWholeScreenPixmap = pScreenPixmap;

    QDesktopWidget *desktop = QApplication::desktop();
    int currentScreen = desktop->screenNumber(QCursor::pos());
    setGeometry(desktop->screenGeometry(currentScreen));

    showFullScreen();
    setWindowOpacity(1.0);

    return exec();
}


void UBCustomCaptureWindow::mouseMoveEvent ( QMouseEvent * event )
{
    if (mIsSelecting)
    {
        mSelectionBand->setGeometry(QRect(mOrigin, event->pos()).normalized());
    }

    event->accept();
}


void UBCustomCaptureWindow::mousePressEvent ( QMouseEvent * event )
{
    if (!mIsSelecting)
    {
        mIsSelecting = true;
        mOrigin = event->pos();

        if (!mSelectionBand)
        {
            mSelectionBand = new UBRubberBand(QRubberBand::Rectangle, this);
        }

        mSelectionBand->setGeometry(QRect(mOrigin, QSize()));
        mSelectionBand->show();
        event->accept();
    }
}


void UBCustomCaptureWindow::mouseReleaseEvent ( QMouseEvent * event )
{
    mIsSelecting = false;

    if (mSelectionBand)
    {
        mSelectionBand->hide();
    }

    event->accept();

    // do not accept very small selection
    if (!(mSelectionBand->geometry().width() < 6 && mSelectionBand->geometry().height() < 6))
    {
        accept();
    }

}


void UBCustomCaptureWindow::keyPressEvent ( QKeyEvent * event )
{
    if (event->key() == Qt::Key_Escape)
    {
        mIsSelecting = false;

        if (mSelectionBand)
        {
            mSelectionBand->hide();
        }

        event->accept();
        reject();
    }
}


void UBCustomCaptureWindow::showEvent ( QShowEvent * event )
{
    Q_UNUSED(event);
}

void UBCustomCaptureWindow::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.drawPixmap(0,0, mWholeScreenPixmap);
}
