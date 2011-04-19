#ifndef UBWINDOWCAPTUREDELEGATE_H_
#define UBWINDOWCAPTUREDELEGATE_H_

#include <QtGui>

class UBWindowCaptureDelegate : public QObject
{
        Q_OBJECT;

    public:
        UBWindowCaptureDelegate(QObject *parent = 0);
        ~UBWindowCaptureDelegate();

        int execute();
        const QPixmap getCapturedWindow();

    private:
        bool eventFilter(QObject *target, QEvent *event);
        void processPos(QPoint pPoint);
        void drawSelectionRect();

        bool mIsCapturing;
        bool mCancel;
        QPoint mLastPoint;
        HWND mCurrentWindow;
        QPixmap mCapturedPixmap;

};
#endif /* UBWINDOWCAPTUREDELEGATE_H_ */
