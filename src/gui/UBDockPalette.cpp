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
#include <QPoint>
#include <QPointF>
#include <QPainterPath>

#include "UBDockPalette.h"
#include "core/UBSettings.h"
#include "frameworks/UBPlatformUtils.h"
#include "core/UBApplication.h"
#include "core/UBPreferencesController.h"
#include "core/UBDownloadManager.h"

#include "core/memcheck.h"

/**
 * \brief The constructor
 */
UBDockPalette::UBDockPalette(QWidget *parent, const char *name)
    :QWidget(parent, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint)
    , mOrientation(eUBDockOrientation_Left)
    , mPreferredWidth(100)
    , mPreferredHeight(100)
    , mCanResize(false)
    , mResized(false)
    , mCollapseWidth(150)
    , mLastWidth(-1)
    , mHTab(0)
    , mpStackWidget(NULL)
    , mpLayout(NULL)
    , mCurrentTab(0)
{
    setObjectName(name);

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    mpStackWidget = new QStackedWidget(this);
    mpLayout->addWidget(mpStackWidget);

    // clear the tab widgets
    mTabWidgets.clear();

    // We let 2 pixels in order to keep a small border for the resizing
    setMinimumWidth(2*border() + 2);

    if (parent)
    {
        setAttribute(Qt::WA_NoMousePropagation);
        setAttribute(Qt::WA_TranslucentBackground);
    }
    else
    {
        // standalone window
        setAttribute(Qt::WA_TranslucentBackground);
    }

    mBackgroundBrush = QBrush(UBSettings::paletteColor);

    // This is the only way to set the background as transparent!
    setStyleSheet("QWidget {background-color: transparent}");

    // Set the position of the tab
    onToolbarPosUpdated();
    connect(UBSettings::settings()->appToolBarPositionedAtTop, SIGNAL(changed(QVariant)), this, SLOT(onToolbarPosUpdated()));
    connect(UBDownloadManager::downloadManager(), SIGNAL(allDownloadsFinished()), this, SLOT(onAllDownloadsFinished()));
}

/**
 * \brief The destructor
 */
