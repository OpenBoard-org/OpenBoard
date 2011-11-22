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

#include "UBLibWidget.h"
#include "UBLibItemProperties.h"

#include "core/UBApplication.h"
#include "core/UBDownloadManager.h"

#include "frameworks/UBFileSystemUtils.h"

#include "core/memcheck.h"

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBLibItemProperties::UBLibItemProperties(QWidget *parent, const char *name):QWidget(parent)
    , mpLayout(NULL)
    , mpButtonLayout(NULL)
    , mpAddPageButton(NULL)
    , mpAddToLibButton(NULL)
    , mpSetAsBackgroundButton(NULL)
    , mpObjInfoLabel(NULL)
    , mpObjInfos(NULL)
    , mpThumbnail(NULL)
    , mpOrigPixmap(NULL)
    , mpElement(NULL)
    , mpItem(NULL)
{
    setObjectName(name);

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    // Create the GUI
    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    maxThumbHeight = height() / 4;

    mpThumbnail = new QLabel();
    QPixmap icon(":images/libpalette/notFound.png");
    icon.scaledToWidth(THUMBNAIL_WIDTH);

    mpThumbnail->setPixmap(icon);
    mpThumbnail->setObjectName("DockPaletteWidgetBox");
    mpThumbnail->setStyleSheet("background:white;");
    mpThumbnail->setAlignment(Qt::AlignHCenter);
    mpLayout->addWidget(mpThumbnail, 0);

    mpButtonLayout = new QHBoxLayout();
    mpLayout->addLayout(mpButtonLayout, 0);

    mpAddPageButton = new UBLibItemButton();
    mpAddPageButton->setText(tr("Add to page"));
    mpButtonLayout->addWidget(mpAddPageButton);

    mpSetAsBackgroundButton = new UBLibItemButton();
    mpSetAsBackgroundButton->setText(tr("Set as background"));
    mpButtonLayout->addWidget(mpSetAsBackgroundButton);

    mpAddToLibButton = new UBLibItemButton();
    mpAddToLibButton->setText(tr("Add to library"));
    mpButtonLayout->addWidget(mpAddToLibButton);

    mpButtonLayout->addStretch(1);

    mpObjInfoLabel = new QLabel(tr("Object informations"));
    mpObjInfoLabel->setStyleSheet(QString("color: #888888; font-size : 18px; font-weight:bold;"));
    mpLayout->addWidget(mpObjInfoLabel, 0);

    mpObjInfos = new QTreeWidget(this);
    mpObjInfos->setColumnCount(2);
    mpObjInfos->header()->hide();
    mpObjInfos->setAlternatingRowColors(true);
    mpObjInfos->setRootIsDecorated(false);
    mpObjInfos->setObjectName("DockPaletteWidgetBox");
    mpObjInfos->setStyleSheet("background:white;");
    mpLayout->addWidget(mpObjInfos, 1);

    connect(mpAddPageButton, SIGNAL(clicked()), this, SLOT(onAddToPage()));
    connect(mpSetAsBackgroundButton, SIGNAL(clicked()), this, SLOT(onSetAsBackground()));
    connect(mpAddToLibButton, SIGNAL(clicked()), this, SLOT(onAddToLib()));

}

/**
 * \brief Destructor
 */
UBLibItemProperties::~UBLibItemProperties()
{
    if(NULL != mpOrigPixmap)
    {
        delete mpOrigPixmap;
        mpOrigPixmap = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
    if(NULL != mpSetAsBackgroundButton)
    {
        delete mpSetAsBackgroundButton;
        mpSetAsBackgroundButton = NULL;
    }
    if(NULL != mpAddPageButton)
    {
        delete mpAddPageButton;
        mpAddPageButton = NULL;
    }
    if(NULL != mpAddToLibButton)
    {
        delete mpAddToLibButton;
        mpAddToLibButton = NULL;
    }
    if(NULL != mpObjInfoLabel)
    {
        delete mpObjInfoLabel;
        mpObjInfoLabel = NULL;
    }
    if(NULL != mpObjInfos)
    {
        delete mpObjInfos;
        mpObjInfos = NULL;
    }
    if(NULL != mpThumbnail)
    {
        delete mpThumbnail;
        mpThumbnail = NULL;
    }
}

/**
 * \brief Handle the resize event
 * @param event as the resize event
 */
void UBLibItemProperties::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    adaptSize();
}

/**
 * \brief Adapt the thumbnail size
 */
