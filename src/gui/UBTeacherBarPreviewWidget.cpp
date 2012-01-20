#include "core/UBApplication.h"
#include "customWidgets/UBGlobals.h"
#include "frameworks/UBFileSystemUtils.h"

#include "UBTeacherBarPreviewWidget.h"


UBTeacherBarPreviewMedia::UBTeacherBarPreviewMedia(QWidget* parent, const char* name) : QWidget(parent)
{
    setObjectName(name);
    mWidget = new UBWidgetList(parent);
    mLayout.addWidget(mWidget);
    setLayout(&mLayout);
    mWidgetList.clear();
}

UBTeacherBarPreviewMedia::~UBTeacherBarPreviewMedia()
{
    DELETEPTR(mWidget);
}

void UBTeacherBarPreviewMedia::cleanMedia()
{
    foreach(QWidget* eachWidget, mWidgetList.keys()){
        if(QString(eachWidget->metaObject()->className()).contains("UBDraggable")){
            mWidget->removeWidget(eachWidget);
            delete eachWidget;
            eachWidget = NULL;
        }
        else{
            mWidget->removeWidget(eachWidget);
        }
    }
    mWidgetList.clear();
}

void UBTeacherBarPreviewMedia::loadWidgets(QList<QWidget*> pWidgetsList, bool isResizable)
{
    foreach(QWidget*eachWidget, pWidgetsList){
        mWidget->addWidget(eachWidget);
        mWidgetList[eachWidget]="DRAG UNAVAILABLE";
    }
}


int UBTeacherBarPreviewMedia::loadMedia(QStringList pMedias)
{
    int addedMedia = 0;
//    foreach(QString eachString, pMedias){
//        if(!eachString.isEmpty()){
//            QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(eachString);
//            if(mimeType.contains("image")){
//                UBDraggableLabel* label = new UBDraggableLabel();
//                label->loadImage(eachString);
//                mWidget->addWidget(label);
//                mWidgetList[label]=eachString;
//                addedMedia += 1;
//            }
//            else if(mimeType.contains("video") || mimeType.contains("audio")){
//                UBDraggableMediaPlayer* mediaPlayer = new UBDraggableMediaPlayer();
//                mediaPlayer->setFile(eachString);
//                mWidget->addWidget(mediaPlayer);
//                mWidgetList[mediaPlayer] = eachString;
//                addedMedia += 1;
//            }
//            else{
//                qWarning() << "pMediaPath" << eachString;
//                qWarning() << "bad idea to come here";
//            }
//        }
//    }
    return addedMedia;
}

// -----------------------------------------------------------------------------------------------
UBActionPreview::UBActionPreview(QWidget *parent, const char *name):QWidget(parent)
  , mpOwner(NULL)
  , mpContent(NULL)
{
    setObjectName(name);
    setLayout(&mLayout);
    mpOwner = new QLabel(this);
    mpOwner->setObjectName("UBActionPreviewOwner");
    mOwnerLayout.addWidget(mpOwner, 0);
    mOwnerLayout.addStretch(1);
    mLayout.addLayout(&mOwnerLayout);
    mpContent = new QLabel(this);
    mpContent->setObjectName("UBActionPreviewContent");
    mpContent->setWordWrap(true);
    mLayout.addWidget(mpContent);
    setContentsMargins(-9, -9, -9, -9);
}

UBActionPreview::~UBActionPreview()
{
    if(NULL != mpOwner){
        delete mpOwner;
        mpOwner = NULL;
    }
    if(NULL != mpContent){
        delete mpContent;
        mpContent = NULL;
    }
}

void UBActionPreview::setOwner(const QString &owner)
{
    if(NULL != mpOwner && NULL != mpContent){
        switch(owner.toInt()){
            case eActionOwner_Teacher:
                mpOwner->setText(tr("Teacher"));
                mpContent->setStyleSheet("background:lightblue; border:lightblue;");
                break;

            case eActionOwner_Student:
                mpOwner->setText(tr("Student"));
                mpContent->setStyleSheet("background:lightgreen; border:lightgreen;");
                break;
        }
    }
}

void UBActionPreview::setContent(const QString &content)
{
    if(NULL != mpContent){
        mpContent->setText(content);
    }
}

// -------------------------------------------------------------------------------------------------------------------
UBTBPreviewContainer::UBTBPreviewContainer(QWidget *parent, const char *name):UBWidgetList(parent)
{
    setObjectName(name);
}

UBTBPreviewContainer::~UBTBPreviewContainer()
{

}

