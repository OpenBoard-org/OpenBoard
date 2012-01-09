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

#include "core/memcheck.h"

UBTeacherBarWidget::UBTeacherBarWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
    , mpLayout(NULL)
    , mpTitleLayout(NULL)
    , mpDurationLayout(NULL)
    , mpTitleLabel(NULL)
    , mpDurationLabel(NULL)
    , mpTitle(NULL)
    , mpMediaLabel(NULL)
    , mpDropMediaZone(NULL)
    , mpContainer(NULL)
    , mpContainerLayout(NULL)
    , mpDuration1(NULL)
    , mpDuration2(NULL)
    , mpDuration3(NULL)
    , mpDurationButtons(NULL)
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

    mpStackWidget = new QStackedWidget(this);
    mpContainerLayout->addWidget(mpStackWidget);
    mpStackWidget->addWidget(mpContainer);
    mpStackWidget->addWidget(mpPreview);

    mpLayout = new QVBoxLayout(mpContainer);
    mpContainer->setLayout(mpLayout);

    // Title
    mpTitleLabel = new QLabel(tr("Title"), mpContainer);
    mpTitle = new QLineEdit(mpContainer);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    connect(mpTitle, SIGNAL(textChanged(const QString&)), this, SLOT(onTitleTextChanged(const QString&)));
    mpLayout->addWidget(mpTitleLabel, 0);
    mpLayout->addWidget(mpTitle, 0);

    // Duration
    mpDurationLabel = new QLabel(tr("Duration"), mpContainer);
    mpLayout->addWidget(mpDurationLabel, 0);
    mpDurationLayout = new QHBoxLayout();
    mpDuration1 = new QCheckBox(this);
    mpDuration1->setIcon(QIcon(":images/duration1.png"));
    mpDuration1->setChecked(true);
    mpDurationLayout->addWidget(mpDuration1, 0);
    mpDuration2 = new QCheckBox(this);
    mpDuration2->setIcon(QIcon(":images/duration2.png"));
    mpDurationLayout->addWidget(mpDuration2, 0);
    mpDuration3 = new QCheckBox(this);
    mpDuration3->setIcon(QIcon(":images/duration3.png"));
    mpDurationLayout->addWidget(mpDuration3, 0);
    mpDurationButtons = new QButtonGroup(mpContainer);
    mpDurationButtons->addButton(mpDuration1);
    mpDurationButtons->addButton(mpDuration2);
    mpDurationButtons->addButton(mpDuration3);
    mpLayout->addLayout(mpDurationLayout, 0);

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
    mpMediaLabel = new QLabel(tr("Media"), mpContainer);
    mpLayout->addWidget(mpMediaLabel, 0);
    mpDropMediaZone = new UBTeacherBarDropMediaZone(mpContainer);
    mpLayout->addWidget(mpDropMediaZone, 1);

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
}

