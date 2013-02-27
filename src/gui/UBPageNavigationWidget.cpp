/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBPageNavigationWidget.h"
#include "core/UBApplication.h"

#include "board/UBBoardController.h"

#include "document/UBDocumentContainer.h"

#include "globals/UBGlobals.h"

#include "core/memcheck.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBPageNavigationWidget::UBPageNavigationWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
  , mNavigator(NULL)
  , mLayout(NULL)
  , mHLayout(NULL)
  , mPageNbr(NULL)
  , mClock(NULL)
{
    setObjectName(name);
    mName = "PageNavigator";
    mVisibleState = true;

    SET_STYLE_SHEET();

    mIconToRight = QPixmap(":images/pages_open.png");
    mIconToLeft = QPixmap(":images/pages_close.png");

    // Build the gui
    mLayout = new QVBoxLayout(this);
    setLayout(mLayout);

    mNavigator = new UBDocumentNavigator(this);
    mLayout->addWidget(mNavigator, 1);

    mHLayout = new QHBoxLayout();
    mLayout->addLayout(mHLayout, 0);

    mPageNbr = new QLabel(this);
    mClock = new QLabel(this);
    mHLayout->addWidget(mPageNbr);
    mHLayout->addWidget(mClock);

    // Configure the page number indicator
    mPageNbr->setStyleSheet(QString("QLabel { color: white; background-color: transparent; border: none; font-family: Arial; font-weight: bold; font-size: 20px }"));
    setPageNumber(0, 0);
    mPageNbr->setAlignment(Qt::AlignHCenter);

    // Configure the clock
    mClock->setStyleSheet(QString("QLabel {color: white; background-color: transparent; text-align: center; font-family: Arial; font-weight: bold; font-size: 20px}"));
    mTimeFormat = QLocale::system().timeFormat(QLocale::ShortFormat);
    mClock->setAlignment(Qt::AlignHCenter);

    //strip seconds
    mTimeFormat = mTimeFormat.remove(":ss");
    mTimeFormat = mTimeFormat.remove(":s");
    mTimerID = startTimer(1000);

}

/**
 * \brief Destructor
 */
UBPageNavigationWidget::~UBPageNavigationWidget()
{
    killTimer(mTimerID);

    if(NULL != mClock)
    {
        delete mClock;
        mClock = NULL;
    }
    if(NULL != mPageNbr)
    {
        delete mPageNbr;
        mPageNbr = NULL;
    }
    if(NULL != mHLayout)
    {
        delete mHLayout;
        mHLayout = NULL;
    }
    if(NULL != mLayout)
    {
        delete mLayout;
        mLayout = NULL;
    }
    if(NULL != mNavigator)
    {
        delete mNavigator;
        mNavigator = NULL;
    }
}


/**
 * \brief Refresh the thumbnails widget
 */
void UBPageNavigationWidget::refresh()
{
    // TOLIK!!!
    // mNavigator->setDocument(UBApplication::boardController->activeDocument());
}

/**
 * \brief Notify a timer event
 * @param event as the timer event
 */
void UBPageNavigationWidget::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    updateTime();
}

/**
 * \brief Update the current time
 */
void UBPageNavigationWidget::updateTime()
{
    if (mClock)
    {
        mClock->setText(QLocale::system().toString (QTime::currentTime(), mTimeFormat));
    }
}

/**
 * \brief Set the page number
 * @param current as the current page
 * @param total as the total number of pages
 */
void UBPageNavigationWidget::setPageNumber(int current, int total)
{
    mPageNbr->setText(QString("%1 / %2").arg(current).arg(UBDocumentContainer::sceneIndexFromPage(total)));
}

/**
 * \brief Get the custom margin value
 * @return the custom margin value
 */
int UBPageNavigationWidget::customMargin()
{
    return 5;
}

/**
 * \brief Get the border value
 * @return the border value
 */
int UBPageNavigationWidget::border()
{
    return 15;
}

