#include "UBTeacherBarWidget.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "UBMainWindow.h"

#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "gui/UBMediaPlayer.h"

#include "frameworks/UBFileSystemUtils.h"

#include "customWidgets/UBDraggableLabel.h"
#include "customWidgets/UBMediaWidget.h"
#include "customWidgets/UBGlobals.h"

#include "core/memcheck.h"

UBTeacherBarWidget::UBTeacherBarWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
    , mpLayout(NULL)
    , mpTitleLayout(NULL)
    , mpTitleLabel(NULL)
    , mpTitle(NULL)
    , mpMediaLabel(NULL)
    , mpContainer(NULL)
    , mpContainerLayout(NULL)
    , mpActionLabel(NULL)
    , mpActions(NULL)
    , mpActionButton(NULL)
    , mpActionLayout(NULL)
    , mpCommentLabel(NULL)
    , mpComments(NULL)
    , mpLinkLabel(NULL)
    , mpLinks(NULL)
    , mpLinkButton(NULL)
    , mpLinkLayout(NULL)
    , mpStackWidget(NULL)
    , mpPreview(NULL)
    , mpMediaContainer(NULL)
    , mpDocPreviewWidget(NULL)
    , mpDocEditWidget(NULL)
{
    setObjectName(name);
    mName = "TeacherBarWidget";
    mVisibleState = true;
    mActionList.clear();
    mUrlList.clear();

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    mIconToLeft = QPixmap(":images/teacher_open.png");
    mIconToRight = QPixmap(":images/teacher_close.png");

    // Create the GUI
    mpContainerLayout = new QVBoxLayout(this);
    setLayout(mpContainerLayout);

    mpContainer = new QWidget(this);
    mpContainer->setObjectName("DockPaletteWidgetBox");

    mpPreview = new UBTeacherBarPreviewWidget(this);
    mpDocPreviewWidget = new UBTBDocumentPreviewWidget(this);
    mpDocEditWidget = new UBTBDocumentEditWidget(this);

    mpStackWidget = new QStackedWidget(this);
    mpContainerLayout->addWidget(mpStackWidget);
    mpStackWidget->addWidget(mpContainer);
    mpStackWidget->addWidget(mpPreview);
    mpStackWidget->addWidget(mpDocPreviewWidget);
    mpStackWidget->addWidget(mpDocEditWidget);

    mpLayout = new QVBoxLayout(mpContainer);
    mpContainer->setLayout(mpLayout);

    // Title
    mpTitleLabel = new QLabel(tr("Title"), mpContainer);
    mpTitle = new QLineEdit(mpContainer);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    connect(mpTitle, SIGNAL(textChanged(const QString&)), this, SLOT(onTitleTextChanged(const QString&)));
    mpLayout->addWidget(mpTitleLabel, 0);
    mpLayout->addWidget(mpTitle, 0);

    // Actions
    mpActionLabel = new QLabel(tr("Actions"), mpContainer);
    mpLayout->addWidget(mpActionLabel, 0);
    mpActions = new UBWidgetList(mpContainer);
    mpActions->setEmptyText(tr("Add actions"));
    mpLayout->addWidget(mpActions, 1);
    mpActionButton = new QPushButton(mpContainer);
    mpActionButton->setObjectName("DockPaletteWidgetButton");
    mpActionButton->setText(tr("Add action"));
    mpActionLayout = new QHBoxLayout();
    mpActionLayout->addWidget(mpActionButton, 0);
    mpActionLayout->addStretch(1);
    mpLayout->addLayout(mpActionLayout, 0);

    // Media
    mpMediaLabel = new QLabel(tr("Medias"), mpContainer);
    mpLayout->addWidget(mpMediaLabel, 0);
    mpMediaContainer = new UBTBMediaContainer(this);
    mpMediaContainer->setEmptyText(tr("Drop media here"));
    mpLayout->addWidget(mpMediaContainer, 1);

    // Links
    mpLinkLabel = new QLabel(tr("Links"), mpContainer);
    mpLayout->addWidget(mpLinkLabel, 0);
    mpLinks = new UBWidgetList(mpContainer);
    mpLayout->addWidget(mpLinks, 1);
    mpLinkButton = new QPushButton(tr("Add link"), mpContainer);
    mpLinkButton->setObjectName("DockPaletteWidgetButton");
    mpLinkLayout = new QHBoxLayout();
    mpLinkLayout->addWidget(mpLinkButton, 0);
    mpLinkLayout->addStretch(1);
    mpLayout->addLayout(mpLinkLayout, 0);

    // Comments
    mpCommentLabel = new QLabel(tr("Comments"), mpContainer);
    mpLayout->addWidget(mpCommentLabel, 0);
    mpComments = new QTextEdit(this);
    mpComments->setObjectName("DockPaletteWidgetBox");
    mpComments->setStyleSheet("background:white;");
    mpLayout->addWidget(mpComments, 1);

    connect(UBApplication::boardController, SIGNAL(activeSceneWillChange()), this, SLOT(saveContent()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(loadContent()));
    connect(UBApplication::mainWindow->actionQuit, SIGNAL(triggered()), this, SLOT(saveContent()));
    connect(mpTitle, SIGNAL(textChanged(QString)), this, SLOT(onValueChanged()));
    connect(mpActionButton, SIGNAL(clicked()), this, SLOT(onActionButton()));
    connect(mpLinkButton, SIGNAL(clicked()), this, SLOT(onLinkButton()));
    connect(mpPreview, SIGNAL(showEditMode()), this, SLOT(onShowEditMode()));
    connect(mpMediaContainer, SIGNAL(mediaDropped(QString)), this, SLOT(onMediaDropped(QString)));
    connect(mpDocPreviewWidget, SIGNAL(onEditClicked()), this, SLOT(onTBStateChanged(eTeacherBarState)));
    connect(mpDocPreviewWidget, SIGNAL(onPageViewClicked()), this, SLOT(onTBStateChanged(eTeacherBarState)));
    connect(mpDocEditWidget, SIGNAL(onPageViewClicked()), this, SLOT(onTBStateChanged(eTeacherBarState)));
    connect(mpDocEditWidget, SIGNAL(onPreviewClicked()), this, SLOT(onTBStateChanged(eTeacherBarState)));
}

UBTeacherBarWidget::~UBTeacherBarWidget()
{
    DELETEPTR(mpDocPreviewWidget);
    DELETEPTR(mpDocEditWidget);
    DELETEPTR(mpMediaContainer);
    DELETEPTR(mpComments);
    DELETEPTR(mpCommentLabel);
    DELETEPTR(mpLinks);
    DELETEPTR(mpLinkLabel);
    DELETEPTR(mpLinkButton);
    DELETEPTR(mpLinkLayout);
    DELETEPTR(mpMediaLabel);
    DELETEPTR(mpActionButton);
    DELETEPTR(mpActionLayout);
    DELETEPTR(mpActionLabel);
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpTitle);
    DELETEPTR(mpTitleLayout);
    DELETEPTR(mpLayout);
    DELETEPTR(mpContainer);
    DELETEPTR(mpContainerLayout);
    DELETEPTR(mpPreview);
    DELETEPTR(mpStackWidget);
}

