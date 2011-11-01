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

#include "UBBoardPaletteManager.h"

#include "frameworks/UBPlatformUtils.h"
#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"
#include "core/UBSettings.h"
#include "core/UBSetting.h"
#include "core/UBDisplayManager.h"

#include "gui/UBMainWindow.h"
#include "gui/UBStylusPalette.h"
#include "gui/UBKeyboardPalette.h"
#include "gui/UBToolWidget.h"
#include "gui/UBZoomPalette.h"
#include "gui/UBActionPalette.h"
#include "gui/UBFavoriteToolPalette.h"


#include "web/UBWebPage.h"
#include "web/UBWebController.h"
#include "web/browser/WBBrowserWindow.h"
#include "web/browser/WBTabWidget.h"
#include "web/browser/WBWebView.h"

#include "desktop/UBDesktopAnnotationController.h"


#include "network/UBNetworkAccessManager.h"
#include "network/UBServerXMLHttpRequest.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBAbstractWidget.h"
#include "domain/UBGraphicsPixmapItem.h"

#include "document/UBDocumentProxy.h"
#include "podcast/UBPodcastController.h"
#include "board/UBDrawingController.h"

#include "tools/UBToolsManager.h"

#include "UBBoardController.h"

#include "core/memcheck.h"

UBBoardPaletteManager::UBBoardPaletteManager(QWidget* container, UBBoardController* pBoardController)
    : QObject(container)
    , mKeyboardPalette(0)
    , mContainer(container)
    , mBoardControler(pBoardController)
    , mStylusPalette(0)
    , mZoomPalette(0)
    , mLeftPalette(NULL)
    , mRightPalette(NULL)
    , mDesktopRightPalette(NULL)
    , mBackgroundsPalette(0)
    , mToolsPalette(0)
    , mAddItemPalette(0)
    , mErasePalette(NULL)
    , mPagePalette(NULL)
    , mPendingPageButtonPressed(false)
    , mPendingZoomButtonPressed(false)
    , mPendingPanButtonPressed(false)
    , mPendingEraseButtonPressed(false)
    , mpPageNavigWidget(NULL)
    , mpLibWidget(NULL)
    , mpCachePropWidget(NULL)
//    , mDesktopRightPalette(NULL)
    , mpTeacherBarWidget(NULL)
    , mpDesktopLibWidget(NULL)
{
    setupPalettes();
    connectPalettes();
}


UBBoardPaletteManager::~UBBoardPaletteManager()
{
    if(NULL != mpTeacherBarWidget)
    {
        delete mpTeacherBarWidget;
        mpTeacherBarWidget = NULL;
    }
    if(NULL != mpPageNavigWidget)
    {
        delete mpPageNavigWidget;
        mpPageNavigWidget = NULL;
    }
    if(NULL != mpLibWidget)
    {
        delete mpLibWidget;
        mpLibWidget = NULL;
    }
    if(NULL != mpTeacherBarWidget)
    {
        delete mpTeacherBarWidget;
        mpTeacherBarWidget = NULL;
    }
    if(NULL != mpCachePropWidget)
    {
        delete mpCachePropWidget;
        mpCachePropWidget = NULL;
    }
    delete mAddItemPalette;
    if(NULL != mLeftPalette)
    {
        delete mLeftPalette;
        mLeftPalette = NULL;
    }

    if(NULL != mRightPalette)
    {
        delete mRightPalette;
        mRightPalette = NULL;
    }

    if(NULL != mStylusPalette)
    {
        delete mStylusPalette;
        mStylusPalette = NULL;
    }

    if(NULL != mpDesktopLibWidget)
    {
        delete mpDesktopLibWidget;
        mpDesktopLibWidget = NULL;
    }
//     if(NULL != mDesktopRightPalette)
//     {
//         delete mDesktopRightPalette;
//         mDesktopRightPalette = NULL;
//     }
}

