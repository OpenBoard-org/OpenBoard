#include "UBLibItemProperties.h"

#include "core/UBApplication.h"

#include "board/UBBoardController.h"

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
    , mpNavigBar(NULL)
    , mpNavigLayout(NULL)
    , mpNavigBack(NULL)
    , mpNavigBackAction(NULL)
{
    setObjectName(name);

    // Create the GUI
    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    mpNavigBar = new QWidget(this);
    mpNavigLayout = new QHBoxLayout(mpNavigBar);
    mpNavigBar->setLayout(mpNavigLayout);
    mpNavigBar->setStyleSheet(QString("background-color : white; border-radius : 10px;"));
    mpNavigBar->setMaximumHeight(42);
    mpNavigBackAction = new QAction(QIcon(":/images/libpalette/back.png"), tr("Back"), mpNavigBar);
    mpNavigBack = new QToolButton(mpNavigBar);
    mpNavigBar->addAction(mpNavigBackAction);
    mpNavigBack->setDefaultAction(mpNavigBackAction);
    mpNavigBack->setIconSize(QSize(32,32));
    mpNavigBack->setToolButtonStyle(Qt::ToolButtonIconOnly);
    mpNavigBack->setStyleSheet(QString("QToolButton {color: white; font-weight: bold; font-family: Arial; background-color: transparent; border: none}"));
    mpNavigBack->setFocusPolicy(Qt::NoFocus);

    mpNavigLayout->addWidget(mpNavigBack, 0);
    connect(mpNavigBackAction, SIGNAL(triggered()), this, SLOT(onBack()));

    mpNavigLayout->addStretch(1);

    mpLayout->addWidget(mpNavigBar);

    maxThumbHeight = height() / 4;

    mpThumbnail = new QLabel();
    QPixmap icon(":images/libpalette/notFound.png");
    icon.scaledToWidth(THUMBNAIL_WIDTH);

    mpThumbnail->setPixmap(icon);
    mpThumbnail->setStyleSheet(QString("background-color : white; padding : 10 px; border-radius : 10px;"));
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

    mpObjInfos = new QTextEdit(this);
    mpObjInfos->setReadOnly(true);
    mpObjInfos->setStyleSheet(QString("background-color: white; border-radius : 10px;"));
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
    if(NULL != mpNavigBackAction)
    {
        delete mpNavigBackAction;
        mpNavigBackAction = NULL;
    }
    if(NULL != mpNavigBack)
    {
        delete mpNavigBack;
        mpNavigBack = NULL;
    }
    if(NULL != mpNavigLayout)
    {
        delete mpNavigLayout;
        mpNavigLayout = NULL;
    }
    if(NULL != mpNavigBar)
    {
        delete mpNavigLayout;
        mpNavigLayout = NULL;
    }
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
    //if(NULL != mpButtonLayout)
    //{
    //    delete mpButtonLayout;
    //    mpButtonLayout = NULL;
    //}
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
    if(NULL != mpNavigBar)
    {
        mpNavigBar->resize(width(), mpNavigBar->height());
    }
}

/**
 * \brief Add the element to the page
 */
void UBLibItemProperties::onAddToPage()
{
    UBApplication::boardController->libraryController()->addItemToPage(mpElement);
}

/**
 * \brief Add the item to the library
 */
void UBLibItemProperties::onAddToLib()
{

}

/**
 * \brief Set the item as background
 */
void UBLibItemProperties::onSetAsBackground()
{
    UBApplication::boardController->libraryController()->setItemAsBackground(mpElement);
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
        mpObjInfos->setText(elem->information());
    }

    if(UBFileSystemUtils::mimeTypeFromFileName(elem->path().toLocalFile()).contains("image"))
    {
        // Show the Set as background button
        mpAddToLibButton->hide();
        mpSetAsBackgroundButton->show();
    }
    else
    {
        // TODO: if we are browsing ONLINE objects, we must show mpAddToLibButton !
        mpSetAsBackgroundButton->hide();
        mpAddToLibButton->hide();
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