void UBTeacherBarWidget::onValueChanged()
{
    if(isEmpty())
    {
        mIconToLeft = QPixmap(":images/teacher_open_disabled.png");
        mIconToRight = QPixmap(":images/teacher_close_disabled.png");
    }
    else
    {
        mIconToLeft = QPixmap(":images/teacher_open.png");
        mIconToRight = QPixmap(":images/teacher_close.png");
    }

    UBApplication::boardController->paletteManager()->refreshPalettes();
}

void UBTeacherBarWidget::saveContent()
{
    sTeacherBarInfos infos;
    // Title
    infos.title = mpTitle->text();

    // Actions
    for(int i=0; i<mActionList.size(); i++){
        infos.actions << QString("%0;%1").arg(mActionList.at(i)->comboValue()).arg(mActionList.at(i)->text());
    }
    // Media
    foreach(QString media, mpMediaContainer->mediaUrls()){
        infos.medias << media;
    }

    // Links
    for(int j=0; j<mUrlList.size(); j++){
        if("" != mUrlList.at(j)->url()){
            infos.urls << mUrlList.at(j)->url();
        }
    }
    // Comments
    infos.comments = mpComments->document()->toPlainText();

    UBPersistenceManager::persistenceManager()->persistTeacherBar(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex(), infos);
}

