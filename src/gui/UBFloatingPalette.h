

#ifndef UBFLOATINGPALLETTE_H_
#define UBFLOATINGPALLETTE_H_

#include <QWidget>
#include <QPoint>

typedef enum
{
    eMinimizedLocation_None,
    eMinimizedLocation_Left,
    eMinimizedLocation_Top,
    eMinimizedLocation_Right,
    eMinimizedLocation_Bottom
}eMinimizedLocation;

class UBFloatingPalette : public QWidget
{
    Q_OBJECT

    public:

        UBFloatingPalette(Qt::Corner = Qt::TopLeftCorner, QWidget *parent = 0);

        virtual void mouseMoveEvent(QMouseEvent *event);
        virtual void mousePressEvent(QMouseEvent *event);
        virtual void mouseReleaseEvent(QMouseEvent *event);

        /**
         * Add another floating palette to the associated palette. All associated palettes will have the same width
         * that is calculated as the minimum width of all associated palettes.
         */
        void addAssociatedPalette(UBFloatingPalette* pOtherPalette);
        void removeAssociatedPalette(UBFloatingPalette* pOtherPalette);

        virtual void adjustSizeAndPosition(bool pUp = true);

        void setCustomPosition(bool pFlag);

        QSize preferredSize();

        void setBackgroundBrush(const QBrush& brush);
        void setMinimizePermission(bool permission);

    protected:

        virtual void enterEvent(QEvent *event);
        virtual void showEvent(QShowEvent *event);
        virtual void paintEvent(QPaintEvent *event);

        virtual int radius();
        virtual int border();
        virtual int gripSize();

        QBrush mBackgroundBrush;
        bool mbGrip;
        static const int sLayoutContentMargin = 12;
        static const int sLayoutSpacing = 15;
        void moveInsideParent(const QPoint &position);
        bool mCustomPosition;

    private:
        void removeAllAssociatedPalette();
        void minimizePalette(const QPoint& pos);

        QList<UBFloatingPalette*> mAssociatedPalette;
        QPoint mDragPosition;
        bool mIsMoving;
        bool mCanBeMinimized;
	eMinimizedLocation mMinimizedLocation;
        Qt::Corner mDefaultPosition;

    signals:
        void mouseEntered();
	void minimizeStart(eMinimizedLocation location);
	void maximizeStart();
        void maximized();
};


#endif /* UBFLOATINGPALLETTE_H_ */
