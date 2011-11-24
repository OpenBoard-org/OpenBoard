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
#include <QDebug>

#include "UBLibWidget.h"
#include "core/UBApplication.h"

#include "core/memcheck.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBLibWidget::UBLibWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
  , mLayout(NULL)
  , mStackedWidget(NULL)
  , mNavigator(NULL)
  , mProperties(NULL)
  , mActionBar(NULL)
  , mpWebView(NULL)
  , mpPathViewer(NULL)
{
    setObjectName(name);
    mName = "LibWidget";
    mVisibleState = true;

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    mIconToLeft = QPixmap(":images/library_open.png");
    mIconToRight = QPixmap(":images/library_close.png");
    setAcceptDrops(true);

    mLayout = new QVBoxLayout(this);
    setLayout(mLayout);

    // -------------
    // Build the GUI
    // -------------
    // The 'global' widgets
    mStackedWidget = new QStackedWidget(this);
    mActionBar = new UBLibActionBar(this);
    mpPathViewer = new UBLibPathViewer(this);
    mpPathViewer->setMaximumHeight(62);

    // The internal widgets
    mNavigator = new UBLibNavigatorWidget(this);
    mProperties = new UBLibItemProperties(this);
    mpWebView = new UBLibWebView(this);

    mLayout->addWidget(mpPathViewer, 0);
    mLayout->addWidget(mStackedWidget, 1);
    mLayout->addWidget(mActionBar, 0);

    mStackedWidget->addWidget(mNavigator);
    mStackedWidget->addWidget(mProperties);
    mStackedWidget->addWidget(mpWebView);

    mStackedWidget->setCurrentIndex(ID_NAVIGATOR);
    miCrntStackWidget = ID_NAVIGATOR;

    connect(mNavigator, SIGNAL(updateNavigBar(UBChainedLibElement*)), this, SLOT(onUpdateNavigBar(UBChainedLibElement*)));
    connect(mNavigator, SIGNAL(propertiesRequested(UBLibElement*)), this, SLOT(showProperties(UBLibElement*)));
    connect(mNavigator, SIGNAL(displaySearchEngine(UBLibElement*)), this, SLOT(showSearchEngine(UBLibElement*)));
    connect(mProperties, SIGNAL(showFolderContent()), this, SLOT(showFolder()));
    connect(this, SIGNAL(showLibElemProperties()), mpPathViewer, SLOT(showBack()));
    connect(this, SIGNAL(showLibSearchEngine()), mpPathViewer, SLOT(showBack()));
}

/**
 * \brief Destructor
 */
UBLibWidget::~UBLibWidget()
{
    if(NULL != mpPathViewer)
    {
        delete mpPathViewer;
        mpPathViewer = NULL;
    }
    if(NULL != mNavigator)
    {
        delete mNavigator;
        mNavigator = NULL;
    }
    if(NULL != mpWebView)
    {
        delete mpWebView;
        mpWebView = NULL;
    }
    if(NULL != mProperties)
    {
        delete mProperties;
        mProperties = NULL;
    }
    if(NULL != mStackedWidget)
    {
        delete mStackedWidget;
        mStackedWidget = NULL;
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
 * \brief Handles the drag enter event
 * @param pEvent as the drag enter event
 */
void UBLibWidget::dragEnterEvent(QDragEnterEvent *pEvent)
{
    setBackgroundRole(QPalette::Highlight);
    pEvent->acceptProposedAction();
}

void UBLibWidget::dragLeaveEvent(QDragLeaveEvent *pEvent)
{
    pEvent->accept();
}

/**
 * \brief Handles the drop event
 * @param pEvent as the drop event
 */
void UBLibWidget::dropEvent(QDropEvent *pEvent)
{
    processMimeData(pEvent->mimeData());
    setBackgroundRole(QPalette::Dark);
    mStackedWidget->setCurrentIndex(miCrntStackWidget);
    pEvent->acceptProposedAction();
}

/**
 * \brief Handles the drag move event
 * @param pEvent as the drag move event
 */
void UBLibWidget::dragMoveEvent(QDragMoveEvent *pEvent)
{
    pEvent->acceptProposedAction();
}

/**
 * \brief Process the dropped MIME data
 * @param pData as the mime dropped data
 */
void UBLibWidget::processMimeData(const QMimeData *pData)
{
    // Display the different mime types contained in the mime data
    QStringList qslFormats = pData->formats();
    for(int i = 0; i < qslFormats.size(); i++)
    {
        qDebug() << "Dropped element format " << i << " = "<< qslFormats.at(i);
    }
}

void UBLibWidget::showProperties(UBLibElement *elem)
{
    if(NULL != elem)
    {
        emit showLibElemProperties();
        mActionBar->setButtons(eButtonSet_Properties);
        mProperties->showElement(elem);
        mStackedWidget->setCurrentIndex(ID_PROPERTIES);
        miCrntStackWidget = ID_PROPERTIES;
    }
}

void UBLibWidget::showSearchEngine(UBLibElement *elem)
{
    if(NULL != elem)
    {
        emit showLibSearchEngine();
        mActionBar->hide();
        mpWebView->setElement(elem);
        mStackedWidget->setCurrentIndex(ID_WEBVIEW);
        miCrntStackWidget = ID_WEBVIEW;
    }
}

void UBLibWidget::showFolder()
{
    mActionBar->setButtons(mActionBar->previousButtonSet());
    mStackedWidget->setCurrentIndex(ID_NAVIGATOR);
    miCrntStackWidget = ID_NAVIGATOR;
}

int UBLibWidget::customMargin()
{
    return 5;
}

int UBLibWidget::border()
{
    return 15;
}

void UBLibWidget::onUpdateNavigBar(UBChainedLibElement *elem)
{
    mpPathViewer->displayPath(elem);
    mpPathViewer->show();

    if(ID_NAVIGATOR != miCrntStackWidget)
    {
        showFolder();
    }
}