void UBTeacherBarWidget::loadContent()
{
    clearWidgetLists();
    sTeacherBarInfos nextInfos = UBPersistenceManager::persistenceManager()->getTeacherBarInfos(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex());
    // Title
    mpTitle->setText(nextInfos.title);

    // Actions
    for(int i=0; i<nextInfos.actions.size(); i++){
        QStringList qslAction = nextInfos.actions.at(i).split(";");
        if(qslAction.size() >= 2){
            UBTeacherStudentAction* pAction = new UBTeacherStudentAction(mpContainer);
            pAction->setComboValue(qslAction.at(0).toInt());
            pAction->setText(qslAction.at(1));
            mActionList << pAction;
            mpActions->addWidget(pAction);
        }
    }
    // Media
    foreach(QString url, nextInfos.medias){
        if("" != url){
            QWidget* pMedia = mpMediaContainer->generateMediaWidget(url);
            if(NULL != pMedia){
                mMediaList << pMedia;
                mpMediaContainer->addWidget(pMedia);
            }
        }
    }

    // Links
    for(int j=0; j<nextInfos.urls.size(); j++){
        QString qsUrl = nextInfos.urls.at(j);
        if("" != qsUrl){
            UBUrlWidget* pLink = new UBUrlWidget(mpContainer);
            pLink->setUrl(qsUrl);
            mUrlList << pLink;
            mpLinks->addWidget(pLink);
        }
    }
    // Comments
    if(NULL != mpComments){
        mpComments->document()->setPlainText(nextInfos.comments);
    }

    mpPreview->mediaViewer()->cleanMedia();

    if(!isEmpty()){
        // Update the fields of the preview widget
        mpPreview->setTitle(mpTitle->text());
        mpPreview->mediaViewer()->loadMedia(nextInfos.medias);

        mpStackWidget->setCurrentWidget(mpPreview);
        mpPreview->clean();

        // Add the actions
        if(!mActionList.empty()){
            QStringList actions;
            foreach(UBTeacherStudentAction* action, mActionList){
                QString desc = QString("%0;%1").arg(action->comboValue()).arg(action->text());
                actions << desc;
            }
            mpPreview->setActions(actions);
        }

        // Add the media
        if(nextInfos.medias.count() > 0){
            QList<QWidget*> widgetList;
            widgetList.append(mpPreview->mediaLabel());
            mpPreview->mediaViewer()->loadWidgets(widgetList,false);
            int loadedMedia = mpPreview->mediaViewer()->loadMedia(nextInfos.medias);
            if(loadedMedia)
                    mpPreview->mediaLabel()->setVisible(true);
        }


        // Add the links
        if(!mUrlList.empty()){
            QStringList links;
            foreach(UBUrlWidget* url, mUrlList){
                QStringList list = url->url().split(";");
                QString formedlink = "<a href=";
                if(!list.at(0).startsWith("http://"))
                    formedlink += "http://";
                formedlink += list.at(0) + ">" + list.at(1) + "</a>";
                links << formedlink;
            }
            mpPreview->setLinks(links);
        }

        // Add the comments
        mpPreview->setComments(mpComments->document()->toPlainText());
    }
    else{
        mpStackWidget->setCurrentWidget(mpContainer);
    }


}