UBDockPalette::~UBDockPalette()
{
    if(NULL != mpStackWidget)
    {
        delete mpStackWidget;
        mpStackWidget = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}

/**
 * \brief Get the current orientation
 * @return the current orientation
 */
eUBDockOrientation UBDockPalette::orientation()
{
    return mOrientation;
}

/**
 * \brief Set the current orientation
 * @param orientation as the given orientation
 */
void UBDockPalette::setOrientation(eUBDockOrientation orientation)
{
    // Set the size
    mOrientation = orientation;

    if(orientation == eUBDockOrientation_Left || orientation == eUBDockOrientation_Right)
    {
	setMaximumHeight(parentWidget()->height());
	setMinimumHeight(maximumHeight());
    }
    else if(orientation == eUBDockOrientation_Top || orientation == eUBDockOrientation_Bottom)
    {
	setMaximumWidth(parentWidget()->width());
	setMinimumWidth(maximumWidth());
    }

}

/**
 * \brief Handle the mouse move event
 * @param as the mouse event
 */
void UBDockPalette::mouseMoveEvent(QMouseEvent *event)
{
    QPoint p = event->pos();

    if(mCanResize && ((mMousePressPos - p).manhattanLength() > QApplication::startDragDistance()))
    {
	switch(mOrientation)
	{
	case eUBDockOrientation_Left:
            if(p.x() < collapseWidth() && p.x() >= minimumWidth())
            {
                resize(border(), height());
                mLastWidth = collapseWidth() + 1;
                mResized = true;
            }
            else if(p.x() <= maximumWidth() && p.x() >= minimumWidth())
	    {
		resize(p.x(), height());
                mResized = true;
	    }
	    break;
	case eUBDockOrientation_Right:
            if((this->x() + p.x() > parentWidget()->width() - collapseWidth()) && (this->x() + p.x() < parentWidget()->width()))
            {
                resize(border(), height());
                mLastWidth = collapseWidth() + 1;
                mResized = true;
            }
            else if((this->x() + p.x() >= parentWidget()->width() - maximumWidth()) && (this->x() + p.x() <= parentWidget()->width() - this->minimumWidth()))
	    {
		resize(parentWidget()->width() - (this->x() + p.x()), height());
                mResized = true;
	    }

	    break;
	case eUBDockOrientation_Top:
	case eUBDockOrientation_Bottom:
	    if(p.y() <= maximumHeight())
	    {
		resize(width(), p.y());
                mResized = true;
	    }
	    break;

	default:
	    break;
	}
    }
}

/**
 * \brief Handle the mouse press event
 * @param as the mouse event
 */
void UBDockPalette::mousePressEvent(QMouseEvent *event)
{
    mClickTime = QTime::currentTime();
    // The goal here is to verify if the user can resize the widget.
    // It is only possible to resize it if the border is selected
    QPoint p = event->pos();
    mMousePressPos = p;
    mResized = false;

    switch(mOrientation)
    {
    case eUBDockOrientation_Left:
        if((p.x() >= width() - 2 * border()) && (p.x() <= width()))
	{
	    mCanResize = true;
	}
	break;
    case eUBDockOrientation_Right:
        if((p.x() >= 0) && (p.x() <= 2 * border()))
	{
	    mCanResize = true;
	}
	break;
    case eUBDockOrientation_Top:
	// Not supported yet
	break;
    case eUBDockOrientation_Bottom:
	// Not supported yet
	break;
    default:
	break;
    }
}

/**
 * \brief Handle the mouse release event
 * @param event as the mouse event
 */
void UBDockPalette::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(!mResized && mClickTime.elapsed() < CLICKTIME)
    {
        int nbTabs = mTabWidgets.size();
        int clickedTab = 0;
        // If the clicked position is in the tab, perform the related action
        if(eUBDockOrientation_Left == mOrientation)
        {
            if(mMousePressPos.x() >= width() - 2*border() &&
               mMousePressPos.y() >= mHTab &&
               mMousePressPos.x() <= width() &&
               mMousePressPos.y() <= mHTab + nbTabs*TABSIZE + (nbTabs -1)*tabSpacing())
            {
                clickedTab = (mMousePressPos.y() - mHTab)/(TABSIZE+tabSpacing());
                tabClicked(clickedTab);
            }
        }
        else if(eUBDockOrientation_Right == mOrientation)
        {
            if(mMousePressPos.x() >= 0 &&
               mMousePressPos.x() <= 2*border() &&
               mMousePressPos.y() >= mHTab &&
               mMousePressPos.y() <= mHTab + nbTabs*TABSIZE + (nbTabs -1)*tabSpacing())
            {
                clickedTab = (mMousePressPos.y() - mHTab)/(TABSIZE+tabSpacing());
                tabClicked(clickedTab);
            }
        }
    }

    mCanResize = false;
}

/**
 * \brief Handle the resize event
 * @param event as the resize event
 */
void UBDockPalette::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    updateMaxWidth();
    if(parentWidget())
    {
        setMinimumHeight(parentWidget()->height());
    }
	// Set the position
    QPoint origin;
    switch(mOrientation)
    {
    case eUBDockOrientation_Right:
		origin.setX(parentWidget()->width() - this->width());
		origin.setY(0);
	break;
    case eUBDockOrientation_Bottom:
	// Not supported yet
    case eUBDockOrientation_Top:
	// Not supported yet
    case eUBDockOrientation_Left:
    default:
	origin.setX(0);
	origin.setY(0);
	break;
    }
    move(origin.x(), origin.y());
}

/**
 * \brief Handle the mouse enter event
 * @param event as the mouse event
 */
void UBDockPalette::enterEvent(QEvent *event)
{
    Q_UNUSED(event);
    // We want to set the cursor as an arrow everytime it enters the palette
    setCursor(Qt::ArrowCursor);
}

/**
 * \brief Handle the mouse leave event
 * @param event as the mouse event
 */
void UBDockPalette::leaveEvent(QEvent *event)
{
    Q_UNUSED(event);
    // Restore the cursor to its previous shape
    unsetCursor();
}

/**
 * \brief Draw the palette
 * @param event as the paint event
 */
