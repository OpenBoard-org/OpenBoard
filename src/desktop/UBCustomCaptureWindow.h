/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBCUSTOMCAPTUREWINDOW_H_
#define UBCUSTOMCAPTUREWINDOW_H_

#include <QtGui>
/**
* This class is used to do the custom capture of Uninotes
**/
class UBCustomCaptureWindow : public QDialog
{

    Q_OBJECT;

    public:
        UBCustomCaptureWindow(QWidget *parent = 0);
        virtual ~UBCustomCaptureWindow();

        // launch the custom capture window based on pSreenPixmap.
        // The function returns a DialogCode result.
        int execute(const QPixmap &pScreenPixmap);
        // return the selected area pixmap.
        QPixmap getSelectedPixmap();

    protected:
        virtual void showEvent ( QShowEvent * event );
        virtual void mouseMoveEvent ( QMouseEvent * event );
        virtual void mousePressEvent ( QMouseEvent * event );
        virtual void mouseReleaseEvent ( QMouseEvent * event );
        virtual void keyPressEvent ( QKeyEvent * event );
        virtual void paintEvent(QPaintEvent *event);

        QPixmap mWholeScreenPixmap;
        QRubberBand *mSelectionBand;
        QStyle *mRubberBandStyle;
        QPoint mOrigin;
        bool mIsSelecting;
};

#endif /* UBCUSTOMCAPTUREWINDOW_H_ */