bool UBTeacherBarWidget::isEmpty()
{
    return  mpTitle->text() == "" &&
            mpLinks->empty() &&
            mpActions->empty() &&
            mpMediaContainer->empty() &&
            mpComments->document()->toPlainText() == "";
}

void UBTeacherBarWidget::onTitleTextChanged(const QString& text)
{
	mpTitle->setToolTip(text);
}

void UBTeacherBarWidget::onActionButton()
{
    UBTeacherStudentAction* pAction = new UBTeacherStudentAction(mpContainer);
    mActionList << pAction;
    mpActions->addWidget(pAction);
}

void UBTeacherBarWidget::onLinkButton()
{
    UBUrlWidget* pUrl = new UBUrlWidget(mpContainer);
    mUrlList << pUrl;
    mpLinks->addWidget(pUrl);
}

void UBTeacherBarWidget::clearWidgetLists()
{
    if(NULL != mpMediaContainer){
        for(int i=0; i<mMediaList.size(); i++){
            mpMediaContainer->removeWidget(mMediaList.at(i));
            delete mMediaList.at(i);
        }
        mMediaList.clear();
        mpMediaContainer->cleanMedias();
    }

    if(NULL != mpActions){
        for(int i=0; i<mActionList.size(); i++){
            mpActions->removeWidget(mActionList.at(i));
            delete mActionList.at(i);
        }
        mActionList.clear();
    }

    if(NULL != mpLinks){
        for(int i=0; i<mUrlList.size(); i++){
            mpLinks->removeWidget(mUrlList.at(i));
            delete mUrlList.at(i);
        }
        mUrlList.clear();
    }
}

void UBTeacherBarWidget::onShowEditMode()
{
    mpStackWidget->setCurrentWidget(mpContainer);
}

void UBTeacherBarWidget::onMediaDropped(const QString &url)
{
    if("" != url){
        QWidget* pMedia = mpMediaContainer->generateMediaWidget(url);
        if(NULL != pMedia){
            mMediaList << pMedia;
            mpMediaContainer->addWidget(pMedia);
        }
    }
}

void UBTeacherBarWidget::onTBStateChanged(eTeacherBarState state)
{
    switch(state){
    case eTeacherBarState_DocumentEdit:
        mpStackWidget->setCurrentWidget(mpDocEditWidget);
        break;
    case eTeacherBarState_DocumentPreview:
        mpStackWidget->setCurrentWidget(mpDocPreviewWidget);
        break;
    case eTeacherBarState_PageEdit:
        mpStackWidget->setCurrentWidget(mpContainer);
        break;
    case eTeacherBarState_PagePreview:
        mpStackWidget->setCurrentWidget(mpPreview);
        break;
    }
}

// ---------------------------------------------------------------------------------------------
UBTeacherStudentAction::UBTeacherStudentAction(QWidget *parent, const char *name):QWidget(parent)
  , mpText(NULL)
  , mpLayout(NULL)
  , mpComboLayout(NULL)
  , mpCombo(NULL)
{
    setObjectName(name);

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    // Create the GUI
    mpLayout = new QHBoxLayout(this);
    setLayout(mpLayout);

    mpComboLayout = new QVBoxLayout();

    mpCombo = new QComboBox(this);
    mpCombo->setObjectName("DockPaletteWidgetComboBox");
    mpCombo->setMinimumWidth(80);
    mpCombo->addItem(tr("Teacher"));
    mpCombo->addItem(tr("Student"));
    mpComboLayout->addWidget(mpCombo, 0);
    mpComboLayout->addStretch(1);

    mpLayout->addLayout(mpComboLayout, 0);

    mpText = new QTextEdit(this);
    mpText->setObjectName("DockPaletteWidgetBox");
    mpText->setStyleSheet("background:white;");

    mpLayout->addWidget(mpText, 1);

}

