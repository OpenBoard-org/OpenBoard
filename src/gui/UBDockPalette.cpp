/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include <QPoint>
#include <QPointF>
#include <QPainterPath>

#include "UBDockPalette.h"

#include "frameworks/UBPlatformUtils.h"

#include "core/UBSettings.h"
#include "core/UBApplication.h"
#include "core/UBPreferencesController.h"
#include "core/UBDownloadManager.h"

#include "board/UBBoardController.h"

#include "core/memcheck.h"

/**
 * \brief The constructor
 */
UBDockPalette::UBDockPalette(eUBDockPaletteType paletteType, QWidget *parent, const char *name)
:QWidget(parent, Qt::FramelessWindowHint | Qt::X11BypassWindowManagerHint)
, mCurrentMode(eUBDockPaletteWidget_BOARD)
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
, mPaletteType(paletteType)
, mTabPalette(new UBTabDockPalette(this, parent))
{
    setObjectName(name);

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    mpStackWidget = new QStackedWidget(this);
    mpLayout->addWidget(mpStackWidget);

    // clear the tab widgets
    mTabWidgets.clear();

    // We let 2 pixels in order to keep a small border for the resizing
    setMinimumWidth(0);

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

    connect(UBApplication::boardController,SIGNAL(documentSet(UBDocumentProxy*)),this,SLOT(onDocumentSet(UBDocumentProxy*)));
    connect(this,SIGNAL(pageSelectionChangedRequired()),UBApplication::boardController,SLOT(selectionChanged()));
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

void UBDockPalette::onDocumentSet(UBDocumentProxy* documentProxy)
{
    Q_UNUSED(documentProxy);
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
    moveTabs();
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
    emit mouseEntered();
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
            path.addRect(0.0, 0.0, width(), height());
        }
        else if(mOrientation == eUBDockOrientation_Right)
        {
            path.addRect(0.0, 0.0, width(), height());
        }

        painter.drawPath(path);
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
    mTabPalette->update();
    emit pageSelectionChangedRequired();
}

/**
 * \brief Show the tab widget related to the given index
 * @param tabIndex as the given index
 */
void UBDockPalette::showTabWidget(int tabIndex)
{
    mpStackWidget->setCurrentIndex(tabIndex);
    mCurrentTab = tabIndex;

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
    if(width() < mCollapseWidth)
        resize(mLastWidth,height());
    else{
        mLastWidth = width();
        update();
        resize(0,height());
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
    moveTabs();
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
        resizeTabs();
        update();
    }
}
/**
 * \brief Remove the given tab
 * @param widgetName as the tab widget name
 */
