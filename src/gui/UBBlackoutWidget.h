

#ifndef UBBLACKOUTWIDGET_H_
#define UBBLACKOUTWIDGET_H_

#include <QWidget>

class UBBlackoutWidget : public QWidget
{
    Q_OBJECT

    public:
        UBBlackoutWidget(QWidget *parent = 0);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void keyPressEvent(QKeyEvent *event);

    signals:
        void activity();

    public slots:
        void doActivity();
};


#endif /* UBBLACKOUTWIDGET_H_ */