void UBLibItemProperties::adaptSize()
{
    if(NULL != mpOrigPixmap)
    {
        if(width() < THUMBNAIL_WIDTH + 40)
        {
            mpThumbnail->setPixmap(mpOrigPixmap->scaledToWidth(width() - 40));
        }
        else
        {
            mpThumbnail->setPixmap(mpOrigPixmap->scaledToWidth(THUMBNAIL_WIDTH));
        }
    }
}

/**
 * \brief Add the element to the page
 */
void UBLibItemProperties::onAddToPage()
{
    if(UBApplication::isFromWeb(mpElement->path().toString())){
        sDownloadFileDesc desc;
        desc.isBackground = false;
        desc.modal = true;
        desc.name = QFileInfo(mpElement->path().toString()).fileName();
        desc.url = mpElement->path().toString();
        UBDownloadManager::downloadManager()->addFileToDownload(desc);

    }else{
        UBLibWidget* libWidget = dynamic_cast<UBLibWidget*>(parentWidget()->parentWidget());
        libWidget->libNavigator()->libraryWidget()->libraryController()->addItemToPage(mpElement);
    }
}

/**
 * \brief Add the item to the library
 */
void UBLibItemProperties::onAddToLib()
{
    if(UBApplication::isFromWeb(mpElement->path().toString())){
        sDownloadFileDesc desc;
        desc.isBackground = false;
        desc.modal = false;
        desc.name = QFileInfo(mpElement->path().toString()).fileName();
        desc.url = mpElement->path().toString();
        UBDownloadManager::downloadManager()->addFileToDownload(desc);
    }
}

/**
 * \brief Set the item as background
 */
void UBLibItemProperties::onSetAsBackground()
{
    if(UBApplication::isFromWeb(mpElement->path().toString())){
        sDownloadFileDesc desc;
        desc.isBackground = true;
        desc.modal = true;
        desc.name = QFileInfo(mpElement->path().toString()).fileName();
        desc.url = mpElement->path().toString();
        UBDownloadManager::downloadManager()->addFileToDownload(desc);

    }else{
        UBLibWidget* libWidget = dynamic_cast<UBLibWidget*>(parentWidget()->parentWidget());
        libWidget->libNavigator()->libraryWidget()->libraryController()->setItemAsBackground(mpElement);
    }
}

/**
 * \brief Show the given element properties
 * @param elem as the given element
 */
void UBLibItemProperties::showElement(UBLibElement *elem)
{
    if(NULL != mpOrigPixmap)
    {
        delete mpOrigPixmap;
        mpOrigPixmap = NULL;
    }
    if(NULL != elem)
    {
        mpElement = elem;
        mpOrigPixmap = new QPixmap(QPixmap::fromImage(*elem->thumbnail()));
        mpThumbnail->setPixmap(QPixmap::fromImage(*elem->thumbnail()).scaledToWidth(THUMBNAIL_WIDTH));
        populateMetadata();
    }

    if(UBApplication::isFromWeb(elem->path().toString())){
        mpAddToLibButton->show();
        if(elem->metadatas()["Type"].toLower().contains("image")){
            mpSetAsBackgroundButton->show();
        }else{
            mpSetAsBackgroundButton->hide();
        }
    }else{
        mpAddToLibButton->hide();
        if(UBFileSystemUtils::mimeTypeFromFileName(elem->path().toLocalFile()).contains("image")){
            mpSetAsBackgroundButton->show();
        }else{
            mpSetAsBackgroundButton->hide();
        }
    }
}

/**
 * \brief Navigate backward
 */
void UBLibItemProperties::onBack()
{
    emit showFolderContent();
}

/**
 * \brief Handle the show event
 * @param event as the show event
 */
void UBLibItemProperties::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    adaptSize();
}

void UBLibItemProperties::populateMetadata()
{
    if(NULL != mpObjInfos){
        mpObjInfos->clear();
        QMap<QString, QString> metas = mpElement->metadatas();
        QList<QString> lKeys = metas.keys();
        QList<QString> lValues = metas.values();

        for(int i=0; i< metas.size(); i++){
            QStringList values;
            values << lKeys.at(i);
            values << lValues.at(i);
            mpItem = new QTreeWidgetItem(values);
            mpObjInfos->addTopLevelItem(mpItem);
        }
        mpObjInfos->resizeColumnToContents(0);
    }
}

/**
 * \brief Constructor
 * @param parent as the parent widget
 * @param name as the object name
 */
UBLibItemButton::UBLibItemButton(QWidget *parent, const char *name):QPushButton(parent)
{
    setObjectName(name);
    setStyleSheet(QString("background-color : #DDDDDD; color : #555555; border-radius : 6px; padding : 5px; font-weight : bold; font-size : 12px;"));
}

/**
 * \brief Destructor
 */
UBLibItemButton::~UBLibItemButton()
{

}
