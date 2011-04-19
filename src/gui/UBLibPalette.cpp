#include <QDebug>
#include "UBLibPalette.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBLibPalette::UBLibPalette(QWidget *parent, const char *name):UBDockPalette(parent, name)
    , mLayout(NULL)
    , mStackedWidget(NULL)
    , mNavigator(NULL)
    , mProperties(NULL)
    , mActionBar(NULL)
{
    setOrientation(eUBDockOrientation_Right);
    mIcon = QPixmap(":images/paletteLibrary.png");
    setAcceptDrops(true);

    resize(300, height());
    setContentsMargins(border(), 0, 0, 0);
    mCollapseWidth = 180;

    mLayout = new QVBoxLayout(this);
    mLayout->setMargin(3);
    setLayout(mLayout);

    // Build the GUI
    mStackedWidget = new QStackedWidget(this);
    mActionBar = new UBLibActionBar(this);
    mNavigator = new UBLibNavigatorWidget(this);
    mProperties = new UBLibItemProperties(this);

    mLayout->addWidget(mStackedWidget, 1);
    mLayout->addWidget(mActionBar, 0);

    mStackedWidget->addWidget(mNavigator);
    mStackedWidget->addWidget(mProperties);

    mStackedWidget->setCurrentIndex(ID_NAVIGATOR);

    connect(mNavigator, SIGNAL(propertiesRequested(UBLibElement*)), this, SLOT(showProperties(UBLibElement*)));
    connect(mProperties, SIGNAL(showFolderContent()), this, SLOT(showFolder()));
}

/**
 * \brief Destructor
 */
UBLibPalette::~UBLibPalette()
{
    if(NULL != mStackedWidget)
    {
        delete mStackedWidget;
        mStackedWidget = NULL;
    }
    if(NULL != mNavigator)
    {
        delete mNavigator;
        mNavigator = NULL;
    }
    if(NULL != mProperties)
    {
        delete mProperties;
        mProperties = NULL;
    }
    if(NULL != mActionBar)
    {
        delete mActionBar;
        mActionBar = NULL;
    }
    if(NULL != mLayout)
    {
	delete mLayout;
	mLayout = NULL;
    }
}

/**
 * \brief Update the maximum width
 */
void UBLibPalette::updateMaxWidth()
{
    setMaximumWidth((int)((parentWidget()->width() * 2)/3));
    setMaximumHeight(parentWidget()->height());
    setMinimumHeight(parentWidget()->height());
}

/**
 * \brief Handles the drag enter event
 * @param pEvent as the drag enter event
 */
void UBLibPalette::dragEnterEvent(QDragEnterEvent *pEvent)
{
    setBackgroundRole(QPalette::Highlight);
    pEvent->acceptProposedAction();
}

/**
 * \brief Handles the drop event
 * @param pEvent as the drop event
 */
void UBLibPalette::dropEvent(QDropEvent *pEvent)
{
    processMimeData(pEvent->mimeData());
    setBackgroundRole(QPalette::Dark);
    pEvent->acceptProposedAction();
}

/**
 * \brief Handles the drag move event
 * @param pEvent as the drag move event
 */
void UBLibPalette::dragMoveEvent(QDragMoveEvent *pEvent)
{
    pEvent->acceptProposedAction();
}

/**
 * \brief Process the dropped MIME data
 * @param pData as the mime dropped data
 */
void UBLibPalette::processMimeData(const QMimeData *pData)
{
    // Display the different mime types contained in the mime data
    QStringList qslFormats = pData->formats();
    for(int i = 0; i < qslFormats.size(); i++)
    {
        qDebug() << "Dropped element format " << i << " = "<< qslFormats.at(i);
    }

}

void UBLibPalette::mouseMoveEvent(QMouseEvent *event)
{
    if(mCanResize)
    {
        UBDockPalette::mouseMoveEvent(event);
    }
    else
    {
        //qDebug() << "Mouse move event detected!" ;
    }
}

void UBLibPalette::showProperties(UBLibElement *elem)
{
    if(NULL != elem)
    {
        mActionBar->setButtons(eButtonSet_Properties);
        // Show the properties of this object
        mProperties->showElement(elem);
        mStackedWidget->setCurrentIndex(ID_PROPERTIES);
    }
}

void UBLibPalette::showFolder()
{
    mActionBar->setButtons(mActionBar->previousButtonSet());
    mStackedWidget->setCurrentIndex(ID_NAVIGATOR);
}
