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




#ifndef UBCUSTOMCAPTUREWINDOW_H_
#define UBCUSTOMCAPTUREWINDOW_H_

#include <QtGui>
#include <QDialog>
#include <QRubberBand>

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
