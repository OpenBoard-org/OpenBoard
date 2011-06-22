#include "UBLibNavigatorWidget.h"
#include "UBLibPalette.h"

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
    , mPathViewer(NULL)
    , mSlider(NULL)
    , mSliderWidthSetting(NULL)
{
    setObjectName(name);
    setAcceptDrops(true);

    UBLibPalette* pLibPalette = dynamic_cast<UBLibPalette*>(parentWidget());

    mLayout = new QVBoxLayout(this);
    setLayout(mLayout);

    mPathViewer = new UBLibPathViewer(this);
    mPathViewer->setMaximumHeight(62);
    mLayout->addWidget(mPathViewer, 0);

    mLibWidget = new UBLibraryWidget(this);
    mLayout->addWidget(mLibWidget, 1);

    mSlider = new QSlider(Qt::Horizontal, this);
    mSlider->setMinimumHeight(20);
    mSlider->setStyleSheet(QString("QSlider::handle::horizontal{background-color:#EEEEEE; margin-top:-5px; margin-bottom:-5px; height:20px; width:18px; border-radius:10px; border:1px solid #555555;} QSlider::groove::horizontal{background-color:#999999; height:10px; border-radius:5px; border:1px solid #555555;}"));
    mSlider->setRange(lowBoundForSlider, topBoundForSlider);
    mSliderWidthSetting = new UBSetting(UBSettings::settings(), "Library", "LibWidgetWidth", topBoundForSlider);
    int defaultWidth = mSliderWidthSetting->get().toInt();
    mSlider->setValue(defaultWidth);
    mSlider->setTickInterval(tickIntervalForSlider);
    mLayout->addWidget(mSlider, 0);
    mLibWidget->updateThumbnailsSize(defaultWidth);

    connect(mLibWidget, SIGNAL(navigBarUpdate(UBLibElement*)), this, SLOT(onNavigbarUpate(UBLibElement*)));
    connect(mLibWidget, SIGNAL(propertiesRequested(UBLibElement*)), this, SLOT(onPropertiesRequested(UBLibElement*)));
    connect(mPathViewer, SIGNAL(mouseClick(UBChainedLibElement*)), this, SLOT(onPathItemClicked(UBChainedLibElement*)));
    connect(mSlider,SIGNAL(valueChanged(int)),this,SLOT(updateThumbnailsSize(int)));
    connect(mPathViewer, SIGNAL(elementsDropped(QList<QString>,UBLibElement*)), mLibWidget, SLOT(onElementsDropped(QList<QString>,UBLibElement*)));

    if(NULL != pLibPalette)
    {
       connect(mLibWidget, SIGNAL(navigBarUpdate(UBLibElement*)), pLibPalette->actionBar(), SLOT(onNavigbarUpdate(UBLibElement*)));
       connect(mLibWidget, SIGNAL(itemsSelected(QList<UBLibElement*>, bool)), pLibPalette->actionBar(), SLOT(onSelectionChanged(QList<UBLibElement*>, bool)));
       connect(pLibPalette->actionBar(), SIGNAL(deleteDone()), mLibWidget, SLOT(onRefreshCurrentFolder()));
       connect(mLibWidget, SIGNAL(favoritesEntered(bool)), pLibPalette->actionBar(), SLOT(onFavoritesEntered(bool)));
       connect(pLibPalette->actionBar(), SIGNAL(searchElement(QString)), mLibWidget, SLOT(onSearchElement(QString)));
       connect(pLibPalette->actionBar(), SIGNAL(newFolderToCreate()), mLibWidget, SLOT(onNewFolderToCreate()));
    }

    mLibWidget->init();
}

/**
 * \brief Destructor
 */
UBLibNavigatorWidget::~UBLibNavigatorWidget()
{
    //if(NULL != mLayout)
    //{
    //    delete mLayout;
    //    mLayout = NULL;
    //}
    //if(NULL != mLibWidget)
    //{
    //    delete mLibWidget;
    //    mLibWidget = NULL;
    //}
    if(NULL != mPathViewer)
    {
        delete mPathViewer;
        mPathViewer = NULL;
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
}

void UBLibNavigatorWidget::dropMe(const QMimeData *_data)
{
    // Forward the mime data to the library widget

}

/**
 * \brief Update the navigation bar
 * @param pElem as the current element
 */
void UBLibNavigatorWidget::onNavigbarUpate(UBLibElement *pElem)
{
    if(NULL != pElem)
    {
		// Refresh the path navigator
		mPathViewer->displayPath(mLibWidget->chainedElements);

		// Show the path navigator
		mPathViewer->show();
    }
}

/**
 * \brief Handle the click event on an item
 * @param elem as the clicked element
 */
void UBLibNavigatorWidget::onPathItemClicked(UBChainedLibElement *elem)
{
    // If this element has some subelement, remove them
    removeNextChainedElements(elem);

    // The refresh the view
    mLibWidget->setCurrentElemsAndRefresh(elem);
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
            removeNextChainedElements(fromElem->nextElement());
            delete fromElem->nextElement();
            fromElem->setNextElement(NULL);
        }
    }
}

/**
 * \brief Handles the properties requested event
 * @param elem as tje related element
 */
void UBLibNavigatorWidget::onPropertiesRequested(UBLibElement *elem)
{
    emit propertiesRequested(elem);
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
