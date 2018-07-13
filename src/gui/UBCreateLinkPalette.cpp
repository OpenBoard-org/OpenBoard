/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
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



#include "UBCreateLinkPalette.h"
#include <QVBoxLayout>
#include <QStackedWidget>
#include <QToolButton>
#include <QPushButton>
#include <QLineEdit>
#include <QPainter>
#include <QMouseEvent>
#include <QCheckBox>
#include <QButtonGroup>
#include <QComboBox>

#include "core/UBApplication.h"
#include "core/UBDownloadManager.h"

#include "document/UBDocumentController.h"

#include "board/UBBoardController.h"

#include "customWidgets/UBGraphicsItemAction.h"

#include "frameworks/UBFileSystemUtils.h"


UBCreateLinkLabel::UBCreateLinkLabel(QString labelText, QWidget *parent) :
    QLabel(parent)
{
    setAcceptDrops(true);
    mInitialText = labelText;
    setText(mInitialText);
    setMinimumSize(200, 200);
    setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);
    setAlignment(Qt::AlignCenter);
    setStyleSheet("border-style: dashed; border-color:#999999;border-radius: 10px; border-width: 2px ");
}

void UBCreateLinkLabel::dragEnterEvent(QDragEnterEvent *event)
{
    setText(tr("<drop content>"));
    setBackgroundRole(QPalette::Highlight);

    event->acceptProposedAction();
}

void UBCreateLinkLabel::dropEvent(QDropEvent *event)
{
    setBackgroundRole(QPalette::NoRole);
    const QMimeData *mimeData = event->mimeData();

    QString path;

    if (mimeData->hasImage())
        setText(tr("Images are not accepted"));
    else if (mimeData->hasHtml())
        path = mimeData->html();
    else if (mimeData->hasText())
        path = mimeData->text();
    else if (mimeData->hasUrls())
        path = mimeData->urls().at(0).toLocalFile();
    else
        setText(tr("Cannot display data"));

    if(UBFileSystemUtils::mimeTypeFromFileName(path).contains("audio")){
        if(!path.startsWith("file:"))
            setText(QFileInfo(path).completeBaseName());
        else
            setText(QFileInfo(QUrl(path).toLocalFile()).completeBaseName());
    }
    else
        setText(tr("Dropped file isn't reconized to be an audio file"));

    emit droppedFile(path);
    event->acceptProposedAction();
}

void UBCreateLinkLabel::dragLeaveEvent(QDragLeaveEvent *event)
{
    setText(mInitialText);
    event->accept();
}

UBCreateLinkPalette::UBCreateLinkPalette(QWidget *parent) :
    UBFloatingPalette(Qt::TopRightCorner, parent)
  ,mButtonGroup(0)
  ,mIsFirstTime(true)
{
    setObjectName("UBCreateLinkPalette");
    mLayout = new QVBoxLayout(this);
    mLayout->setContentsMargins(10,28,10,10);
    setLayout(mLayout);
    mStackedWidget = new QStackedWidget(this);
    mStackedWidget->setObjectName("UBCreateLinkPaletteStackedWidget");
    mStackedWidget->setStyleSheet("QWidget#UBCreateLinkPaletteStackedWidget{background-color:#eeeeee; border-width: 2px; border-style: solid; border-radius: 10px; border-color:#999999;}");
    mLayout->addWidget(mStackedWidget);
    init();
    hide();
}


UBCreateLinkPalette::~UBCreateLinkPalette()
{

}

QToolButton* UBCreateLinkPalette::addInitiaWidgetButton(QString textButton, QString iconPath)
{
    QToolButton* button = new QToolButton(mInitialWidget);
    button->setIcon(QIcon(iconPath));
    button->setIconSize(QSize(100,100));
    button->setText(textButton);
    button->setStyleSheet("background-color:transparent; board : none;");
    button->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    return button;
}