void UBBoardPaletteManager::initPalettesPosAtStartup()
{
    mStylusPalette->initPosition();
}

void UBBoardPaletteManager::setupLayout()
{

}

/**
 * \brief Set up the dock palette widgets
 */
void UBBoardPaletteManager::setupDockPaletteWidgets()
{

    //------------------------------------------------//
    // Create the widgets for the dock palettes

    mpPageNavigWidget = new UBPageNavigationWidget();
    mpPageNavigWidget->registerMode(eUBDockPaletteWidget_BOARD);
//    connect(this, SIGNAL(signal_changeMode(eUBDockPaletteWidgetMode)), mpPageNavigWidget, SLOT(slot_changeMode(eUBDockPaletteWidgetMode)));

    mpLibWidget = new UBLibWidget();
    mpLibWidget ->registerMode(eUBDockPaletteWidget_BOARD);
    mpLibWidget ->registerMode(eUBDockPaletteWidget_DESKTOP);
//    connect(this, SIGNAL(signal_changeMode(eUBDockPaletteWidgetMode)), mpLibWidget, SLOT(slot_changeMode(eUBDockPaletteWidgetMode)));

    mpCachePropWidget = new UBCachePropertiesWidget();
    mpCachePropWidget->registerMode(eUBDockPaletteWidget_BOARD);
//    connect(this, SIGNAL(signal_changeMode(eUBDockPaletteWidgetMode)), mpCachePropWidget, SLOT(slot_changeMode(eUBDockPaletteWidgetMode)));

    mpTeacherBarWidget = new UBTeacherBarWidget();
    mpTeacherBarWidget->registerMode(eUBDockPaletteWidget_BOARD);
//    connect(this, SIGNAL(signal_changeMode(eUBDockPaletteWidgetMode)), mpTeacherBarWidget, SLOT(slot_changeMode(eUBDockPaletteWidgetMode)));

    //------------------------------------------------//
    // Add the dock palettes
    mLeftPalette = new UBLeftPalette(mContainer);

    // LEFT palette widgets
    mLeftPalette->registerWidget(mpPageNavigWidget);
    mLeftPalette->addTab(mpPageNavigWidget);

    mLeftPalette->connectSignals();

    //------------------------------------------------//

    mRightPalette = new UBRightPalette(mContainer);

    // RIGHT palette widgets
    mRightPalette->registerWidget(mpLibWidget);
    mRightPalette->addTab(mpLibWidget);

    // ???
    mRightPalette->registerWidget(mpCachePropWidget); 
//    mRightPalette->addTab(mpCachePropWidget);

    // ???
    mRightPalette->registerWidget(mpTeacherBarWidget);
    mRightPalette->addTab(mpTeacherBarWidget);

    mRightPalette->connectSignals();

    //------------------------------------------------//

    changeMode(eUBDockPaletteWidget_BOARD, true);

    //------------------------------------------------//

//     mLeftPalette->showTabWidget(0);
//     mRightPalette->showTabWidget(0);
// 
//     //------------------------------------------------//
}

void UBBoardPaletteManager::slot_changeMainMode(UBApplicationController::MainMode mainMode)
{
//    Board = 0, Internet, Document, Tutorial, ParaschoolEditor, WebDocument

    switch( mainMode )
    {
        case UBApplicationController::Board: 
            {
                // call changeMode only when switch NOT from desktop mode
                if(!UBApplication::applicationController->isShowingDesktop())
                    changeMode(eUBDockPaletteWidget_BOARD);
            }
            break;

        case UBApplicationController::Tutorial:
            {
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != NULL)
                    mKeyboardPalette->hide();
            }
            break;

        case UBApplicationController::Internet:
            changeMode(eUBDockPaletteWidget_WEB);
            break;

        case UBApplicationController::Document:
            changeMode(eUBDockPaletteWidget_DOCUMENT);
            break;

        default:
            {
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != NULL)
                    mKeyboardPalette->hide();
            }
            break;
    }
}

