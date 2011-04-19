
#ifndef UBIDLETIMER_H_
#define UBIDLETIMER_H_

#include <QObject>
#include <QDateTime>

class QEvent;

class UBIdleTimer : public QObject
{
    Q_OBJECT

    public:

        UBIdleTimer(QObject *parent = 0);
        virtual ~UBIdleTimer();

    protected:

        bool eventFilter(QObject *obj, QEvent *event);
        virtual void timerEvent(QTimerEvent *event);

    private:

        QDateTime mLastInputEventTime;
        bool mCursorIsHidden;
};


#endif /* UBIDLETIMER_H_ */
