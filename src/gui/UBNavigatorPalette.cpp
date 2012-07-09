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
#include "UBNavigatorPalette.h"
#include "core/UBApplication.h"
#include "board/UBBoardController.h"

#include "core/memcheck.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBNavigatorPalette::UBNavigatorPalette(QWidget *parent, const char *name):
	UBDockPalette(eUBDockPaletteType_LEFT, parent, name)
	, mNavigator(NULL)
	, mLayout(NULL)
    , mHLayout(NULL)
    , mPageNbr(NULL)
    , mClock(NULL)
{
    // Build the gui
    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(customMargin(), customMargin(), 2*border() + customMargin(), customMargin());
    setLayout(mLayout);

    mNavigator = new UBDocumentNavigator(this);
    mNavigator->setStyleSheet(QString("background-color : transparent;"));
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

    connect(mNavigator, SIGNAL(changeCurrentPage()), this, SLOT(changeCurrentPage()));  
}

/**
 * \brief Destructor
 */
UBNavigatorPalette::~UBNavigatorPalette()
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
 * \brief Set the current document in the navigator
 * @param document as the given document
 */
void UBNavigatorPalette::setDocument(UBDocumentProxy *document)
{
    if(mNavigator->currentDoc() != document)
    {
	mNavigator->setDocument(document);
    }
}

/**
 * \brief Change the current page
 */
void UBNavigatorPalette::changeCurrentPage()
{
    //	Get the index of the page to display
    int iPage = mNavigator->selectedPageNumber();
    if(NO_PAGESELECTED != iPage)
    {
	// Display the selected page
    UBApplication::boardController->setActiveDocumentScene(mNavigator->currentDoc(), iPage);
    }
}

/**
 * \brief Refresh the thumbnails widget
 */
void UBNavigatorPalette::refresh()
{
    mNavigator->setDocument(UBApplication::boardController->activeDocument());
}

/**
 * \brief Handle the resize event
 * @param event as the resize event
 */
void UBNavigatorPalette::resizeEvent(QResizeEvent *event)
{
    UBDockPalette::resizeEvent(event);
    if(NULL != mNavigator)
    {
        mNavigator->setMinimumHeight(height() - 2*border());
    }
}

void UBNavigatorPalette::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event);
    updateTime();
}

void UBNavigatorPalette::updateTime()
{
    if (mClock)
    {
        mClock->setText(QLocale::system().toString (QTime::currentTime(), mTimeFormat));
    }
}

void UBNavigatorPalette::setPageNumber(int current, int total)
{
    mPageNbr->setText(QString("%1 / %2").arg(current).arg(total));
}
