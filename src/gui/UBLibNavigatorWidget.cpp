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
#include "UBLibNavigatorWidget.h"
#include "UBLibWidget.h"

#include "core/UBApplication.h"
#include "core/memcheck.h"

static int lowBoundForSlider = 40;
static int topBoundForSlider = 120;
static int tickIntervalForSlider = 10;

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBLibNavigatorWidget::UBLibNavigatorWidget(QWidget *parent, const char *name):QWidget(parent)
  , mLayout(NULL)
  , mLibWidget(NULL)
  , mSlider(NULL)
  , mSliderWidthSetting(NULL)
{
    setObjectName(name);

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    setAcceptDrops(true);

    UBLibWidget* libWidget = dynamic_cast<UBLibWidget*>(parentWidget());
    mLayout = new QVBoxLayout(this);
    setLayout(mLayout);

    mLibWidget = new UBLibraryWidget(this);
    mLayout->addWidget(mLibWidget, 1);

    mSlider = new QSlider(Qt::Horizontal, this);
    mSlider->setMinimumHeight(20);
    mSlider->setRange(lowBoundForSlider, topBoundForSlider);
    mSliderWidthSetting = new UBSetting(UBSettings::settings(), "Library", "LibWidgetWidth", topBoundForSlider);
    int defaultWidth = mSliderWidthSetting->get().toInt();
    mSlider->setValue(defaultWidth);
    mSlider->setTickInterval(tickIntervalForSlider);
    mLayout->addWidget(mSlider, 0);

    connect(mLibWidget, SIGNAL(navigBarUpdate(UBLibElement*)), this, SLOT(onNavigbarUpate(UBLibElement*)));
    connect(this, SIGNAL(updateNavigBar(UBChainedLibElement*)), libWidget, SLOT(onUpdateNavigBar(UBChainedLibElement*)));
    mLibWidget->updateThumbnailsSize(defaultWidth);


    connect(mLibWidget, SIGNAL(propertiesRequested(UBLibElement*)), this, SLOT(onPropertiesRequested(UBLibElement*)));
    connect(mLibWidget, SIGNAL(displaySearchEngine(UBLibElement*)), this, SLOT(onDisplaySearchEngine(UBLibElement*)));    
    connect(mSlider,SIGNAL(valueChanged(int)),this,SLOT(updateThumbnailsSize(int)));
    connect(libWidget->pathViewer(), SIGNAL(mouseClick(UBChainedLibElement*)), this, SLOT(onPathItemClicked(UBChainedLibElement*)));
    connect(libWidget->pathViewer(), SIGNAL(elementsDropped(QList<QString>,UBLibElement*)), mLibWidget, SLOT(onElementsDropped(QList<QString>,UBLibElement*)));
    connect(mLibWidget, SIGNAL(navigBarUpdate(UBLibElement*)), libWidget->actionBar(), SLOT(onNavigbarUpdate(UBLibElement*)));
    connect(mLibWidget, SIGNAL(itemsSelected(QList<UBLibElement*>, bool)), libWidget->actionBar(), SLOT(onSelectionChanged(QList<UBLibElement*>, bool)));
    connect(libWidget->actionBar(), SIGNAL(deleteDone()), mLibWidget, SLOT(onRefreshCurrentFolder()));
    connect(mLibWidget, SIGNAL(favoritesEntered(bool)), libWidget->actionBar(), SLOT(onFavoritesEntered(bool)));
    connect(libWidget->actionBar(), SIGNAL(searchElement(QString)), mLibWidget, SLOT(onSearchElement(QString)));
    connect(libWidget->actionBar(), SIGNAL(newFolderToCreate()), mLibWidget, SLOT(onNewFolderToCreate()));
    connect(mLibWidget, SIGNAL(itemClicked()),libWidget->actionBar(), SLOT(onItemChanged()));
    connect(libWidget->pathViewer(), SIGNAL(mouseClick(UBChainedLibElement*)),libWidget->actionBar(), SLOT(onItemChanged()));
    mLibWidget->init();
}

/**
 * \brief Destructor
 */
UBLibNavigatorWidget::~UBLibNavigatorWidget()
{
    if(NULL != mLibWidget)
    {
        delete mLibWidget;
        mLibWidget = NULL;
    }
    if(NULL != mSlider)
    {
        delete mSlider;
        mSlider = NULL;
    }
    if(NULL != mSliderWidthSetting)
    {
        delete mSliderWidthSetting;
        mSliderWidthSetting = NULL;
    }
    if(NULL != mLayout)
    {
        delete mLayout;
        mLayout = NULL;
    }
}

void UBLibNavigatorWidget::dropMe(const QMimeData *_data)
{
    // Forward the mime data to the library widget
    Q_UNUSED(_data);
}

/**
 * \brief Update the navigation bar
 * @param pElem as the current element
 */
void UBLibNavigatorWidget::onNavigbarUpate(UBLibElement *pElem)
{
    Q_UNUSED(pElem);
    emit updateNavigBar(mLibWidget->chainedElements);
}

/**
 * \brief Handle the click event on an item
 * @param elem as the clicked element
 */
void UBLibNavigatorWidget::onPathItemClicked(UBChainedLibElement *elem)
{
	if (!this->libraryWidget()->isLoadingLibraryItems())
	{
		// If this element has some subelement, remove them
		removeNextChainedElements(elem);

		// The refresh the view
		mLibWidget->setCurrentElemsAndRefresh(elem);
	}
}

/**
 * \brief Remove the next chained elements
 * @param fromElem as the current elem
 */
void UBLibNavigatorWidget::removeNextChainedElements(UBChainedLibElement *fromElem)
{
    if(NULL != fromElem)
    {
        if(NULL != fromElem->nextElement())
        {
            //removeNextChainedElements(fromElem->nextElement());
            //delete fromElem->nextElement()->element();
            //delete fromElem->nextElement();
            delete fromElem->nextElement();
            fromElem->setNextElement(NULL);
        }
    }
}

/**
 * \brief Handles the properties requested event
 * @param elem as the related element
 */
void UBLibNavigatorWidget::onPropertiesRequested(UBLibElement *elem)
{
    emit propertiesRequested(elem);
}

/**
 * \brief Handles the display search engine requested event
 * @param elem as the related element
 */
void UBLibNavigatorWidget::onDisplaySearchEngine(UBLibElement *elem)
{
    emit displaySearchEngine(elem);
}

/**
 * \brief Update the thumbnails size
 * @param newSize as the given thumbnails size
 */
void UBLibNavigatorWidget::updateThumbnailsSize(int newSize)
{
    mSliderWidthSetting->set(newSize);
    mLibWidget->updateThumbnailsSize(newSize);
}
