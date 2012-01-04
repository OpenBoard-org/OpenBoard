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
    , mpCommentLabel(NULL)
    , mpComments(NULL)
    , mpLinkLabel(NULL)
    , mpLinks(NULL)
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
    mpLayout->addWidget(mpActionButton, 0);

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
    mpLayout->addWidget(mpLinkButton);

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
    if( mpTitle->text() == ""
        && mpAction1->teacherText() == ""
        && mpAction1->studentText() == "")
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

}

void UBTeacherBarWidget::onLinkButton()
{

}

UBTeacherStudentAction::UBTeacherStudentAction(int actionNumber, QWidget *parent, const char *name):QWidget(parent)
    , mpActionLabel(NULL)
    , mpTeacherLabel(NULL)
    , mpStudentLabel(NULL)
    , mpTeacher(NULL)
    , mpStudent(NULL)
    , mpLayout(NULL)
    , mpTeacherLayout(NULL)
    , mpStudentLayout(NULL)
{
    setObjectName(name);
    mActionNumber = actionNumber;

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    // Create the GUI
    mpLayout = new QVBoxLayout(this);
    setLayout(mpLayout);

    mpActionLabel = new QLabel(tr("Action %0").arg(mActionNumber), this);
    mpLayout->addWidget(mpActionLabel, 0);

    mpTeacherLayout = new QHBoxLayout();

    mpTeacherLabel = new QLabel(tr("Teacher"), this);
    mpTeacherLabel->setAlignment(Qt::AlignTop);
    mpTeacher = new QTextEdit(this);
    mpTeacher->setObjectName("TeacherStudentBox");
    mpTeacher->setStyleSheet("background-color:#FF9F6D");
    mpTeacherLayout->addWidget(mpTeacherLabel, 0);
    mpTeacherLayout->addWidget(mpTeacher, 1);
    mpLayout->addLayout(mpTeacherLayout, 1);

    mpStudentLayout = new QHBoxLayout();
    mpStudentLabel = new QLabel(tr("Student"), this);
    mpStudentLabel->setAlignment(Qt::AlignTop);
    mpStudent = new QTextEdit(this);
    mpStudent->setObjectName("TeacherStudentBox");
    mpStudent->setStyleSheet("background-color:#06E983");
    mpStudentLayout->addWidget(mpStudentLabel, 0);
    mpStudentLayout->addWidget(mpStudent, 1);
    mpLayout->addLayout(mpStudentLayout, 1);
}

UBTeacherStudentAction::~UBTeacherStudentAction()
{
    if(NULL != mpActionLabel)
    {
        delete mpActionLabel;
        mpActionLabel = NULL;
    }
    if(NULL != mpTeacherLabel)
    {
        delete mpTeacherLabel;
        mpTeacherLabel = NULL;
    }
    if(NULL != mpTeacher)
    {
        delete mpTeacher;
        mpTeacher = NULL;
    }
    if(NULL != mpTeacherLayout)
    {
        delete mpTeacherLayout;
        mpTeacherLayout = NULL;
    }
    if(NULL != mpStudentLabel)
    {
        delete mpStudentLabel;
        mpStudentLabel = NULL;
    }
    if(NULL != mpStudent)
    {
        delete mpStudent;
        mpStudent = NULL;
    }
    if(NULL != mpStudentLayout)
    {
        delete mpStudentLayout;
        mpStudentLayout = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}

QString UBTeacherStudentAction::teacherText()
{
    return mpTeacher->document()->toPlainText();
}

QString UBTeacherStudentAction::studentText()
{
    return mpStudent->document()->toPlainText();
}

void UBTeacherStudentAction::setTeacherText(QString text)
{
    mpTeacher->setText(text);
}

void UBTeacherStudentAction::setStudentText(QString text)
{
    mpStudent->setText(text);
}

QTextEdit* UBTeacherStudentAction::teacher()
{
    return mpTeacher;
}

QTextEdit* UBTeacherStudentAction::student()
{
    return mpStudent;
}


UBTeacherBarDropMediaZone::UBTeacherBarDropMediaZone(QWidget *parent, const char *name):UBWidgetList(parent)

{
    setObjectName(name);
    setAcceptDrops(true);
    setEmptyText(tr("Drag media here ..."));
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
    QString mimeType = UBFileSystemUtils::mimeTypeFromFileName(pEvent->mimeData()->urls().at(0).toLocalFile());
    if(mimeType.contains("image")){
        QPixmap pix = QPixmap(pEvent->mimeData()->urls().at(0).toLocalFile());
        QLabel* label = new QLabel();
        label->setPixmap(pix);
        label->setScaledContents(true);
        addWidget(label);
        mWidgetList << label;
    }
    else if(mimeType.contains("video") || mimeType.contains("audio")){
        UBMediaPlayer* mediaPlayer = new UBMediaPlayer();
        mediaPlayer->setFile(pEvent->mimeData()->urls().at(0).toLocalFile());
        mediaPlayer->playPause();
        addWidget(mediaPlayer);
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