UBTeacherBarWidget::~UBTeacherBarWidget()
{
    if(NULL != mpComments){
        delete mpComments;
        mpComments = NULL;
    }
    if(NULL != mpCommentLabel){
        delete mpCommentLabel;
        mpCommentLabel = NULL;
    }
    if(NULL != mpLinks){
        delete mpLinks;
        mpLinks = NULL;
    }
    if(NULL != mpLinkLabel){
        delete mpLinkLabel;
        mpLinkLabel = NULL;
    }
    if(NULL != mpLinkButton){
        delete mpLinkButton;
        mpLinkButton = NULL;
    }
    if(NULL != mpLinkLayout){
        delete mpLinkLayout;
        mpLinkLayout = NULL;
    }
    if(NULL != mpDropMediaZone){
        delete mpDropMediaZone;
        mpDropMediaZone = NULL;
    }
    if(NULL != mpMediaLabel){
        delete mpMediaLabel;
        mpMediaLabel = NULL;
    }
    if(NULL != mpActionButton){
        delete mpActionButton;
        mpActionButton = NULL;
    }
    if(NULL != mpActionLayout){
        delete mpActionLayout;
        mpActionLayout = NULL;
    }
    if(NULL != mpActionLabel){
        delete mpActionLabel;
        mpActionLabel = NULL;
    }
    if(NULL != mpDurationLabel){
        delete mpDurationLabel;
        mpDurationLabel = NULL;
    }
    if(NULL != mpDuration1){
        delete mpDuration1;
        mpDuration1 = NULL;
    }
    if(NULL != mpDuration2){
        delete mpDuration2;
        mpDuration2 = NULL;
    }
    if(NULL != mpDuration3){
        delete mpDuration3;
        mpDuration3 = NULL;
    }
    if(NULL != mpDurationButtons){
        delete mpDurationButtons;
        mpDurationButtons = NULL;
    }
    if(NULL != mpDurationLayout){
        delete mpDurationLayout;
        mpDurationLayout = NULL;
    }
    if(NULL != mpTitleLabel){
        delete mpTitleLabel;
        mpTitleLabel = NULL;
    }
    if(NULL != mpTitle){
        delete mpTitle;
        mpTitle = NULL;
    }
    if(NULL != mpTitleLayout){
        delete mpTitleLayout;
        mpTitleLayout = NULL;
    }
    if(NULL != mpLayout){
        delete mpLayout;
        mpLayout = NULL;
    }
    if(NULL != mpContainer){
        delete mpContainer;
        mpContainer = NULL;
    }
    if(NULL != mpContainerLayout){
        delete mpContainerLayout;
        mpContainerLayout = NULL;
    }
    if(NULL != mpPreview){
        delete mpPreview;
        mpPreview = NULL;
    }
    if(NULL != mpStackWidget){
        delete mpStackWidget;
        mpStackWidget = NULL;
    }
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
    // Duration
    if(mpDuration1->isChecked()){
        infos.Duration = eDuration_Quarter;
    }else if(mpDuration2->isChecked()){
        infos.Duration = eDuration_Half;
    }else{
        infos.Duration = eDuration_ThreeQuarter;
    }
    // Actions
    for(int i=0; i<mActionList.size(); i++){
        infos.actions << QString("%0;%1").arg(mActionList.at(i)->comboValue()).arg(mActionList.at(i)->text());
    }
    // Media
    // TODO :   Get the url of the dropped medias and store them in infos.medias
     infos.medias = mpDropMediaZone->mediaList();

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
    // Duration
    switch(nextInfos.Duration){
        case eDuration_Quarter: mpDuration1->setChecked(true);
            break;
        case eDuration_Half: mpDuration2->setChecked(true);
            break;
        case eDuration_ThreeQuarter: mpDuration3->setChecked(true);
            break;
        default: mpDuration1->setChecked(true);
            break;
    }
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
    mpDropMediaZone->reloadMedia(nextInfos.medias);

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
        if(mpDuration1->isChecked()){
            mpPreview->setDuration(eDuration_Quarter);
        }else if(mpDuration2->isChecked()){
            mpPreview->setDuration(eDuration_Half);
        }else{
            mpPreview->setDuration(eDuration_ThreeQuarter);
        }
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
        mpPreview->mediaViewer()->loadMedia(nextInfos.medias);

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
            mpDropMediaZone->empty() &&
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
    mpDropMediaZone->cleanMedias();

    if(NULL != mpActions){
        for(int i=0; i<mActionList.size(); i++){
            mpActions->removeWidget(mActionList.at(i));
        }
        mActionList.clear();
    }

    if(NULL != mpLinks){
        for(int i=0; i<mUrlList.size(); i++){
            mpLinks->removeWidget(mUrlList.at(i));
        }
        mUrlList.clear();
    }
}