void UBDockPalette::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(mBackgroundBrush);
    QPainterPath path;
    path.setFillRule(Qt::WindingFill);

    int nbTabs = mTabWidgets.size();
    if(0 < nbTabs)
    {
        // First draw the BIG RECTANGLE (I write it big because the rectangle is big...)
        if(mOrientation == eUBDockOrientation_Left)
        {
            path.addRect(0.0, 0.0, width()-2*border(), height());
        }
        else if(mOrientation == eUBDockOrientation_Right)
        {
            path.addRect(2*border(), 0.0, width()-2*border(), height());
        }

        // THEN DRAW THE small tabs (yes, the tabs are small...)
        if(eUBDockTabOrientation_Up == mTabsOrientation)
        {
            mHTab = border();
        }
        else
        {
            mHTab = height() - border() - nbTabs*TABSIZE - (nbTabs-1)*tabSpacing();
        }

        for(int i = 0; i < mTabWidgets.size(); i++)
        {
            UBDockPaletteWidget* pCrntWidget = mTabWidgets.at(i);
            if(NULL != pCrntWidget)
            {
                if(mOrientation == eUBDockOrientation_Left)
                {
                    path.addRoundedRect(width()-4*border(), mHTab + i*TABSIZE + i*tabSpacing(), 4*border(), TABSIZE, radius(), radius());
                    painter.drawPath(path);
                    QPixmap iconPixmap;
                    if(mCollapseWidth >= width())
                    {
                        // Get the collapsed icon
                        iconPixmap = pCrntWidget->iconToRight();
                    }
                    else
                    {
                        // Get the expanded icon
                        iconPixmap = pCrntWidget->iconToLeft();
                    }
                    painter.drawPixmap(width() - 2*border() + 1, mHTab + i*TABSIZE + i*tabSpacing() + 1 , 2*border() - 4, TABSIZE - 2, iconPixmap);
                }
                else if(mOrientation == eUBDockOrientation_Right)
                {
                    path.addRoundedRect(0.0, mHTab + i*TABSIZE + i*tabSpacing(), 4*border(), TABSIZE, radius(), radius());
                    painter.drawPath(path);
                    QPixmap iconPixmap;
                    if(mCollapseWidth >= width())
                    {
                        // Get the collapsed icon
                        iconPixmap = pCrntWidget->iconToLeft();
                    }
                    else
                    {
                        // Get the expanded icon
                        iconPixmap = pCrntWidget->iconToRight();
                    }
                    painter.drawPixmap(2, mHTab + i*TABSIZE + i*tabSpacing() + 1, 2*border() - 4, TABSIZE - 2, iconPixmap);
                }
                else
                {
                    painter.drawRoundedRect(border(), border(), width() - 2 * border(), height() - 2 * border(), radius(), radius());
                }
            }
        }
    }
}

/**
 * \brief Set the background brush
 * @param brush as the given brush
 */
void UBDockPalette::setBackgroundBrush(const QBrush &brush)
{
    if (mBackgroundBrush != brush)
    {
	mBackgroundBrush = brush;
	update();
    }
}

/**
 * Get the border size
 * @return the border size
 */
int UBDockPalette::border()
{
    return 15;
}

/**
 * \brief Get the radius size
 * @return the radius size
 */
int UBDockPalette::radius()
{
    return 5;
}

/**
 * \brief Set the maximum width
 */
void UBDockPalette::updateMaxWidth()
{
    // Only the inherited class will overload this method
}

/**
 * \brief Get the collapse width value
 * @return the collapse widht value
 */
int UBDockPalette::collapseWidth()
{
    return mCollapseWidth;
}

/**
 * \brief collapse/expand automatically the palette after a click on its tab
 */
void UBDockPalette::tabClicked(int tabIndex)
{
    // If the current tab is not the clicked one, show its content
    if(mCurrentTab != tabIndex)
    {
        showTabWidget(tabIndex);
    }
    // else collapse the palette
    else
    {
        toggleCollapseExpand();
    }
}

/**
 * \brief Show the tab widget related to the given index
 * @param tabIndex as the given index
 */
void UBDockPalette::showTabWidget(int tabIndex)
{
    mpStackWidget->setCurrentIndex(tabIndex);
    mCurrentTab = tabIndex;

    // Expand it if collapsed
    if(mLastWidth != -1)
    {
        toggleCollapseExpand();
    }

    // Update the current tab index
    if(NULL != (dynamic_cast<UBDockPaletteWidget*>(mpStackWidget->widget(tabIndex)))){
        mCrntTabWidget = dynamic_cast<UBDockPaletteWidget*>(mpStackWidget->widget(tabIndex))->name();
    }

}

/**
 * \brief Toggle the collapse / expand state
 */
void UBDockPalette::toggleCollapseExpand()
{
    if(mLastWidth == -1)
    {
        // The palette must be collapsed
        mLastWidth = width();
        resize(2*border(), height());
    }
    else
    {
        // The palette will be expanded
        resize(mLastWidth, height());
        mLastWidth = -1;
    }
}