// ------------------------------------------------------------------------------------
UBTeacherBarPreviewWidget::UBTeacherBarPreviewWidget(UBTeacherBarDataMgr* pDataMgr, QWidget *parent, const char *name):QWidget(parent)
  , mpEditButton(NULL)
  , mpTitle(NULL)
  , mpDuration(NULL)
  , mpActionsLabel(NULL)
  , mpMediaLabel(NULL)
  , mpCommentsLabel(NULL)
  , mpComments(NULL)
  , mpLinksLabel(NULL)
  , mpContentContainer(NULL)
{
    setObjectName(name);
    mpDataMgr = pDataMgr;
    setLayout(&mLayout);

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    // Build the Preview widget
    // Title + duration
    mpTitle = new QLabel(this);
    mpTitle->setObjectName("UBTeacherBarPreviewTitle");
    mpTitle->setWordWrap(true);
    mpTitle->setAlignment(Qt::AlignCenter);
    mpDuration = new QLabel(this);
    mTitleDurationLayout.addWidget(mpTitle, 0);
    mTitleDurationLayout.addWidget(mpDuration, 1);
    mLayout.addLayout(&mTitleDurationLayout, 0);

    mpContentContainer = new UBTBPreviewContainer(this);
    mLayout.addWidget(mpContentContainer, 1);
    //mLayout.addWidget(&mMediaViewer, 1);
    // The next line is disgusting. This is a quickfix that must be reworked later
    mMediaViewer.setContentsMargins(-9, -9, -9, -9);

    hideElements();

    // Edit button
    mpEditButton = new QPushButton(tr("Edit infos"), this);
    mpEditButton->setObjectName("DockPaletteWidgetButton");
    mEditLayout.addStretch(1);
    mEditLayout.addWidget(mpEditButton, 0);
    mEditLayout.addStretch(1);
    mLayout.addLayout(&mEditLayout, 0);


    connect(mpEditButton, SIGNAL(clicked()), this, SLOT(onEdit()));
}

UBTeacherBarPreviewWidget::~UBTeacherBarPreviewWidget()
{
    DELETEPTR(mpLinksLabel);
    DELETEPTR(mpComments);
    DELETEPTR(mpTitle);
    DELETEPTR(mpDuration);
    DELETEPTR(mpActionsLabel);
    DELETEPTR(mpMediaLabel);
    DELETEPTR(mpCommentsLabel);
    DELETEPTR(mpContentContainer);
    DELETEPTR(mpEditButton);
}

void UBTeacherBarPreviewWidget::onEdit()
{
    emit showEditMode();
}

void UBTeacherBarPreviewWidget::setTitle(const QString &title)
{
    if(NULL != mpTitle){
        mpTitle->setText(title);
    }
}

void UBTeacherBarPreviewWidget::setComments(const QString &comments)
{
    if("" != comments){
        mWidgets.clear();
        mpComments->setText(comments);
        mpComments->setVisible(true);
        mpCommentsLabel->setVisible(true);
        mWidgets << mpCommentsLabel;
        mMediaViewer.loadWidgets(mWidgets, false);
        mWidgets.clear();
        mWidgets << mpComments;
        mMediaViewer.loadWidgets(mWidgets, true);
    }
}

void UBTeacherBarPreviewWidget::clean()
{
    mMediaViewer.cleanMedia();

    foreach(QWidget* eachWidget, mStoredWidgets){
        delete eachWidget;
        eachWidget = NULL;
    }
    mStoredWidgets.clear();

    hideElements();
}

void UBTeacherBarPreviewWidget::hideElements()
{
    mpActionsLabel = new QLabel(tr("Actions"), this);
    mpActionsLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mpMediaLabel = new QLabel(tr("Medias"), this);
    mpMediaLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mpCommentsLabel = new QLabel(tr("Comments"), this);
    mpCommentsLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mpComments = new QLabel(this);
    mpComments->setWordWrap(true);
    mpComments->setObjectName("UBTeacherBarPreviewComments");
    mpLinksLabel = new QLabel(tr("Links"), this);
    mpLinksLabel->setObjectName("UBTeacherBarPreviewSubtitle");

    mpActionsLabel->setVisible(false);
    mpMediaLabel->setVisible(false);
    mpCommentsLabel->setVisible(false);
    mpComments->setVisible(false);
    mpLinksLabel->setVisible(false);
}

void UBTeacherBarPreviewWidget::setActions(QStringList actions)
{
    if(!actions.empty()){
        mWidgets.clear();
        mpActionsLabel->setVisible(true);
        mWidgets << mpActionsLabel;
        mediaViewer()->loadWidgets(mWidgets,false);
        mWidgets.clear();
        foreach(QString action, actions){
            QStringList desc = action.split(';');
            if(2 <= desc.size()){
                QString owner = desc.at(0);
                QString act = desc.at(1);
                mpTmpAction = new UBActionPreview(this);
                mpTmpAction->setOwner(owner);
                mpTmpAction->setContent(act);
                mWidgets << mpTmpAction;
            }
        }
        mMediaViewer.loadWidgets(mWidgets, true);
    }
}

void UBTeacherBarPreviewWidget::setLinks(QStringList links)
{
    if(!links.empty()){
        mWidgets.clear();
        mpLinksLabel->setVisible(true);
        mWidgets << mpLinksLabel;
        mMediaViewer.loadWidgets(mWidgets, false);
        mWidgets.clear();
        foreach(QString link, links){
            mpTmpLink = new QLabel(link, this);
            mpTmpLink->setOpenExternalLinks(true);
            mWidgets << mpTmpLink;
        }
        mMediaViewer.loadWidgets(mWidgets, true);
    }
}

void UBTeacherBarPreviewWidget::updateFields()
{

}

void UBTeacherBarPreviewWidget::clearFields()
{

}
