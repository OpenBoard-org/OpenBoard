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

#include "core/memcheck.h"

UBTeacherBarWidget::UBTeacherBarWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
    , mpLayout(NULL)
    , mpTitleLayout(NULL)
    , mpDurationLayout(NULL)
    , mpTitleLabel(NULL)
    , mpDurationLabel(NULL)
    , mpTitle(NULL)
    , mpAction1(NULL)
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
{
    setObjectName(name);
    mName = "TeacherBarWidget";
    mVisibleState = true;

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    mIconToLeft = QPixmap(":images/teacher_open.png");
    mIconToRight = QPixmap(":images/teacher_close.png");

    // Create the GUI
    mpContainerLayout = new QVBoxLayout(this);
    setLayout(mpContainerLayout);

    mpContainer = new QWidget(this);
    mpContainer->setObjectName("DockPaletteWidgetBox");
    mpContainerLayout->addWidget(mpContainer);

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
    mpDurationButtons = new QButtonGroup(this);
    mpDurationButtons->addButton(mpDuration1);
    mpDurationButtons->addButton(mpDuration2);
    mpDurationButtons->addButton(mpDuration3);
    mpLayout->addLayout(mpDurationLayout, 0);

    // Actions
    mpActionLabel = new QLabel(tr("Actions"), this);
    mpLayout->addWidget(mpActionLabel, 0);
    mpActions = new UBWidgetList(this);
    mpActions->setEmptyText(tr("Add actions"));
    mpLayout->addWidget(mpActions, 1);
    mpActionButton = new QPushButton(this);
    mpActionButton->setObjectName("DockPaletteWidgetButton");
    mpActionButton->setText(tr("Add action"));
    mpActionLayout = new QHBoxLayout();
    mpActionLayout->addWidget(mpActionButton, 0);
    mpActionLayout->addStretch(1);
    mpLayout->addLayout(mpActionLayout, 0);

    // Media
    mpMediaLabel = new QLabel(tr("Media"), this);
    mpLayout->addWidget(mpMediaLabel, 0);
    mpDropMediaZone = new UBTeacherBarDropMediaZone();
    mpLayout->addWidget(mpDropMediaZone, 1);

    // Links
    mpLinkLabel = new QLabel(tr("Links"), this);
    mpLayout->addWidget(mpLinkLabel, 0);
    mpLinks = new UBWidgetList(this);
    mpLayout->addWidget(mpLinks, 1);
    mpLinkButton = new QPushButton(tr("Add link"), this);
    mpLinkButton->setObjectName("DockPaletteWidgetButton");
    mpLinkLayout = new QHBoxLayout();
    mpLinkLayout->addWidget(mpLinkButton, 0);
    mpLinkLayout->addStretch(1);
    mpLayout->addLayout(mpLinkLayout, 0);

    // Comments
    mpCommentLabel = new QLabel(tr("Comments"), this);
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
    if(NULL != mpAction1){
        delete mpAction1;
        mpAction1 = NULL;
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
}

void UBTeacherBarWidget::onValueChanged()
{
    if( mpTitle->text() == "")
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
    infos.title = mpTitle->text();
    UBPersistenceManager::persistenceManager()->persistTeacherBar(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex(), infos);
}

void UBTeacherBarWidget::loadContent()
{
    sTeacherBarInfos nextInfos = UBPersistenceManager::persistenceManager()->getTeacherBarInfos(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex());
    mpTitle->setText(nextInfos.title);
}

void UBTeacherBarWidget::onTitleTextChanged(const QString& text)
{
	mpTitle->setToolTip(text);
}

void UBTeacherBarWidget::onActionButton()
{
    UBTeacherStudentAction* pAction = new UBTeacherStudentAction(this);
    mActionList << pAction;
    mpActions->addWidget(pAction);
}

void UBTeacherBarWidget::onLinkButton()
{

}

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
    mpCombo->addItem(tr("Teacher"));
    mpCombo->addItem(tr("Student"));
    mpComboLayout->addWidget(0);
    mpComboLayout->addStretch(1);

    mpLayout->addLayout(mpComboLayout, 0);

    mpText = new QTextEdit(this);
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

    // TODO : Implement this method

    return str;
}

UBTeacherBarDropMediaZone::UBTeacherBarDropMediaZone(QWidget *parent, const char *name):QWidget(parent)

{
    setObjectName(name);
    setAcceptDrops(true);

    mWidget.setEmptyText(tr("Drag media here ..."));
    mLayout.addWidget(&mWidget);
    setLayout(&mLayout);

}

UBTeacherBarDropMediaZone::~UBTeacherBarDropMediaZone()
{
    qDeleteAll(mWidgetList);
}


void UBTeacherBarDropMediaZone::dragEnterEvent(QDragEnterEvent *pEvent)
{
    pEvent->acceptProposedAction();
}

void UBTeacherBarDropMediaZone::dragLeaveEvent(QDragLeaveEvent *pEvent)
{
    pEvent->accept();
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

    mimeType = mimeType.isEmpty() ?  UBFileSystemUtils::mimeTypeFromFileName(resourcePath) : mimeType;
    if(mimeType.contains("image")){
        qDebug() << pixFromDropEvent.size();
        QPixmap pix = pixFromDropEvent.height() ? pixFromDropEvent : QPixmap(resourcePath);
        QLabel* label = new QLabel();
        label->setPixmap(pix);
        //label->resize(size());
        label->setScaledContents(true);
        mWidget.addWidget(label);
        mWidgetList << label;
    }
    else if(mimeType.contains("video") || mimeType.contains("audio")){
        UBMediaPlayer* mediaPlayer = new UBMediaPlayer();
        mediaPlayer->setFile(resourcePath);
        //mediaPlayer->resize(size());
        mediaPlayer->playPause();
        mWidget.addWidget(mediaPlayer);
        mWidgetList << mediaPlayer;
    }
    else{
        qWarning() << "bad idea to come here";
    }
    pEvent->acceptProposedAction();
}

void UBTeacherBarDropMediaZone::dragMoveEvent(QDragMoveEvent *pEvent)
{
    pEvent->acceptProposedAction();
}
