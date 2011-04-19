
#ifndef UBSPINNINGWHEEL_H_
#define UBSPINNINGWHEEL_H_

#include <QWidget>

class UBSpinningWheel : public QWidget
{
    Q_OBJECT;

    public:
        UBSpinningWheel(QWidget *parent = 0);
        virtual ~UBSpinningWheel();
        virtual void paintEvent(QPaintEvent *event);

    public slots:
        void startAnimation();
        void stopAnimation();

    protected:
        virtual QSize sizeHint() const;
        virtual void timerEvent(QTimerEvent *event);

    private:
        QAtomicInt mPosition;
        int mTimerID;
};

#endif /* UBSPINNINGWHEEL_H_ */

