/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */



#include <QApplication>
#include <QDesktopWidget>
#include <QStyle>

#include "UBCustomCaptureWindow.h"

#include "frameworks/UBPlatformUtils.h"
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
        QRect r = mSelectionBand->geometry();
        return mWholeScreenPixmap.copy( r.x()       *mWholeScreenPixmap.devicePixelRatio(),
                                        r.y()       *mWholeScreenPixmap.devicePixelRatio(),
                                        r.width()   *mWholeScreenPixmap.devicePixelRatio(),
                                        r.height()  *mWholeScreenPixmap.devicePixelRatio());
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
    this->show();
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