void UBTeacherBarWidget::onShowEditMode()
{
    mpStackWidget->setCurrentWidget(mpContainer);
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
    if(NULL != mpCombo){
        delete mpCombo;
        mpCombo = NULL;
    }
    if(NULL != mpText){
        delete mpText;
        mpText = NULL;
    }
    if(NULL != mpComboLayout){
        delete mpComboLayout;
        mpComboLayout = NULL;
    }
    if(NULL != mpLayout){
        delete mpLayout;
        mpLayout = NULL;
    }
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
UBTeacherBarDropMediaZone::UBTeacherBarDropMediaZone(QWidget *parent, const char *name):QWidget(parent)

{
    setObjectName(name);
    setAcceptDrops(true);
    mWidget = new UBWidgetList(parent);
    mWidget->setEmptyText(tr("Drag media here ..."));
    mLayout.addWidget(mWidget);
    setLayout(&mLayout);
    // The next line is disgusting. This is a quickfix that must be reworked later
    setContentsMargins(-9, -9, -9, -9);
}

UBTeacherBarDropMediaZone::~UBTeacherBarDropMediaZone()
{
    cleanMedias();
    if(mWidget){
        delete mWidget;
        mWidget = NULL;
    }
}

void UBTeacherBarDropMediaZone::cleanMedias()
{
    foreach(QWidget* eachWidget,mWidgetList){
        mWidget->removeWidget(eachWidget);
        delete eachWidget;
    }

    mWidgetList.clear();

    mMediaList.clear();
}

bool UBTeacherBarDropMediaZone::empty()
{
    return mWidget->empty();
}

void UBTeacherBarDropMediaZone::dragEnterEvent(QDragEnterEvent *pEvent)
{
    pEvent->acceptProposedAction();
}

void UBTeacherBarDropMediaZone::dragLeaveEvent(QDragLeaveEvent *pEvent)
{
    pEvent->accept();
}


void UBTeacherBarDropMediaZone::addMedia(QString pMediaPath)
{
    if(!pMediaPath.isEmpty())
        mMediaList.append(pMediaPath);
    else
        qWarning() << __FUNCTION__ <<  "empty path";

    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(pMediaPath);
    if(mimeType.contains("image")){
        QPixmap pix = QPixmap(pMediaPath);
        QLabel* label = new QLabel();
        label->setPixmap(pix);
        label->setScaledContents(true);
        mWidget->addWidget(label);
        mWidgetList << label;
    }
    else if(mimeType.contains("video") || mimeType.contains("audio")){
        UBMediaPlayer* mediaPlayer = new UBMediaPlayer();
        mediaPlayer->setFile(pMediaPath);
        mWidget->addWidget(mediaPlayer);
        mWidgetList << mediaPlayer;
    }
    else{
        qWarning() << "pMediaPath" << pMediaPath;
        qWarning() << "bad idea to come here";
    }

}

void UBTeacherBarDropMediaZone::reloadMedia(QStringList pList)
{
    cleanMedias();
    foreach(QString eachString, pList){
        addMedia(eachString);
    }
}

void UBTeacherBarDropMediaZone::dropEvent(QDropEvent *pEvent)
{
    QPixmap pixFromDropEvent;
    QString mimeType;
    QString resourcePath;
    if(pEvent->mimeData()->hasText()){
        qDebug() << "pEvent->mimeData()->hasText()" << pEvent->mimeData()->text();
        resourcePath = pEvent->mimeData()->text();
    }
    else if(pEvent->mimeData()->hasUrls()){
        qDebug() << "pEvent->mimeData()->hasUrls()" << pEvent->mimeData()->urls().at(0);
        resourcePath = pEvent->mimeData()->urls().at(0).toLocalFile();
    }
    else if(pEvent->mimeData()->hasImage()){
        qDebug() << "pEvent->mimeData()->hasImage()";
        pixFromDropEvent.loadFromData(pEvent->mimeData()->imageData().toByteArray());
        if(!pixFromDropEvent.isNull())
            mimeType = "image";
    }

    if (mimeType.isEmpty() && resourcePath.isEmpty()){
        pEvent->acceptProposedAction();
        return;
    }
    if(!resourcePath.isEmpty())
        addMedia(resourcePath);
    pEvent->acceptProposedAction();
}

void UBTeacherBarDropMediaZone::dragMoveEvent(QDragMoveEvent *pEvent)
{
    pEvent->acceptProposedAction();
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
    if(NULL != mpTitle){
        delete mpTitle;
        mpTitle = NULL;
    }
    if(NULL != mpTitleLabel){
        delete mpTitleLabel;
        mpTitleLabel = NULL;
    }
    if(NULL != mpUrlLabel){
        delete mpUrlLabel;
        mpUrlLabel = NULL;
    }
    if(NULL != mpUrl){
        delete mpUrl;
        mpUrl = NULL;
    }
    if(NULL != mpTitleLayout){
        delete mpTitleLayout;
        mpTitleLayout = NULL;
    }
    if(NULL != mpLabelLayout){
        delete mpLabelLayout;
        mpLabelLayout = NULL;
    }
    if(NULL != mpLayout){
        delete mpLayout;
        mpLayout = NULL;
    }
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

    mLayout.addWidget(&mMediaViewer, 1);
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
    if(NULL != mpLinksLabel){
        delete mpLinksLabel;
        mpLinksLabel = NULL;
    }
    if(NULL != mpComments){
        delete mpComments;
        mpComments = NULL;
    }
    if(NULL != mpTitle){
        delete mpTitle;
        mpTitle = NULL;
    }
    if(NULL != mpDuration){
        delete mpDuration;
        mpDuration = NULL;
    }
    if(NULL != mpActionsLabel){
        delete mpActionsLabel;
        mpActionsLabel = NULL;
    }
    if(NULL != mpMediaLabel){
        delete mpMediaLabel;
        mpMediaLabel =  NULL;
    }
    if(NULL != mpCommentsLabel){
        delete mpCommentsLabel;
        mpCommentsLabel = NULL;
    }
    if(NULL != mpEditButton){
        delete mpEditButton;
        mpEditButton = NULL;
    }
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
        mWidgets << mpComments;
        mMediaViewer.loadWidgets(mWidgets);
    }
}

