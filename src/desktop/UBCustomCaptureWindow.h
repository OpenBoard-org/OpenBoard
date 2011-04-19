/*
 * UNCustomCaptureWindow.h
 *
 *  Created on: Feb 2, 2009
 *      Author: julienbachmann
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
