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
{
    setObjectName(name);
    mName = "LibWidget";
    mIconToLeft = QPixmap(":images/library_open.png");
    mIconToRight = QPixmap(":images/library_close.png");
    setAcceptDrops(true);

    mLayout = new QVBoxLayout(this);
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
    miCrntStackWidget = ID_NAVIGATOR;

    connect(mNavigator, SIGNAL(propertiesRequested(UBLibElement*)), this, SLOT(showProperties(UBLibElement*)));
    connect(mProperties, SIGNAL(showFolderContent()), this, SLOT(showFolder()));
}

/**
 * \brief Destructor
 */
UBLibWidget::~UBLibWidget()
{
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
        mActionBar->setButtons(eButtonSet_Properties);
        // Show the properties of this object
        mProperties->showElement(elem);
        mStackedWidget->setCurrentIndex(ID_PROPERTIES);
        miCrntStackWidget = ID_PROPERTIES;
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