void UBDockPalette::removeTab(UBDockPaletteWidget* widget)
{
    int nWidget = mTabWidgets.indexOf(widget);
    if( nWidget >= 0 )
    {
        mpStackWidget->removeWidget(widget);
        mTabWidgets.remove(nWidget);
        widget->hide();
        update();
    }
    resizeTabs();
    mCurrentTab = qMax(mCurrentTab - 1, 0);
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
void UBDockPalette::onShowTabWidget(UBDockPaletteWidget* widget)
{
    if (mRegisteredWidgets.contains(widget))
    {
        widget->setVisibleState(true);
        addTab(widget);
        moveTabs();
    }
}

/**
 * \brief Hide the given widget
 * @param widgetName as the given widget name
 */
void UBDockPalette::onHideTabWidget(UBDockPaletteWidget* widget)
{
    if (mRegisteredWidgets.contains(widget))
    {
        widget->setVisibleState(false);
        removeTab(widget);
        moveTabs();
    }
}

/**
 * \brief Connect the show / hide signals of the widget related to this dock palette
 */
void UBDockPalette::connectSignals()
{
    for(int i=0; i < mRegisteredWidgets.size(); i++)
    {
        connect(mRegisteredWidgets.at(i), SIGNAL(showTab(UBDockPaletteWidget*)), this, SLOT(onShowTabWidget(UBDockPaletteWidget*)));
        connect(mRegisteredWidgets.at(i), SIGNAL(hideTab(UBDockPaletteWidget*)), this, SLOT(onHideTabWidget(UBDockPaletteWidget*)));
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

void UBDockPalette::moveTabs()
{
    int x = width();
    if (mOrientation == eUBDockOrientation_Right)
    {
        if (parentWidget())
            x = parentWidget()->width() - width() - border() * 2;
    }

    int y = border();
    if(eUBDockTabOrientation_Down == mTabsOrientation)
    {
        if (mOrientation == eUBDockOrientation_Right)
        {
            if (parentWidget())
                y = parentWidget()->height() - border()- mTabWidgets.size() * TABSIZE;
        }
        else
            y = height() - border()- mTabWidgets.size() * TABSIZE;
    }

    mHTab = y;

    mTabPalette->move(x,y + mTabPalette->mVerticalOffset);
}
void UBDockPalette::resizeTabs()
{
    int numTabs = mTabWidgets.size();
    mTabPalette->setFixedSize(2 * border(), (numTabs * TABSIZE) + qMax(numTabs - 1, 0) * tabSpacing());
}
QRect UBDockPalette::getTabPaletteRect()
{
    QRect tabsRect = mTabPalette->rect();
    QPoint topLeft = mTabPalette->mapToParent(tabsRect.topLeft());
    QPoint bottomRight = mTabPalette->mapToParent(tabsRect.bottomRight());

    return QRect(topLeft, bottomRight);
}
void UBDockPalette::assignParent(QWidget *widget)
{
    setParent(widget);
    mTabPalette->setParent(widget);
}
void UBDockPalette::setVisible(bool visible)
{
    QWidget::setVisible(visible);
    mTabPalette->setVisible(visible);
}

bool UBDockPalette::switchMode(eUBDockPaletteWidgetMode mode)
{
    mLastOpenedTabForMode.insert(mCurrentMode, mpStackWidget->currentIndex());
    mCurrentMode = mode;
    bool hasVisibleElements = false;
    //-------------------------------//
    // get full palette widgets list, parse it, show all widgets for BOARD mode, and hide all other
    for(int i = 0; i < mRegisteredWidgets.size(); i++)
    {
        UBDockPaletteWidget* pNextWidget = mRegisteredWidgets.at(i);
        if( pNextWidget != NULL )
        {
            if( pNextWidget->visibleInMode(mode) )
            {
                addTab(pNextWidget);
                hasVisibleElements = true;
            }
            else
            {
                removeTab(pNextWidget);
            }
        }
    }
    //-------------------------------//

    if(mRegisteredWidgets.size() > 0)
        showTabWidget(mLastOpenedTabForMode.value(mCurrentMode));

    update();

    return hasVisibleElements;
}


UBTabDockPalette::UBTabDockPalette(UBDockPalette *dockPalette, QWidget *parent) :
            QWidget(parent)
, dock(dockPalette)
, mVerticalOffset(0)
, mFlotable(false)
{
    int numTabs = dock->mTabWidgets.size();
    resize(2 * dock->border(), (numTabs * TABSIZE) + qMax(numTabs - 1, 0) * dock->tabSpacing());

    setAttribute(Qt::WA_TranslucentBackground);
}

void UBTabDockPalette::paintEvent(QPaintEvent *)
{
    int nTabs = dock->mTabWidgets.size();
    if (nTabs <= 0) {
        qDebug() << "not enough tabs";
        return;
    }

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(dock->mBackgroundBrush);

    int yFrom = 0;
    for (int i = 0; i < nTabs; i++) {
        UBDockPaletteWidget* pCrntWidget = dock->mTabWidgets.at(i);
        QPainterPath path;
        path.setFillRule(Qt::WindingFill);
        QPixmap iconPixmap;

        switch (dock->mOrientation) {
        case eUBDockOrientation_Left:
            path.addRect(0, yFrom, width() / 2, TABSIZE);
            path.addRoundedRect(0, yFrom, width(), TABSIZE, dock->radius(), dock->radius());
            if (pCrntWidget) {
                if(dock->mCollapseWidth >= dock->width()) {
                    // Get the collapsed icon
                    iconPixmap = pCrntWidget->iconToRight();
                } else {
                    // Get the expanded icon
                    iconPixmap = pCrntWidget->iconToLeft();
                }

            }
            break;

        case eUBDockOrientation_Right:
            path.addRect(width() /2, yFrom, width() / 2, TABSIZE);
            path.addRoundedRect(0, yFrom, width(), TABSIZE, dock->radius(), dock->radius());
            if (pCrntWidget) {
                if(dock->mCollapseWidth >= dock->width()) {
                    // Get the collapsed icon
                    iconPixmap = pCrntWidget->iconToLeft();
                } else {
                    // Get the expanded icon
                    iconPixmap = pCrntWidget->iconToRight();
                }
            }
            break;

        case eUBDockOrientation_Top: ;
        case eUBDockOrientation_Bottom: ;
        default:
            break;
        }

        painter.save();
        //QPixmap transparencyPix(":/images/tab_mask.png");
        if (dock->mCurrentTab != i) {
            //iconPixmap.setAlphaChannel(transparencyPix); // deprecated; removed for now, to be re-implemented
            QColor color(0x7F, 0x7F, 0x7F, 0x3F);
            painter.setBrush(QBrush(color));
        }

        painter.drawPath(path);
        painter.drawPixmap(2, yFrom + 2, width() - 4, TABSIZE - 4, iconPixmap);
        yFrom += (TABSIZE + dock->tabSpacing());
        painter.restore();
    }
}
UBTabDockPalette::~UBTabDockPalette()
{
}

void UBTabDockPalette::mousePressEvent(QMouseEvent *event)
{
    dock->mClickTime.start();
    // The goal here is to verify if the user can resize the widget.
    // It is only possible to resize it if the border is selected
    QPoint p = event->pos();
    dock->mMousePressPos = p;
    dock->mResized = false;

    switch(dock->mOrientation) {
    case eUBDockOrientation_Left:
        dock->mCanResize = true;
        break;
    case eUBDockOrientation_Right:
        dock->mCanResize = true;
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
void UBTabDockPalette::mouseMoveEvent(QMouseEvent *event)
{
    QPoint p = event->pos();

    if(dock->mCanResize && ((dock->mMousePressPos - p).manhattanLength() > QApplication::startDragDistance()))
    {
        if (qAbs(dock->mMousePressPos.y() - p.y()) > 10 && mFlotable) {
            qDebug() << "y differences" << dock->mMousePressPos.y() << p.y();
            mVerticalOffset += p.y() - dock->mMousePressPos.y();
            move(this->pos().x(),  p.y());
        }

        switch(dock->mOrientation) {

        case eUBDockOrientation_Left:
            p.setX(p.x() + dock->width());
            if(p.x() < dock->collapseWidth() && p.x() >= dock->minimumWidth()) {
                dock->update();
                dock->resize(0, dock->height());
                //dock->mLastWidth = dock->collapseWidth() + 1;
                dock->mResized = true;
            } else if (p.x() <= dock->maximumWidth() && p.x() >= dock->minimumWidth()) {
                dock->resize(p.x(), dock->height());
                dock->mResized = true;
            }
            break;

        case eUBDockOrientation_Right:
            p.setX(p.x() - 2 * dock->border());
            if((dock->x() + p.x() > dock->parentWidget()->width() - dock->collapseWidth()) && (dock->x() + p.x() < dock->parentWidget()->width())) {
                dock->update();
                dock->resize(0, dock->height());
                //dock->mLastWidth = dock->collapseWidth() + 1;
                dock->mResized = true;
            } else if((dock->x() + p.x() >= dock->parentWidget()->width() - dock->maximumWidth()) && (dock->x() + p.x() <= dock->parentWidget()->width() - dock->minimumWidth())) {
                dock->resize(dock->parentWidget()->width() - (dock->x() + p.x()), dock->height());
                dock->mResized = true;
            }
            break;

        case eUBDockOrientation_Top:
        case eUBDockOrientation_Bottom:
            if(p.y() <= dock->maximumHeight()) {
                dock->resize(width(), p.y());
                dock->mResized = true;
            }
            break;

        default:
            break;
        }
    }
}

void UBTabDockPalette::mouseReleaseEvent(QMouseEvent *event)
{
    Q_UNUSED(event);
    if(!dock->mResized && dock->mClickTime.elapsed() < CLICKTIME) {
        int nbTabs = dock->mTabWidgets.size();
        int clickedTab = 0;
        // If the clicked position is in the tab, perform the related action

        if(dock->mMousePressPos.x() >= 0 &&
                dock->mMousePressPos.x() <= width() &&
                dock->mMousePressPos.y() >= 0 &&
                dock->mMousePressPos.y() <= nbTabs * TABSIZE + (nbTabs -1)*dock->tabSpacing()) {

            clickedTab = (dock->mMousePressPos.y()) / (TABSIZE + dock->tabSpacing());
            dock->tabClicked(clickedTab);
        }
    }
    dock->mCanResize = false;
}
