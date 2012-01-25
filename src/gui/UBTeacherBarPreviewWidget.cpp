#include "core/UBApplication.h"
#include "customWidgets/UBGlobals.h"
#include "board/UBBoardController.h"
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


//int UBTeacherBarPreviewMedia::loadMedia(QStringList pMedias)
//{
//    int addedMedia = 0;
////    foreach(QString eachString, pMedias){
////        if(!eachString.isEmpty()){
////            QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(eachString);
////            if(mimeType.contains("image")){
////                UBDraggableLabel* label = new UBDraggableLabel();
////                label->loadImage(eachString);
////                mWidget->addWidget(label);
////                mWidgetList[label]=eachString;
////                addedMedia += 1;
////            }
////            else if(mimeType.contains("video") || mimeType.contains("audio")){
////                UBDraggableMediaPlayer* mediaPlayer = new UBDraggableMediaPlayer();
////                mediaPlayer->setFile(eachString);
////                mWidget->addWidget(mediaPlayer);
////                mWidgetList[mediaPlayer] = eachString;
////                addedMedia += 1;
////            }
////            else{
////                qWarning() << "pMediaPath" << eachString;
////                qWarning() << "bad idea to come here";
////            }
////        }
////    }
//    return addedMedia;
//}

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
    mpContent = new QTextEdit(this);
    mpContent->setReadOnly(true);
    mpContent->setObjectName("UBActionPreviewContent");
    //mpContent->setWordWrap(true);
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