void UBBoardPaletteManager::slot_changeDesktopMode(bool isDesktop)
{
    UBApplicationController::MainMode currMode = UBApplication::applicationController->displayMode();
    if(!isDesktop) 
    {
        switch( currMode )
        {
            case UBApplicationController::Board:
                changeMode(eUBDockPaletteWidget_BOARD);
                break;

            default:
                break;
        }
    }
    else
        changeMode(eUBDockPaletteWidget_DESKTOP);
}

void UBBoardPaletteManager::setupPalettes()
{

    if (UBPlatformUtils::hasVirtualKeyboard())
    {
        mKeyboardPalette = new UBKeyboardPalette(0);
#ifndef Q_WS_WIN
        connect(mKeyboardPalette, SIGNAL(closed()), mKeyboardPalette, SLOT(onDeactivated()));
#endif
#ifndef Q_WS_MAC
        //     mKeyboardPalette->setParent(mContainer);
#endif
    }

    setupDockPaletteWidgets();


    // Add the other palettes
    mStylusPalette = new UBStylusPalette(mContainer, UBSettings::settings()->appToolBarOrientationVertical->get().toBool() ? Qt::Vertical : Qt::Horizontal);
    connect(mStylusPalette, SIGNAL(stylusToolDoubleClicked(int)), UBApplication::boardController, SLOT(stylusToolDoubleClicked(int)));
    mStylusPalette->show(); // always show stylus palette at startup

    mZoomPalette = new UBZoomPalette(mContainer);

    QList<QAction*> backgroundsActions;

    backgroundsActions << UBApplication::mainWindow->actionPlainLightBackground;
    backgroundsActions << UBApplication::mainWindow->actionCrossedLightBackground;
    backgroundsActions << UBApplication::mainWindow->actionPlainDarkBackground;
    backgroundsActions << UBApplication::mainWindow->actionCrossedDarkBackground;

    mBackgroundsPalette = new UBActionPalette(backgroundsActions, Qt::Horizontal , mContainer);
    mBackgroundsPalette->setButtonIconSize(QSize(128, 128));
    mBackgroundsPalette->groupActions();
    mBackgroundsPalette->setClosable(true);
    mBackgroundsPalette->setAutoClose(true);
    mBackgroundsPalette->adjustSizeAndPosition();
    mBackgroundsPalette->hide();

    QList<QAction*> addItemActions;

    addItemActions << UBApplication::mainWindow->actionAddItemToCurrentPage;
    addItemActions << UBApplication::mainWindow->actionAddItemToNewPage;
    addItemActions << UBApplication::mainWindow->actionAddItemToLibrary;

    mAddItemPalette = new UBActionPalette(addItemActions, Qt::Horizontal, 0);
    mAddItemPalette->setButtonIconSize(QSize(128, 128));
    mAddItemPalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mAddItemPalette->groupActions();
    mAddItemPalette->setClosable(true);
    mAddItemPalette->adjustSizeAndPosition();
    mAddItemPalette->hide();

    QList<QAction*> eraseActions;

    eraseActions << UBApplication::mainWindow->actionEraseAnnotations;
    eraseActions << UBApplication::mainWindow->actionEraseItems;
    eraseActions << UBApplication::mainWindow->actionClearPage;

    mErasePalette = new UBActionPalette(eraseActions, Qt::Horizontal , mContainer);
    mErasePalette->setButtonIconSize(QSize(128, 128));
    mErasePalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mErasePalette->groupActions();
    mErasePalette->setClosable(true);
    mErasePalette->adjustSizeAndPosition();
    mErasePalette->hide();

    QList<QAction*> pageActions;

    pageActions << UBApplication::mainWindow->actionNewPage;
    pageActions << UBApplication::mainWindow->actionDuplicatePage;
    pageActions << UBApplication::mainWindow->actionImportPage;

    mPagePalette = new UBActionPalette(pageActions, Qt::Horizontal , mContainer);
    mPagePalette->setButtonIconSize(QSize(128, 128));
    mPagePalette->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    mPagePalette->groupActions();
    mPagePalette->setClosable(true);
    mPagePalette->adjustSizeAndPosition();
    mPagePalette->hide();

    connect(UBSettings::settings()->appToolBarOrientationVertical, SIGNAL(changed(QVariant)), this, SLOT(changeStylusPaletteOrientation(QVariant)));
}