void UBTeacherBarPreviewWidget::clean()
{
    mMediaViewer.cleanMedia();
    hideElements();
}

void UBTeacherBarPreviewWidget::hideElements()
{
    mpActionsLabel = new QLabel(tr("Actions"), this);
    mpActionsLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mpMediaLabel = new QLabel(tr("Media"), this);
    mpMediaLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mpCommentsLabel = new QLabel(tr("Comments"), this);
    mpCommentsLabel->setObjectName("UBTeacherBarPreviewSubtitle");
    mpComments = new QLabel(this);
    mpComments->setWordWrap(true);
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
        mMediaViewer.loadWidgets(mWidgets);
    }
}

void UBTeacherBarPreviewWidget::setLinks(QStringList links)
{
    if(!links.empty()){
        mWidgets.clear();
        mpLinksLabel->setVisible(true);
        mWidgets << mpLinksLabel;
        foreach(QString link, links){
            mpTmpLink = new QLabel(link, this);
            mpTmpLink->setOpenExternalLinks(true);
            mWidgets << mpTmpLink;
        }
        mMediaViewer.loadWidgets(mWidgets);
    }
}

// ------------------------------------------------------------------------------------
UBTeacherBarPreviewMedia::UBTeacherBarPreviewMedia(QWidget* parent, const char* name) : QWidget(parent)
{
    setObjectName(name);
    mWidget = new UBWidgetList(parent);
//    mWidget->setEmptyText(tr("No media found"));
    mLayout.addWidget(mWidget);
//    mWidget->setStyleSheet(QString("background-color: red;"));
    setLayout(&mLayout);
    mWidgetList.clear();
}

UBTeacherBarPreviewMedia::~UBTeacherBarPreviewMedia()
{
    if(mWidget){
        delete mWidget;
        mWidget = NULL;
    }

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

void UBTeacherBarPreviewMedia::loadWidgets(QList<QWidget*> pWidgetsList)
{
    foreach(QWidget*eachWidget, pWidgetsList){
        mWidget->addWidget(eachWidget);
        mWidgetList[eachWidget]="DRAG UNAVAILABLE";
    }
}


void UBTeacherBarPreviewMedia::loadMedia(QStringList pMedias)
{
    foreach(QString eachString, pMedias){
        if(!eachString.isEmpty()){
            QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(eachString);
            if(mimeType.contains("image")){
                UBDraggableLabel* label = new UBDraggableLabel();
                label->loadImage(eachString);
                mWidget->addWidget(label);
                mWidgetList[label]=eachString;
            }
            else if(mimeType.contains("video") || mimeType.contains("audio")){
                UBDraggableMediaPlayer* mediaPlayer = new UBDraggableMediaPlayer();
                mediaPlayer->setFile(eachString);
                mWidget->addWidget(mediaPlayer);
                mWidgetList[mediaPlayer] = eachString;
            }
            else{
                qWarning() << "pMediaPath" << eachString;
                qWarning() << "bad idea to come here";
            }
        }
    }
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
                mpContent->setStyleSheet("background:lightblue;");
                break;

            case eActionOwner_Student:
                mpOwner->setText(tr("Student"));
                mpContent->setStyleSheet("background:lightgreen;");
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