UBTeacherStudentAction::~UBTeacherStudentAction()
{
    DELETEPTR(mpCombo);
    DELETEPTR(mpText);
    DELETEPTR(mpComboLayout);
    DELETEPTR(mpLayout);
}

QString UBTeacherStudentAction::text()
{
    QString str;
    if(NULL != mpText){
        str = mpText->document()->toPlainText();
    }
    return str;
}

QString UBTeacherStudentAction::comboValue()
{
    QString str;

    if(NULL != mpCombo){
        str = QString("%0").arg(mpCombo->currentIndex());
    }

    return str;
}

void UBTeacherStudentAction::setComboValue(int value)
{
    if(NULL != mpCombo){
        mpCombo->setCurrentIndex(value);
    }
}

void UBTeacherStudentAction::setText(const QString& text)
{
    if(NULL != mpText){
        mpText->document()->setPlainText(text);
    }
}

// ---------------------------------------------------------------------------------------------
UBUrlWidget::UBUrlWidget(QWidget *parent, const char *name):QWidget(parent)
  , mpLayout(NULL)
  , mpUrlLabel(NULL)
  , mpUrl(NULL)
{
    setObjectName(name);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    mpLabelLayout = new QHBoxLayout(this);
    mpUrlLabel = new QLabel(tr("Url"), this);
    mpLabelLayout->addWidget(mpUrlLabel, 0);
    mpUrl = new QLineEdit(this);
    mpUrl->setObjectName("DockPaletteWidgetLineEdit");
    mpUrl->setMinimumHeight(20);
    mpLabelLayout->addWidget(mpUrl, 1);

    mpTitleLayout = new QHBoxLayout(this);
    mpTitleLabel = new QLabel(tr("Title"),this);
    mpTitleLayout->addWidget(mpTitleLabel,0);
    mpTitle = new QLineEdit(this);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    mpTitle->setMinimumHeight(20);
    mpTitleLayout->addWidget(mpTitle,1);

    mpLayout->addLayout(mpTitleLayout);
    mpLayout->addLayout(mpLabelLayout);
}

UBUrlWidget::~UBUrlWidget()
{
    DELETEPTR(mpTitle);
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpUrlLabel);
    DELETEPTR(mpUrl);
    DELETEPTR(mpTitleLayout);
    DELETEPTR(mpLabelLayout);
    DELETEPTR(mpLayout);
}

QString UBUrlWidget::url()
{
    QString str;

    if(NULL != mpUrl){
        str = mpUrl->text() + ";" + mpTitle->text();
    }

    return str;
}

void UBUrlWidget::setUrl(const QString &url)
{
    QStringList list = url.split(";");
    if(NULL != mpUrl){
        mpUrl->setText(list.at(0));
        mpTitle->setText(list.at(1));
    }
}

// ------------------------------------------------------------------------------------
UBTeacherBarPreviewWidget::UBTeacherBarPreviewWidget(QWidget *parent, const char *name):QWidget(parent)
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

