/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef UBDOCKPALETTE_H
#define UBDOCKPALETTE_H

#include <QWidget>
#include <QMouseEvent>
#include <QBrush>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QEvent>
#include <QTime>
#include <QPoint>
#include <QPixmap>
#include <QMap>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QVector>

#include "UBDockPaletteWidget.h"

#define TABSIZE	    50       //Height of the tab of the palette
#define CLICKTIME   1000000  //Clicktime to expand or collapse paltte

/**
 * \brief The dock positions
 */
typedef enum
{
    eUBDockOrientation_Left,  /** Left dock */
    eUBDockOrientation_Right, /** Right dock */
    eUBDockOrientation_Top,   /** [to be implemented]Top dock */
    eUBDockOrientation_Bottom /** [to be implemented]Bottom dock */
}eUBDockOrientation;

typedef enum
{
    eUBDockTabOrientation_Up,   /** Up tabs */
    eUBDockTabOrientation_Down  /** Down tabs */
}eUBDockTabOrientation;

class UBDockPalette;

class UBTabDockPalete : public QWidget
{
    Q_OBJECT
    friend class UBDockPalette;

public:

    UBTabDockPalete(UBDockPalette *dockPalette, QWidget *parent = 0);
    ~UBTabDockPalete();

protected:
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *event);

private:
    UBDockPalette *dock;
    int mVerticalOffset;
    bool mFlotable;
};


typedef enum
{
    eUBDockPaletteType_LEFT,
    eUBDockPaletteType_RIGHT,
    eUBDockPaletteType_NAVIGATOR,
} eUBDockPaletteType;


class UBDockPalette : public QWidget
{
    Q_OBJECT
    friend class UBTabDockPalete;

public:
    UBDockPalette(eUBDockPaletteType paletteType, QWidget* parent=0, const char* name="UBDockPalette");
    ~UBDockPalette();

    eUBDockOrientation orientation();
    void setOrientation(eUBDockOrientation orientation);
    void setTabsOrientation(eUBDockTabOrientation orientation);
    void showTabWidget(int tabIndex);
    QRect getTabPaletteRect();

    virtual void assignParent(QWidget *widget);
    virtual void setVisible(bool visible);

//    virtual void mouseMoveEvent(QMouseEvent *event);
//    virtual void mousePressEvent(QMouseEvent *event);
//    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

    void setBackgroundBrush(const QBrush& brush);
    void registerWidget(UBDockPaletteWidget* widget);

    void addTab(UBDockPaletteWidget* widget);
    void removeTab(UBDockPaletteWidget* widget);

    void connectSignals();

    bool switchMode(eUBDockPaletteWidgetMode mode);

    QVector<UBDockPaletteWidget*> GetWidgetsList() { return mRegisteredWidgets; }

public:
    bool isTabFlotable() {return mTabPalette->mFlotable;}
    void setTabFlotable(bool newFlotable) {mTabPalette->mFlotable = newFlotable;}
    int getAdditionalVOffset() const {return mTabPalette->mVerticalOffset;}
    void setAdditionalVOffset(int newOffset) {mTabPalette->mVerticalOffset = newOffset;}

    eUBDockPaletteType paletteType(){return mPaletteType;}

public slots:
    void onShowTabWidget(UBDockPaletteWidget* widget);
    void onHideTabWidget(UBDockPaletteWidget* widget);
    void onAllDownloadsFinished();

protected:
    virtual int border();
    virtual int radius();
    virtual int customMargin();
    virtual void updateMaxWidth();
    virtual void resizeEvent(QResizeEvent *event);
    virtual int collapseWidth();

    /** The current dock orientation */
    eUBDockOrientation mOrientation;
    /** The current background brush */
    QBrush mBackgroundBrush;
    /** The preferred width */
    int mPreferredWidth;
    /** The preferred height */
    int mPreferredHeight;
    /** A flag used to allow the resize */
    bool mCanResize;
    /** A flag indicating if the palette has been resized between a click and a release */
    bool mResized;
    /** The width that trig the collapse */
    int mCollapseWidth;
    /** The last width of the palette */
    int mLastWidth;
    /** The click time*/
    QTime mClickTime;
    /** The mouse pressed position */
    QPoint mMousePressPos;
    /** The tab orientation */
    eUBDockTabOrientation mTabsOrientation;
    /** The h position of the tab */
    int mHTab;
    /** The stacked widget */
    QStackedWidget* mpStackWidget;
    /** The layout */
    QVBoxLayout* mpLayout;
    /** The current tab index */
    int mCurrentTab;
    /** The visible tab widgets */
    QVector<UBDockPaletteWidget*> mTabWidgets;
    /** The current widget */
    QVector<UBDockPaletteWidget*> mRegisteredWidgets;
    /** The current tab widget */
    QString mCrntTabWidget;

private slots:
    void onToolbarPosUpdated();
    void onResizeRequest(QResizeEvent* event);

private:
    void tabClicked(int tabIndex);
    int tabSpacing();
    void toggleCollapseExpand();
    void moveTabs();
    void resizeTabs();

private:
    UBTabDockPalete *mTabPalette;
    eUBDockPaletteType mPaletteType;
};

#endif // UBDOCKPALETTE_H