/**
 * \brief Set the tabs orientation
 * @param orientation as the given tabs orientation
 */
void UBDockPalette::setTabsOrientation(eUBDockTabOrientation orientation)
{
    mTabsOrientation = orientation;
}

/**
 * \brief Update the tab position regarding the toolbar position (up or down)
 */
void UBDockPalette::onToolbarPosUpdated()
{
    // Get the position of the tab
    if(UBSettings::settings()->appToolBarPositionedAtTop->get().toBool())
    {
        setTabsOrientation(eUBDockTabOrientation_Up);
    }
    else
    {
        setTabsOrientation(eUBDockTabOrientation_Down);
    }
    update();
}

/**
 * \brief Get the custom margin
 * @return the custom margin value
 */
int UBDockPalette::customMargin()
{
    return 5;
}

/**
 * \brief Add the given tab widget
 * @param widget as the given widget
 */
void UBDockPalette::addTab(UBDockPaletteWidget *widget)
{
    if(!mTabWidgets.contains(widget) && widget->visibleState())
    {
        widget->setVisible(true);
        mTabWidgets.append(widget);
        mpStackWidget->addWidget(widget);
        mpStackWidget->setCurrentWidget(widget);
        update();
    }
}
/**
 * \brief Remove the given tab
 * @param widgetName as the tab widget name
 */
void UBDockPalette::removeTab(const QString &widgetName)
{
    for(int i = 0; i < mTabWidgets.size(); i++)
    {
        UBDockPaletteWidget* pCrntWidget = mTabWidgets.at(i);
        if(NULL != pCrntWidget && (pCrntWidget->name() == widgetName))
        {
            mpStackWidget->removeWidget(pCrntWidget);
            mTabWidgets.remove(i);
            pCrntWidget->hide();
            update();
            break;
        }
    }
}

/**
 * \brief Handle the resize request
 * @param event as the given resize request
 */
void UBDockPalette::onResizeRequest(QResizeEvent *event)
{
    resizeEvent(event);
}

/**
 * \brief Get the tab spacing
 * @return the tab spacing
 */
int UBDockPalette::tabSpacing()
{
    return 2;
}

/**
 * \brief Show the given widget
 * @param widgetName as the given widget name
 */
void UBDockPalette::onShowTabWidget(const QString &widgetName)
{
    for(int i = 0; i < mRegisteredWidgets.size(); i++)
    {
        UBDockPaletteWidget* pCrntWidget = mRegisteredWidgets.at(i);
        if(NULL != pCrntWidget && (pCrntWidget->name() == widgetName))
        {
            pCrntWidget->setVisibleState(true);
            addTab(pCrntWidget);
            break;
        }
    }
}

/**
 * \brief Hide the given widget
 * @param widgetName as the given widget name
 */
void UBDockPalette::onHideTabWidget(const QString &widgetName)
{
    for(int i = 0; i < mRegisteredWidgets.size(); i++){
        UBDockPaletteWidget* pCrntWidget = mRegisteredWidgets.at(i);
        if(NULL != pCrntWidget && (pCrntWidget->name() == widgetName)){
            pCrntWidget->setVisibleState(false);
            break;
        }
    }
    removeTab(widgetName);
}

/**
 * \brief Connect the show / hide signals of the widget related to this dock palette
 */
void UBDockPalette::connectSignals()
{
    for(int i=0; i < mRegisteredWidgets.size(); i++)
    {
        connect(mRegisteredWidgets.at(i), SIGNAL(showTab(QString)), this, SLOT(onShowTabWidget(QString)));
        connect(mRegisteredWidgets.at(i), SIGNAL(hideTab(QString)), this, SLOT(onHideTabWidget(QString)));
    }
}

/**
 * \brief Register the given widget
 * @param widget as the given widget
 */
void UBDockPalette::registerWidget(UBDockPaletteWidget *widget)
{
    if(!mRegisteredWidgets.contains(widget))
    {
        // Update the parent of this widget
        widget->setParent(this);
        mRegisteredWidgets.append(widget);

        // By default, the widget is hidden
        widget->hide();
    }
}

/**
  * \brief Handles the 'all download finished' notification
  */
void UBDockPalette::onAllDownloadsFinished()
{
    for(int i=0; i<mTabWidgets.size(); i++){
        UBDockPaletteWidget* pW = mTabWidgets.at(i);
        if(NULL != pW && mCrntTabWidget == pW->name()){
            mpStackWidget->setCurrentWidget(pW);
        }
    }
}