void UBBoardPaletteManager::pagePaletteButtonPressed()
{
    mPageButtonPressedTime = QTime::currentTime();

    mPendingPageButtonPressed = true;
    QTimer::singleShot(1000, this, SLOT(pagePaletteButtonReleased()));
}


void UBBoardPaletteManager::pagePaletteButtonReleased()
{
    if (mPendingPageButtonPressed)
    {
        if( mPageButtonPressedTime.msecsTo(QTime::currentTime()) > 900)
        {
            togglePagePalette(true);
        }
        else
        {
            UBApplication::mainWindow->actionNewPage->trigger();
        }

        mPendingPageButtonPressed = false;
    }
}

void UBBoardPaletteManager::erasePaletteButtonPressed()
{
    mEraseButtonPressedTime = QTime::currentTime();

    mPendingEraseButtonPressed = true;
    QTimer::singleShot(1000, this, SLOT(erasePaletteButtonReleased()));
}


void UBBoardPaletteManager::erasePaletteButtonReleased()
{
    if (mPendingEraseButtonPressed)
    {
        if( mEraseButtonPressedTime.msecsTo(QTime::currentTime()) > 900)
        {
            toggleErasePalette(true);
        }
        else
        {
            UBApplication::mainWindow->actionClearPage->trigger();
        }

        mPendingEraseButtonPressed = false;
    }
}



void UBBoardPaletteManager::linkClicked(const QUrl& url)
{
      UBApplication::applicationController->showInternet();
      UBApplication::webController->loadUrl(url);
}


void UBBoardPaletteManager::purchaseLinkActivated(const QString& link)
{
    UBApplication::applicationController->showInternet();
    UBApplication::webController->loadUrl(QUrl(link));
}

void UBBoardPaletteManager::connectPalettes()
{
    connect(UBApplication::mainWindow->actionStylus, SIGNAL(toggled(bool)), this, SLOT(toggleStylusPalette(bool)));

    foreach(QWidget *widget, UBApplication::mainWindow->actionZoomIn->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(zoomButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(zoomButtonReleased()));
        }
    }

    foreach(QWidget *widget, UBApplication::mainWindow->actionZoomOut->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(zoomButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(zoomButtonReleased()));
        }
    }

    foreach(QWidget *widget, UBApplication::mainWindow->actionHand->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(panButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(panButtonReleased()));
        }
    }

    connect(UBApplication::mainWindow->actionBackgrounds, SIGNAL(toggled(bool)), this, SLOT(toggleBackgroundPalette(bool)));
    connect(mBackgroundsPalette, SIGNAL(closed()), this, SLOT(backgroundPaletteClosed()));

    connect(UBApplication::mainWindow->actionPlainLightBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionCrossedLightBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionPlainDarkBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionCrossedDarkBackground, SIGNAL(triggered()), this, SLOT(changeBackground()));
    connect(UBApplication::mainWindow->actionPodcast, SIGNAL(triggered(bool)), this, SLOT(tooglePodcastPalette(bool)));

    connect(UBApplication::mainWindow->actionAddItemToCurrentPage, SIGNAL(triggered()), this, SLOT(addItemToCurrentPage()));
    connect(UBApplication::mainWindow->actionAddItemToNewPage, SIGNAL(triggered()), this, SLOT(addItemToNewPage()));
    connect(UBApplication::mainWindow->actionAddItemToLibrary, SIGNAL(triggered()), this, SLOT(addItemToLibrary()));

    connect(UBApplication::mainWindow->actionEraseItems, SIGNAL(triggered()), mErasePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionEraseAnnotations, SIGNAL(triggered()), mErasePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionClearPage, SIGNAL(triggered()), mErasePalette, SLOT(close()));
    connect(mErasePalette, SIGNAL(closed()), this, SLOT(erasePaletteClosed()));

    foreach(QWidget *widget, UBApplication::mainWindow->actionErase->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(erasePaletteButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(erasePaletteButtonReleased()));
        }
    }

    connect(UBApplication::mainWindow->actionNewPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionDuplicatePage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
    connect(UBApplication::mainWindow->actionImportPage, SIGNAL(triggered()), mPagePalette, SLOT(close()));
    connect(mPagePalette, SIGNAL(closed()), this, SLOT(pagePaletteClosed()));

    foreach(QWidget *widget, UBApplication::mainWindow->actionPages->associatedWidgets())
    {
        QAbstractButton *button = qobject_cast<QAbstractButton*>(widget);
        if (button)
        {
            connect(button, SIGNAL(pressed()), this, SLOT(pagePaletteButtonPressed()));
            connect(button, SIGNAL(released()), this, SLOT(pagePaletteButtonReleased()));
        }
    }

}