void UBTeacherBarPreviewWidget::setDuration(eDuration duration)
{
    if(NULL != mpDuration){
        QPixmap p;
        switch(duration){
            case eDuration_Quarter:
                p = QPixmap(":images/duration1.png");
                break;
            case eDuration_Half:
                p = QPixmap(":images/duration2.png");
                break;
            case eDuration_ThreeQuarter:
                p = QPixmap(":images/duration3.png");
                break;
            default:
                break;
        }
        mpDuration->setPixmap(p.scaledToHeight(16, Qt::SmoothTransformation));
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

// ------------------------------------------------------------------------------------
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
    foreach(QString eachString, pMedias){
        if(!eachString.isEmpty()){
            QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(eachString);
            if(mimeType.contains("image")){
                UBDraggableLabel* label = new UBDraggableLabel();
                label->loadImage(eachString);
                mWidget->addWidget(label);
                mWidgetList[label]=eachString;
                addedMedia += 1;
            }
            else if(mimeType.contains("video") || mimeType.contains("audio")){
                UBDraggableMediaPlayer* mediaPlayer = new UBDraggableMediaPlayer();
                mediaPlayer->setFile(eachString);
                mWidget->addWidget(mediaPlayer);
                mWidgetList[mediaPlayer] = eachString;
                addedMedia += 1;
            }
            else{
                qWarning() << "pMediaPath" << eachString;
                qWarning() << "bad idea to come here";
            }
        }
    }
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

//  ------------------------------------------------------------------------------------------------------------------------------------
UBTBMediaContainer::UBTBMediaContainer(QWidget *parent, const char *name) : UBWidgetList(parent)
{
    setObjectName(name);
    setAcceptDrops(true);
}

UBTBMediaContainer::~UBTBMediaContainer()
{

}

void UBTBMediaContainer::dropEvent(QDropEvent* pEvent)
{
    QPixmap pixFromDropEvent;
    QString mimeType;
    QString resourcePath;
    if(pEvent->mimeData()->hasText()){
        resourcePath = pEvent->mimeData()->text();
    }
    else if(pEvent->mimeData()->hasUrls()){
        resourcePath = pEvent->mimeData()->urls().at(0).toLocalFile();
    }
    else if(pEvent->mimeData()->hasImage()){
        pixFromDropEvent.loadFromData(pEvent->mimeData()->imageData().toByteArray());
        if(!pixFromDropEvent.isNull())
            mimeType = "image";
    }

    if (mimeType.isEmpty() && resourcePath.isEmpty()){
        pEvent->acceptProposedAction();
        return;
    }
    if(!resourcePath.isEmpty()){
        emit mediaDropped(resourcePath);
        pEvent->acceptProposedAction();
    }
}

void UBTBMediaContainer::dragEnterEvent(QDragEnterEvent* pEvent)
{
    pEvent->acceptProposedAction();
}

void UBTBMediaContainer::dragMoveEvent(QDragMoveEvent* pEvent)
{
    pEvent->acceptProposedAction();
}

void UBTBMediaContainer::dragLeaveEvent(QDragLeaveEvent* pEvent)
{
    pEvent->accept();
}

void UBTBMediaContainer::addMedia(const QString& mediaPath)
{
    if(!mediaPath.isEmpty())
        mMediaList.append(mediaPath);
    else
        qWarning() << __FUNCTION__ <<  "empty path";

    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(mediaPath);
    if(mimeType.contains("image")){
        QPixmap pix = QPixmap(mediaPath);
        QLabel* label = new QLabel();
        label->setPixmap(pix);
        label->setScaledContents(true);
        addWidget(label);
    }
    else if(mimeType.contains("video") || mimeType.contains("audio")){
        UBMediaPlayer* mediaPlayer = new UBMediaPlayer();
        mediaPlayer->setFile(mediaPath);
        addWidget(mediaPlayer);
    }
    else{
        qWarning() << "pMediaPath" << mediaPath;
        qWarning() << "bad idea to come here";
    }
}

QStringList UBTBMediaContainer::mediaUrls()
{
    return mMediaList;
}

void UBTBMediaContainer::cleanMedias()
{
    mMediaList.clear();
}

QWidget* UBTBMediaContainer::generateMediaWidget(const QString& url)
{
    QWidget* pW = NULL;

    if(!url.isEmpty())
        mMediaList.append(url);
    else
        qWarning() << __FUNCTION__ <<  "empty path";

    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(url);
    if(mimeType.contains("image")){
        QPixmap pix = QPixmap(url);
        QLabel* label = new QLabel();
        pix.scaledToWidth(label->width());
        label->resize(pix.width(), pix.height());
        label->setPixmap(pix);
        label->setScaledContents(true);
        pW = label;
    }
    else if(mimeType.contains("video") || mimeType.contains("audio")){
        UBMediaWidget* mediaPlayer = new UBMediaWidget(mimeType.contains("audio")?eMediaType_Audio:eMediaType_Video);
        mediaPlayer->setFile(url);
        pW = mediaPlayer;
    }
    else{
        qWarning() << "pMediaPath" << url;
        qWarning() << "bad idea to come here";
    }

    return pW;
}

// -------------------------------------------------------------------------------------------------------------------
UBTBPreviewContainer::UBTBPreviewContainer(QWidget *parent, const char *name):UBWidgetList(parent)
{
    setObjectName(name);
}

UBTBPreviewContainer::~UBTBPreviewContainer()
{

}

// -------------------------------------------------------------------------------------------------------------------
UBTBDocumentEditWidget::UBTBDocumentEditWidget(QWidget *parent, const char *name):QWidget(parent)
  , mpPageViewButton(NULL)
  , mpPreviewButton(NULL)
  , mpTitleLabel(NULL)
  , mpTitle(NULL)
  , mpTargetLabel(NULL)
  , mpTarget(NULL)
  , mpMetadataLabel(NULL)
  , mpLicenseLabel(NULL)
{
    setObjectName(name);
    setLayout(&mLayout);

    mpPageViewButton = new QPushButton(tr("Page View"), this);
    mPageLayout.addStretch(1);
    mPageLayout.addWidget(mpPageViewButton, 0);
    mPageLayout.addStretch(1);
    mLayout.addLayout(&mPageLayout);

    mpTitleLabel = new QLabel(tr("Session Title:"), this);
    mpTitleLabel->setAlignment(Qt::AlignLeft);
    mLayout.addWidget(mpTitleLabel);
    mpTitle = new QLineEdit(this);
    mLayout.addWidget(mpTitle);


    mpPreviewButton = new QPushButton(tr("Preview"), this);
    mPreviewLayout.addStretch(1);
    mPreviewLayout.addWidget(mpPreviewButton, 0);
    mPreviewLayout.addStretch(1);
    mLayout.addLayout(&mPreviewLayout);

    connect(mpPageViewButton, SIGNAL(clicked()), this, SIGNAL(onPageViewClicked()));
    connect(mpPreviewButton, SIGNAL(clicked()), this, SIGNAL(onPreviewClicked()));
}

UBTBDocumentEditWidget::~UBTBDocumentEditWidget()
{
    DELETEPTR(mpTitleLabel);
    DELETEPTR(mpTitle);
    DELETEPTR(mpTargetLabel);
    DELETEPTR(mpTarget);
    DELETEPTR(mpMetadataLabel);
    DELETEPTR(mpLicenseLabel);
    DELETEPTR(mpPageViewButton);
    DELETEPTR(mpPreviewButton);
}

// -------------------------------------------------------------------------------------------------------------------
UBTBDocumentPreviewWidget::UBTBDocumentPreviewWidget(QWidget *parent, const char *name):QWidget(parent)
  , mpPageViewButton(NULL)
  , mpEditButton(NULL)
{
    setObjectName(name);
    setLayout(&mLayout);

    mpPageViewButton = new QPushButton(tr("Page View"), this);
    mPageLayout.addStretch(1);
    mPageLayout.addWidget(mpPageViewButton, 0);
    mPageLayout.addStretch(1);
    mLayout.addLayout(&mPageLayout);

    // TODO : Add the elements here

    mpEditButton = new QPushButton(tr("Edit"), this);
    mPreviewLayout.addStretch(1);
    mPreviewLayout.addWidget(mpEditButton, 0);
    mPreviewLayout.addStretch(1);
    mLayout.addLayout(&mPreviewLayout);

    connect(mpPageViewButton, SIGNAL(clicked()), this, SIGNAL(onPageViewClicked()));
    connect(mpEditButton, SIGNAL(clicked()), this, SIGNAL(onEditClicked()));
}

UBTBDocumentPreviewWidget::~UBTBDocumentPreviewWidget()
{
    DELETEPTR(mpPageViewButton);
    DELETEPTR(mpEditButton);
}


