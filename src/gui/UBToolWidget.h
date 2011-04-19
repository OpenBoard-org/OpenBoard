/*
 * UBToolWidget.h
 *
 *  Created on: Feb 18, 2009
 *      Author: luc
 */

#ifndef UBTOOLWIDGET_H_
#define UBTOOLWIDGET_H_

#include <QtGui>

class UBAbstractWidget;
class QWidget;
class UBGraphicsScene;

class UBToolWidget : public QWidget
{
    Q_OBJECT;

    public:
        UBToolWidget(const QUrl& pUrl, QWidget* pParent = 0);
        UBToolWidget(UBAbstractWidget* pWidget, QWidget* pParent = 0);
        virtual ~UBToolWidget();

        void centerOn(const QPoint& pos);

        QPoint naturalCenter() const;

        UBAbstractWidget* webWidget() const;

    protected:
        void initialize();
        virtual void paintEvent(QPaintEvent *);

        virtual void mousePressEvent ( QMouseEvent * event );
        virtual void mouseMoveEvent ( QMouseEvent * event );
        virtual void mouseReleaseEvent ( QMouseEvent * event );

        virtual bool eventFilter(QObject *obj, QEvent *event);

    private slots:
        void javaScriptWindowObjectCleared();

    protected:

        static QPixmap *sClosePixmap;
        static QPixmap *sUnpinPixmap;

        UBAbstractWidget *mToolWidget;

        QPoint mMousePressPos;

        bool mShouldMoveWidget;

        int mContentMargin;
        int mFrameWidth;
};

#endif /* UBTOOLWIDGET_H_ */
