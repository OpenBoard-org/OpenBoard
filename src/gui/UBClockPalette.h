
#ifndef UBCLOCKPALLETTE_H_
#define UBCLOCKPALLETTE_H_

class QHBoxLayout;
class QLabel;

#include "UBFloatingPalette.h"

class UBClockPalette : public UBFloatingPalette
{
    Q_OBJECT;

    public:
        UBClockPalette(QWidget *parent = 0);
        virtual ~UBClockPalette();

    protected:
        int radius();
        void timerEvent(QTimerEvent *event);

        virtual void showEvent ( QShowEvent * event );
        virtual void hideEvent ( QShowEvent * event );


    protected slots:
        void updateTime();

    private:

        QLabel *mTimeLabel;
        int mTimerID;
        QString mTimeFormat;

};

#endif /* UBCLOCKPALLETTE_H_ */
