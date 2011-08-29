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

#include "UBDockPaletteWidget.h"

#define TABSIZE	    50
#define CLICKTIME   1000000

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

class UBDockPalette : public QWidget
{
    Q_OBJECT
public:
    UBDockPalette(QWidget* parent=0, const char* name="UBDockPalette");
    ~UBDockPalette();

    eUBDockOrientation orientation();
    void setOrientation(eUBDockOrientation orientation);
    void setTabsOrientation(eUBDockTabOrientation orientation);

    virtual void mouseMoveEvent(QMouseEvent *event);
    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void paintEvent(QPaintEvent *event);
    virtual void enterEvent(QEvent *);
    virtual void leaveEvent(QEvent *);

    void setBackgroundBrush(const QBrush& brush);
    void addTabWidget(const QString& widgetName, UBDockPaletteWidget* widget);
    void removeTab(const QString& widgetName);

protected:
    virtual int border();
    virtual int radius();
    virtual int customMargin();
    virtual void updateMaxWidth();
    virtual void resizeEvent(QResizeEvent *event);
    virtual int collapseWidth();

    /** The current dock orientation */
    eUBDockOrientation mOrientation;
    /** The current baclground brush */
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
    ///** The palette icon */
    //QPixmap mIcon;
    /** The tab orientation */
    eUBDockTabOrientation mTabsOrientation;
    /** The h position of the tab */
    int mHTab;
    /** The tab widgets */
    QMap<QString, UBDockPaletteWidget*> mTabWidgets;
    /** The stacked widget */
    QStackedWidget* mpStackWidget;

private slots:
    void onToolbarPosUpdated();

private:
    void tabClicked();
};

#endif // UBDOCKPALETTE_H