bool isFirstResized = true;
void UBBoardPaletteManager::containerResized()
{
    int innerMargin = UBSettings::boardMargin;

    int userLeft = innerMargin;
    int userWidth = mContainer->width() - (2 * innerMargin);
    int userTop = innerMargin;
    int userHeight = mContainer->height() - (2 * innerMargin);

    if(mStylusPalette)
    {
        mStylusPalette->move(userLeft, userTop);
        mStylusPalette->adjustSizeAndPosition();
        mStylusPalette->initPosition();
    }

    if(mZoomPalette)
    {
        mZoomPalette->move(userLeft + userWidth - mZoomPalette->width()
                , userTop + userHeight /*- mPageNumberPalette->height()*/ - innerMargin - mZoomPalette->height());
        mZoomPalette->adjustSizeAndPosition();
    }

    if (isFirstResized && mKeyboardPalette && mKeyboardPalette->parent() == UBApplication::boardController->controlContainer())
    {
        isFirstResized = false;
        mKeyboardPalette->move(userLeft + (userWidth - mKeyboardPalette->width())/2,
                               userTop + (userHeight - mKeyboardPalette->height())/2);
        mKeyboardPalette->adjustSizeAndPosition();
    }

    if(mLeftPalette)
    {
        mLeftPalette->resize(mLeftPalette->width()-1, mContainer->height());
        mLeftPalette->resize(mLeftPalette->width(), mContainer->height());
    }

    if(mRightPalette)
    {
        mRightPalette->resize(mRightPalette->width()-1, mContainer->height());
        mRightPalette->resize(mRightPalette->width(), mContainer->height());
    }
}


void UBBoardPaletteManager::changeBackground()
{
    if (UBApplication::mainWindow->actionCrossedLightBackground->isChecked())
        UBApplication::boardController->changeBackground(false, true);
    else if (UBApplication::mainWindow->actionPlainDarkBackground->isChecked())
        UBApplication::boardController->changeBackground(true, false);
    else if (UBApplication::mainWindow->actionCrossedDarkBackground->isChecked())
        UBApplication::boardController->changeBackground(true, true);
    else
        UBApplication::boardController->changeBackground(false, false);

    UBApplication::mainWindow->actionBackgrounds->setChecked(false);
}