void UBCreateLinkPalette::init()
{
    mInitialWidget = new QWidget(this);
    QHBoxLayout* initialWidgetLayout = new QHBoxLayout(mInitialWidget);
    mInitialWidget->setLayout(initialWidgetLayout);

    QToolButton* actionPlayAudio = addInitiaWidgetButton(tr("Play an audio file"),":images/createLinkPalette/PlayAudio.svg");
    QToolButton* actionAddLinkToPage = addInitiaWidgetButton(tr("Add Link to Page"),":images/createLinkPalette/LinkPage.svg");
    QToolButton* actionAddLinkToWeb = addInitiaWidgetButton(tr("Add Link to a Web page"),":images/createLinkPalette/LinkWeb.svg");

    actionPlayAudio->setStyleSheet("{background-color:#eeeeee;border-width: 0px;}");
    actionAddLinkToPage->setStyleSheet("{background-color:#eeeeee;border-width: 0px;}");
    actionAddLinkToWeb->setStyleSheet("{background-color:#eeeeee; border:0px; margin:0px; padding:0px;}");

    initialWidgetLayout->addWidget(actionPlayAudio);
    initialWidgetLayout->addWidget(actionAddLinkToPage);
    initialWidgetLayout->addWidget(actionAddLinkToWeb);
    mStackedWidget->addWidget(mInitialWidget);


    mAudioWidget = new QWidget(this);
    QVBoxLayout* audioWidgetLayout = new QVBoxLayout(mAudioWidget);
    mAudioWidget->setLayout(audioWidgetLayout);
    QHBoxLayout* audioBackButtonLayout = new QHBoxLayout();
    QPushButton* audioBackButton = new QPushButton(mAudioWidget);
    audioBackButton->setIcon(QIcon(":images/toolbar/undoOn.png"));
    audioBackButtonLayout->addWidget(audioBackButton);
    audioBackButtonLayout->addStretch();
    audioWidgetLayout->addLayout(audioBackButtonLayout);
    connect(audioBackButton,SIGNAL(clicked()),this,SLOT(onBackButtonClicked()));
    mpAudioLabel = new UBCreateLinkLabel(tr("Drag and drop the audio file from the library in this box"),mAudioWidget);
    connect(mpAudioLabel,SIGNAL(droppedFile(QString&)),this,SLOT(onDroppedAudioFile(QString&)));
    audioWidgetLayout->addWidget(mpAudioLabel);
    QHBoxLayout* audioOkButtonLayout = new QHBoxLayout();
    audioOkButtonLayout->addStretch();
    QPushButton* audioOkButton = new QPushButton(tr("Ok"),mAudioWidget);
    audioOkButtonLayout->addWidget(audioOkButton);
    audioWidgetLayout->addLayout(audioOkButtonLayout);
    connect(audioOkButton,SIGNAL(clicked()),this,SLOT(onOkAudioClicked()));
    mStackedWidget->addWidget(mAudioWidget);
    connect(actionPlayAudio,SIGNAL(clicked()),this,SLOT(onPlayAudioClicked()));


    mPageLinkWidget = new QWidget(this);
    QVBoxLayout* pageLinkWidgetLayout = new QVBoxLayout(mPageLinkWidget);
    mPageLinkWidget->setLayout(pageLinkWidgetLayout);
    QHBoxLayout* pageLinkBackButtonLayout = new QHBoxLayout();
    QPushButton* pageLinkBackButton = new QPushButton(mPageLinkWidget);
    pageLinkBackButton->setIcon(QIcon(":images/toolbar/undoOn.png"));
    pageLinkBackButtonLayout->addWidget(pageLinkBackButton);
    pageLinkBackButtonLayout->addStretch();
    pageLinkWidgetLayout->addLayout(pageLinkBackButtonLayout);

    int activeIndex = UBApplication::boardController->activeSceneIndex();
    int lastSceneIndex = UBApplication::boardController->selectedDocument()->pageCount() - 1;
    mButtonGroup = new QButtonGroup();
    mButtonGroup->setExclusive(true);
    QCheckBox* nextPageCheckBox = new QCheckBox(tr("Next Page"),this);
    mButtonGroup->addButton(nextPageCheckBox,eMoveToNextPage);
    pageLinkWidgetLayout->addWidget(nextPageCheckBox);
    if(activeIndex >= lastSceneIndex)
        nextPageCheckBox->setEnabled(false);
    QCheckBox* previousPageCheckBox = new QCheckBox(tr("Previous Page"),this);
    mButtonGroup->addButton(previousPageCheckBox,eMoveToPreviousPage);
    if(activeIndex == 0)
        previousPageCheckBox->setEnabled(false);
    pageLinkWidgetLayout->addWidget(previousPageCheckBox);
    QCheckBox* firstPageCheckBox = new QCheckBox(tr("Title Page"),this);
    mButtonGroup->addButton(firstPageCheckBox,eMoveToFirstPage);
    pageLinkWidgetLayout->addWidget(firstPageCheckBox);
    QCheckBox* lastPageCheckBox = new QCheckBox(tr("Last Page"),this);
    mButtonGroup->addButton(lastPageCheckBox,eMoveToLastPage);
    pageLinkWidgetLayout->addWidget(lastPageCheckBox);

    QHBoxLayout* toPageNumberLayout = new QHBoxLayout();
    QCheckBox* pageNumberCheckBox = new QCheckBox(tr("Page Number"),this);
    mButtonGroup->addButton(pageNumberCheckBox,eMoveToPage);
    toPageNumberLayout->addWidget(pageNumberCheckBox);
    mPageComboBox = new QComboBox(this);
    toPageNumberLayout->addWidget(mPageComboBox);
    for(int sceneIndex = 0; sceneIndex <= lastSceneIndex;sceneIndex+=1)
        if(sceneIndex != activeIndex)
            mPageComboBox->insertItem(sceneIndex,QString("%1").arg(sceneIndex));
    if(!mPageComboBox->count())
        pageNumberCheckBox->setEnabled(false);
    mPageComboBox->setEnabled(false);
    connect(pageNumberCheckBox,SIGNAL(clicked(bool)),this,SLOT(onPageNumberCheckBoxClicked(bool)));
    pageLinkWidgetLayout->addLayout(toPageNumberLayout);


    connect(pageLinkBackButton,SIGNAL(clicked()),this,SLOT(onBackButtonClicked()));
    QHBoxLayout* pageLinkOkButtonLayout = new QHBoxLayout();
    pageLinkOkButtonLayout->addStretch();
    QPushButton* pageLinkOkButton = new QPushButton(tr("Ok"), mPageLinkWidget);
    pageLinkOkButtonLayout->addWidget(pageLinkOkButton);
    pageLinkWidgetLayout->addLayout(pageLinkOkButtonLayout);
    connect(pageLinkOkButton,SIGNAL(clicked()),this,SLOT(onOkLinkToPageClicked()));
    mStackedWidget->addWidget(mPageLinkWidget);
    connect(actionAddLinkToPage,SIGNAL(clicked()),this,SLOT(onAddLinkToPageClicked()));


    mUrlLinkWidget = new QWidget(this);
    QVBoxLayout* urlLinkWidgetLayout = new QVBoxLayout(mUrlLinkWidget);
    mUrlLinkWidget->setLayout(urlLinkWidgetLayout);
    QHBoxLayout* urlLinkBackButtonLayout = new QHBoxLayout();
    QPushButton* urlLinkBackButton = new QPushButton(mUrlLinkWidget);
    urlLinkBackButton->setIcon(QIcon(":images/toolbar/undoOn.png"));
    urlLinkBackButtonLayout->addWidget(urlLinkBackButton);
    urlLinkBackButtonLayout->addStretch();
    urlLinkWidgetLayout->addLayout(urlLinkBackButtonLayout);
    connect(urlLinkBackButton,SIGNAL(clicked()),this,SLOT(onBackButtonClicked()));
    mUrlLineEdit = new QLineEdit(mUrlLinkWidget);
    mUrlLineEdit->setPlaceholderText(tr("Insert url text here"));
    mUrlLineEdit->setAcceptDrops(true);
    urlLinkWidgetLayout->addWidget(mUrlLineEdit);
    QHBoxLayout* urlLinkOkButtonLayout = new QHBoxLayout();
    urlLinkOkButtonLayout->addStretch();
    QPushButton* urlLinkOkButton = new QPushButton(tr("Ok"), mUrlLinkWidget);
    urlLinkOkButtonLayout->addWidget(urlLinkOkButton);
    urlLinkWidgetLayout->addLayout(urlLinkOkButtonLayout);
    connect(urlLinkOkButton,SIGNAL(clicked()),this,SLOT(onOkLinkToWebClicked()));
    mStackedWidget->addWidget(mUrlLinkWidget);
    connect(actionAddLinkToWeb,SIGNAL(clicked()),this,SLOT(onAddLinkToWebClicked()));

}