void UBActionPreview::setOwner(int owner)
{
    if(NULL != mpOwner && NULL != mpContent){
        switch(owner){
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
        setMinimumHeight(mpOwner->height() + mpContent->height());
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
UBTBPreviewSeparator::UBTBPreviewSeparator(QWidget *parent, const char *name):QFrame(parent)
{
    setObjectName("UBTBSeparator");
    setMinimumHeight(5);
    setMaximumHeight(5);
}

UBTBPreviewSeparator::~UBTBPreviewSeparator()
{

}

// ------------------------------------------------------------------------------------
UBTeacherBarPreviewWidget::UBTeacherBarPreviewWidget(UBTeacherBarDataMgr* pDataMgr, QWidget *parent, const char *name):QWidget(parent)
  , mpEditButton(NULL)
  , mpSessionTitle(NULL)
  , mpTitle(NULL)
  , mpTitleLabel(NULL)
  , mpPageNbrLabel(NULL)
  , mpContentContainer(NULL)
  , mpScheduleLabel(NULL)
  , mpLicenseLabel(NULL)
{
    setObjectName(name);
    mpDataMgr = pDataMgr;
    setLayout(&mLayout);

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    // Build the Preview widget
    // Session Title
    mTitleContainer.setLayout(&mTitleLayout);
    mpSessionTitle = new QLabel(this);
    mpSessionTitle->setText(tr("Session: "));
    mpSessionTitle->setWordWrap(true);
    mpSessionTitle->setAlignment(Qt::AlignRight);
    mpSessionTitle->setObjectName("UBTBPreviewSessionTitle");
    mLayout.addWidget(mpSessionTitle);

    // Title
    mTitleContainer.setLayout(&mTitleLayout);
    mTitleLayout.setContentsMargins(0, 0, 0, 0);
    mpTitleLabel = new QLabel(&mTitleContainer);
    mpTitleLabel->setText(tr("Activity"));
    mpTitleLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mTitleLayout.addWidget(mpTitleLabel, 0);
    mpTitle = new QLabel(&mTitleContainer);
    mpTitle->setObjectName("UBTeacherBarPreviewTitle");
    mpTitle->setWordWrap(true);
    mpTitle->setAlignment(Qt::AlignLeft);
    mTitleLayout.addWidget(mpTitle, 1);
    mpPageNbrLabel = new QLabel(tr("Page n° "), &mTitleContainer);
    mpPageNbrLabel->setAlignment(Qt::AlignRight);
    mpPageNbrLabel->setObjectName("UBTBPreviewSessionTitle");
    mTitleLayout.addWidget(mpPageNbrLabel);
    mTitleLayout.addWidget(&mTitleSeparator);
    mLayout.addWidget(&mTitleContainer);

    // Content
    mpContentContainer = new UBTBPreviewContainer(this);
    mLayout.addWidget(mpContentContainer, 1);

    // License
    mLayout.addWidget(&mLicenseSeparator);
    mpLicenseLabel = new QLabel(tr("License"), this);
    mpLicenseLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mLayout.addWidget(mpLicenseLabel);
    // TODO : Add the license field here


    // Edit button
    mpEditButton = new QPushButton(tr("Edit infos"), this);
    mpEditButton->setObjectName("DockPaletteWidgetButton");
    mEditLayout.addStretch(1);
    mEditLayout.addWidget(mpEditButton, 0);
    mEditLayout.addStretch(1);
    mLayout.addLayout(&mEditLayout, 0);


    connect(mpEditButton, SIGNAL(clicked()), this, SLOT(onEdit()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));
}

UBTeacherBarPreviewWidget::~UBTeacherBarPreviewWidget()
{
    DELETEPTR(mpEditButton);
    DELETEPTR(mpLicenseLabel);
    DELETEPTR(mpScheduleLabel);
    DELETEPTR(mpPageNbrLabel);
    DELETEPTR(mpTitle);
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpSessionTitle);
}

void UBTeacherBarPreviewWidget::onActiveSceneChanged()
{
    mpPageNbrLabel->setText(tr("Page n° %0").arg(UBApplication::boardController->activeSceneIndex()));
}

void UBTeacherBarPreviewWidget::onEdit()
{
    emit showEditMode();
}

void UBTeacherBarPreviewWidget::updateFields()
{
    // Session Title
    if("" != mpDataMgr->sessionTitle()){
        mpSessionTitle->setText(mpDataMgr->sessionTitle());
        mpSessionTitle->setVisible(true);
    }else{
        mpSessionTitle->setVisible(false);
    }

    // Page Title
    if("" != mpDataMgr->pageTitle()){
        mpTitle->setText(mpDataMgr->pageTitle());
        mpPageNbrLabel->setText(tr("Page n° %0").arg(UBApplication::boardController->activeSceneIndex()));
        mTitleContainer.setVisible(true);
    }else{
        mTitleContainer.setVisible(false);
    }

    // Actions
    generateActions();

    // Media
    //generateMedias();

    // Links
    generateLinks();

    // Comments
    generateComments();

}

void UBTeacherBarPreviewWidget::clearFields()
{
    // Session Title
    mpSessionTitle->setText("");

    // Page Title
    mpTitle->setText("");

    // Medias
    if(!mStoredWidgets.empty()){
        foreach(QWidget* pW, mStoredWidgets){
            mpContentContainer->removeWidget(pW);
            DELETEPTR(pW);
        }
        mStoredWidgets.clear();
    }
}

void UBTeacherBarPreviewWidget::generateActions()
{
    if(!mpDataMgr->actions()->empty()){
        foreach(sAction act, *mpDataMgr->actions()){
            mpTmpAction = new UBActionPreview(this);
            mpTmpAction->setOwner(act.type);
            mpTmpAction->setContent(act.content);
            mpContentContainer->addWidget(mpTmpAction);
            mStoredWidgets << mpTmpAction;
        }
    }
}

void UBTeacherBarPreviewWidget::generateMedias()
{
    if(isVisible()){
        foreach(QString mediaUrl, mpDataMgr->mediaUrls()){
            QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(mediaUrl);
            if(mimeType.contains("image")){
                QPixmap pix = QPixmap(mediaUrl);
                QLabel* label = new QLabel();
                pix.scaledToWidth(label->width());
                label->resize(pix.width(), pix.height());
                label->setPixmap(pix);
                label->setScaledContents(true);
                mStoredWidgets << label;
                mpContentContainer->addWidget(label);
            }
            else if(mimeType.contains("video") || mimeType.contains("audio")){
                UBMediaWidget* mediaPlayer = new UBMediaWidget(mimeType.contains("audio")?eMediaType_Audio:eMediaType_Video);
                mediaPlayer->setFile(mediaUrl);
                mStoredWidgets << mediaPlayer;
                mpContentContainer->addWidget(mediaPlayer);
            }
        }
    }
}

void UBTeacherBarPreviewWidget::generateLinks()
{
    if(!mpDataMgr->urls()->empty()){
        foreach(sLink link, *mpDataMgr->urls()){
            mpTmpLink = new QLabel(QString("<a href='%0'>%1</a>").arg(link.link).arg(link.title), this);
            mpTmpLink->setObjectName("UBLinkPreview");
            mpTmpLink->setOpenExternalLinks(true);
            mpContentContainer->addWidget(mpTmpLink);
            mStoredWidgets << mpTmpLink;
        }
    }
}

void UBTeacherBarPreviewWidget::generateComments()
{
    if("" != mpDataMgr->comments()){
        mpTmpComment = new QTextEdit(this);
        mpTmpComment->setObjectName("UBCommentPreview");
        mpTmpComment->setPlainText(mpDataMgr->comments());
        mpTmpComment->setReadOnly(true);
        mpContentContainer->addWidget(mpTmpComment);
        mStoredWidgets << mpTmpComment;
    }
}

void UBTeacherBarPreviewWidget::showEvent(QShowEvent* ev)
{
    //updateFields();
}

// -----------------------------------------------------------------------------------------------------
UBDraggableMedia::UBDraggableMedia(eMediaType type, QWidget *parent, const char *name):UBMediaWidget(type, parent, name)
{

}

UBDraggableMedia::~UBDraggableMedia()
{

}
