/*
 * UNWindowCaptureHelper.h
 *
 *  Created on: Feb 3, 2009
 *      Author: julienbachmann
 */

#ifndef UBWINDOWCAPTURE_H_
#define UBWINDOWCAPTURE_H_


#include <QtGui>

class UBDesktopAnnotationController;

class UBWindowCapture : public QObject
{
    Q_OBJECT;

    public:
        UBWindowCapture(UBDesktopAnnotationController *parent = 0);
        virtual ~UBWindowCapture();
        int execute();
        const QPixmap getCapturedWindow();

    private:
        QPixmap mWindowPixmap;
        UBDesktopAnnotationController* mParent;
};
#endif /* UBWINDOWCAPTURE_H_ */