void UBBoardPaletteManager::activeSceneChanged()
{
    UBGraphicsScene *activeScene =  UBApplication::boardController->activeScene();
    int pageIndex = UBApplication::boardController->activeSceneIndex();

    if (mStylusPalette)
        connect(mStylusPalette, SIGNAL(mouseEntered()), activeScene, SLOT(hideEraser()));

    if (mpPageNavigWidget)
    {
        mpPageNavigWidget->setPageNumber(pageIndex + 1, activeScene->document()->pageCount());
    }

    if (mZoomPalette)
        connect(mZoomPalette, SIGNAL(mouseEntered()), activeScene, SLOT(hideEraser()));

    if (mBackgroundsPalette)
        connect(mBackgroundsPalette, SIGNAL(mouseEntered()), activeScene, SLOT(hideEraser()));
}


void UBBoardPaletteManager::toggleBackgroundPalette(bool checked)
{
    mBackgroundsPalette->setVisible(checked);

    if (checked)
    {
        UBApplication::mainWindow->actionErase->setChecked(false);
        UBApplication::mainWindow->actionNewPage->setChecked(false);

        mBackgroundsPalette->adjustSizeAndPosition();
        mBackgroundsPalette->move((mContainer->width() - mBackgroundsPalette->width()) / 2,
            (mContainer->height() - mBackgroundsPalette->height()) / 5);
    }
}


void UBBoardPaletteManager::backgroundPaletteClosed()
{
    UBApplication::mainWindow->actionBackgrounds->setChecked(false);
}


void UBBoardPaletteManager::toggleStylusPalette(bool checked)
{
    mStylusPalette->setVisible(checked);
}


void UBBoardPaletteManager::toggleErasePalette(bool checked)
{
    mErasePalette->setVisible(checked);
    if (checked)
    {
        UBApplication::mainWindow->actionBackgrounds->setChecked(false);
        UBApplication::mainWindow->actionNewPage->setChecked(false);

        mErasePalette->adjustSizeAndPosition();
        mErasePalette->move((mContainer->width() - mErasePalette->width()) / 2,
            (mContainer->height() - mErasePalette->height()) / 5);
    }
}


void UBBoardPaletteManager::erasePaletteClosed()
{
    UBApplication::mainWindow->actionErase->setChecked(false);
}


void UBBoardPaletteManager::togglePagePalette(bool checked)
{
    mPagePalette->setVisible(checked);
    if (checked)
    {
        UBApplication::mainWindow->actionBackgrounds->setChecked(false);
        UBApplication::mainWindow->actionErase->setChecked(false);

        mPagePalette->adjustSizeAndPosition();
        mPagePalette->move((mContainer->width() - mPagePalette->width()) / 2,
            (mContainer->height() - mPagePalette->height()) / 5);
    }
}


void UBBoardPaletteManager::pagePaletteClosed()
{
    UBApplication::mainWindow->actionPages->setChecked(false);
}


void UBBoardPaletteManager::tooglePodcastPalette(bool checked)
{
    UBPodcastController::instance()->toggleRecordingPalette(checked);
}


void UBBoardPaletteManager::addItem(const QUrl& pUrl)
{
    mItemUrl = pUrl;
    mPixmap = QPixmap();
    mPos = QPointF(0, 0);
    mScaleFactor = 1.;

    mAddItemPalette->show();
    mAddItemPalette->adjustSizeAndPosition();

    QRect controlGeo = UBApplication::applicationController->displayManager()->controlGeometry();

    mAddItemPalette->move(controlGeo.x() + ((controlGeo.width() - mAddItemPalette->geometry().width()) / 2),
          (controlGeo.y() + (controlGeo.height() - mAddItemPalette->geometry().height()) / 5));

}

void UBBoardPaletteManager::processPalettersWidget(UBDockPalette *paletter, eUBDockPaletteWidgetMode mode)
{
    //-------------------------------//
    // get full right palette widgets list, parse it, show all widgets for BOARD mode, and hide all other
    QVector<UBDockPaletteWidget*> widgetsList = paletter->GetWidgetsList();
    for(int i = 0; i < widgetsList.size(); i++)
    {
        UBDockPaletteWidget* pNextWidget = widgetsList.at(i);
        if( pNextWidget != NULL )
        {
            if( pNextWidget->GetRegisteredModes().contains(mode) ) 
            {
                paletter->addTab(pNextWidget);
            }
            else
            {
                paletter->removeTab(pNextWidget->name());
            }
        }
    }
    //-------------------------------//

    if(widgetsList.size() > 0)
        paletter->showTabWidget(0);
    
    paletter->update();
}