void UBCreateLinkPalette::onDroppedAudioFile(QString &path)
{
    Q_ASSERT(path.length());

    mAudioFilePath = path;
}

void UBCreateLinkPalette::onPageNumberCheckBoxClicked(bool checked)
{
   mPageComboBox->setEnabled(checked);
}

void UBCreateLinkPalette::onBackButtonClicked()
{
    mStackedWidget->setCurrentIndex(0);
    adjustGeometry();
}

void UBCreateLinkPalette::onPlayAudioClicked()
{
    mStackedWidget->setCurrentIndex(1);
}

void UBCreateLinkPalette::onAddLinkToPageClicked()
{
    mStackedWidget->setCurrentIndex(2);
}

void UBCreateLinkPalette::onAddLinkToWebClicked()
{
    mStackedWidget->setCurrentIndex(3);
}

void UBCreateLinkPalette::onOkAudioClicked()
{
    emit definedAction(new UBGraphicsItemPlayAudioAction(mAudioFilePath));
    close();
}

void UBCreateLinkPalette::onOkLinkToPageClicked()
{
    eUBGraphicsItemMovePageAction id = (eUBGraphicsItemMovePageAction)mButtonGroup->checkedId();
    UBGraphicsItemMoveToPageAction* action = 0;
    if(id!= eMoveToPage)
        action = new UBGraphicsItemMoveToPageAction(id);
    else
        action = new UBGraphicsItemMoveToPageAction(id,mPageComboBox->currentText().toInt());
    emit definedAction(action);
    close();
}

void UBCreateLinkPalette::onOkLinkToWebClicked()
{
    emit definedAction(new UBGraphicsItemLinkToWebPageAction (mUrlLineEdit->text()));
    close();
}

void UBCreateLinkPalette::paintEvent(QPaintEvent *event)
{
    UBFloatingPalette::paintEvent(event);

    QPainter painter(this);
    painter.drawPixmap(0, 0, QPixmap(":/images/close.svg"));
}


void UBCreateLinkPalette::close()
{
    hide();
}


void UBCreateLinkPalette::mouseReleaseEvent(QMouseEvent * event)
{
    if (event->pos().x() >= 0 && event->pos().x() < QPixmap(":/images/close.svg").width()
        && event->pos().y() >= 0 && event->pos().y() < QPixmap(":/images/close.svg").height())
    {
        event->accept();
        close();
    }

    UBFloatingPalette::mouseReleaseEvent(event);
}


void UBCreateLinkPalette::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    adjustGeometry();
}

void UBCreateLinkPalette::adjustGeometry()
{
    if(mIsFirstTime){
        adjustSizeAndPosition();
        move((parentWidget()->width() - width()) / 2, (parentWidget()->height() - height()) / 5);
        mIsFirstTime = false;
    }
}
