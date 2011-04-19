
#ifndef UBICONBUTTON_H_
#define UBICONBUTTON_H_

#include <QAbstractButton>

class UBIconButton : public QAbstractButton
{
    Q_OBJECT;

    public:
        UBIconButton(QWidget *parent = 0, const QIcon &iconOn = QIcon());

        void setToggleable(bool pToggleable) {mToggleable = pToggleable;}

    protected:
        virtual void paintEvent(QPaintEvent *);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseDoubleClickEvent(QMouseEvent *event);
        QSize minimumSizeHint () const;

    private:
        QSize mIconSize;
        bool mToggleable;

    signals:
        void doubleClicked();
};

#endif /* UBICONBUTTON_H_ */