void UBBoardPaletteManager::changeMode(eUBDockPaletteWidgetMode newMode, bool isInit)
{
    processPalettersWidget(mRightPalette, newMode);
    processPalettersWidget(mLeftPalette, newMode);

    switch( newMode )
    {
        case eUBDockPaletteWidget_BOARD:
            {
                mLeftPalette->setParent(UBApplication::boardController->controlContainer());
                mRightPalette->setParent(UBApplication::boardController->controlContainer());
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != NULL)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(UBApplication::boardController->controlContainer());
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(UBApplication::boardController->controlContainer());
                }

                mLeftPalette->setVisible(true);
                mRightPalette->setVisible(true);

                if( !isInit )
                    containerResized();
            }
            break;

        case eUBDockPaletteWidget_DESKTOP:
            {
                mLeftPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                mRightPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != NULL)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
#ifndef Q_WS_X11
                        mKeyboardPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
#else
                        mKeyboardPalette->setParent(0);
#endif
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent((QWidget*)UBApplication::applicationController->uninotesController()->drawingView());
                }

                mLeftPalette->setVisible(false);
                mRightPalette->setVisible(true);

                if( !isInit )
                    UBApplication::applicationController->uninotesController()->TransparentWidgetResized();
            }
            break;

        case eUBDockPaletteWidget_WEB:
            {
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != NULL)
                {
                    WBBrowserWindow* brWnd = UBApplication::webController->GetCurrentWebBrowser();

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(brWnd);
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(brWnd);
                }

            }
            break;

        default:
            {
                mLeftPalette->setVisible(false);
                mRightPalette->setVisible(false);
                mLeftPalette->setParent(0);
                mRightPalette->setParent(0);
                if (UBPlatformUtils::hasVirtualKeyboard() && mKeyboardPalette != NULL)
                {

                    if(mKeyboardPalette->m_isVisible)
                    {
                        mKeyboardPalette->hide();
                        mKeyboardPalette->setParent(0);
                        mKeyboardPalette->show();
                    }
                    else
                        mKeyboardPalette->setParent(0);

//                    mKeyboardPalette->update();

                }
            }
            break;
    }

    if( !isInit )
        UBApplication::boardController->notifyPageChanged();

    emit signal_changeMode(newMode);
}

/*
void UBBoardPaletteManager::slot_changeMode(eUBDockPaletteWidgetMode newMode)
{
    emit signal_changeMode(newMode);
}
*/

void UBBoardPaletteManager::addItem(const QPixmap& pPixmap, const QPointF& pos,  qreal scaleFactor, const QUrl& sourceUrl)
{
    mItemUrl = sourceUrl;
    mPixmap = pPixmap;
    mPos = pos;
    mScaleFactor = scaleFactor;

     QRect controlGeo = UBApplication::applicationController->displayManager()->controlGeometry();

    mAddItemPalette->show();
    mAddItemPalette->adjustSizeAndPosition();

    mAddItemPalette->move(controlGeo.x() + ((controlGeo.width() - mAddItemPalette->geometry().width()) / 2),
          (controlGeo.y() + (controlGeo.height() - mAddItemPalette->geometry().height()) / 5));
}


void UBBoardPaletteManager::addItemToCurrentPage()
{
    UBApplication::applicationController->showBoard();
    mAddItemPalette->hide();
    if(mPixmap.isNull())
        UBApplication::boardController->downloadURL(mItemUrl);
    else
    {
        UBGraphicsPixmapItem* item = UBApplication::boardController->activeScene()->addPixmap(mPixmap, mPos, mScaleFactor);

        item->setSourceUrl(mItemUrl);
        item->setSelected(true);

        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    }
}


