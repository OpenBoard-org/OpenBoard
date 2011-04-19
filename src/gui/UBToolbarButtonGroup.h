
#ifndef UBTOOLBARBUTTONGROUP_H_
#define UBTOOLBARBUTTONGROUP_H_


#include <QtGui>

class UBToolbarButtonGroup : public QWidget
{
    Q_OBJECT;

    public:
        UBToolbarButtonGroup(QToolBar *toolbar, const QList<QAction*> &actions = QList<QAction*>());
        virtual ~UBToolbarButtonGroup();

        void setIcon(const QIcon &icon, int index);
        void setColor(const QColor &color, int index);
        int currentIndex() const;

    protected:
        void paintEvent(QPaintEvent *);

    private:
        QToolButton         *mToolButton; // The first button in the toolbar
        QString              mLabel;
        QList<QAction*>      mActions;
        QList<QToolButton*>  mButtons;
        int                  mCurrentIndex;
        bool                 mDisplayLabel;
        QActionGroup*        mActionGroup;

    public slots:
        void setCurrentIndex(int index);
        void colorPaletteChanged();
        void displayText(QVariant display);

    private slots:
        void selected(QAction *action);

    signals:
        void activated(int index);
        void currentIndexChanged(int index);
};

#endif /* UBTOOLBARBUTTONGROUP_H_ */