void UBBoardPaletteManager::addItemToNewPage()
{
    UBApplication::boardController->addScene();
    addItemToCurrentPage();
}


void UBBoardPaletteManager::addItemToLibrary()
{
    if(mPixmap.isNull())
    {
       mPixmap = QPixmap(mItemUrl.toLocalFile());
    }

    if(!mPixmap.isNull())
    {
        QString imageDir = UBSettings::settings()->defaultUserImagesDirectory();

        if(mScaleFactor != 1.)
        {
             mPixmap = mPixmap.scaled(mScaleFactor * mPixmap.width(), mScaleFactor* mPixmap.height()
                     , Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        QImage image = mPixmap.toImage();

        if(NULL != mpLibWidget)
        {
            mpLibWidget->libNavigator()->libraryWidget()->libraryController()->importImageOnLibrary(image);
        }
    }
    else
    {
        UBApplication::showMessage(tr("Error Adding Image to Library"));
    }

    mAddItemPalette->hide();
}

void UBBoardPaletteManager::zoomButtonPressed()
{
    mZoomButtonPressedTime = QTime::currentTime();

    mPendingZoomButtonPressed = true;
    QTimer::singleShot(1000, this, SLOT(zoomButtonReleased()));
}


void UBBoardPaletteManager::zoomButtonReleased()
{
    if (mPendingZoomButtonPressed)
    {
        if(mZoomButtonPressedTime.msecsTo(QTime::currentTime()) > 900)
        {
            mBoardControler->zoomRestore();
        }

        mPendingZoomButtonPressed = false;
    }
}

void UBBoardPaletteManager::panButtonPressed()
{
    mPanButtonPressedTime = QTime::currentTime();

    mPendingPanButtonPressed = true;
    QTimer::singleShot(1000, this, SLOT(panButtonReleased()));
}


void UBBoardPaletteManager::panButtonReleased()
{
    if (mPendingPanButtonPressed)
    {
        if(mPanButtonPressedTime.msecsTo(QTime::currentTime()) > 900)
        {
            mBoardControler->centerRestore();
        }

        mPendingPanButtonPressed = false;
    }
}

void UBBoardPaletteManager::showVirtualKeyboard(bool show)
{
    if (mKeyboardPalette)
        mKeyboardPalette->setVisible(show);
}

void UBBoardPaletteManager::changeStylusPaletteOrientation(QVariant var)
{
    bool bVertical = var.toBool();
    bool bVisible = mStylusPalette->isVisible();

	// Clean the old palette
    if(NULL != mStylusPalette)
    {
        // TODO : check why this line creates a crash in the application.
        delete mStylusPalette;
    }

    // Create the new palette
    if(bVertical)
    {
        mStylusPalette = new UBStylusPalette(mContainer, Qt::Vertical);
    }
    else
    {
        mStylusPalette = new UBStylusPalette(mContainer, Qt::Horizontal);
    }

    connect(mStylusPalette, SIGNAL(stylusToolDoubleClicked(int)), UBApplication::boardController, SLOT(stylusToolDoubleClicked(int)));
    mStylusPalette->setVisible(bVisible); // always show stylus palette at startup
}

/*
UBRightPalette* UBBoardPaletteManager::createDesktopRightPalette(QWidget* parent)
{
    mpDesktopLibWidget = new UBLibWidget();
    mDesktopRightPalette = new UBRightPalette(parent);
    mDesktopRightPalette->registerWidget(mpDesktopLibWidget);
    mDesktopRightPalette->addTabWidget(mpDesktopLibWidget);
    mDesktopRightPalette->connectSignals();

    return mDesktopRightPalette;
}
*/

void UBBoardPaletteManager::connectToDocumentController()
{
    emit connectToDocController();
}

void UBBoardPaletteManager::refreshPalettes()
{
    mRightPalette->update();
    mLeftPalette->update();
}
