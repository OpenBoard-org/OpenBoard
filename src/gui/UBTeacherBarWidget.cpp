 #include "UBTeacherBarWidget.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

UBTeacherBarWidget::UBTeacherBarWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
    , mpLayout(NULL)
    , mpTitleLayout(NULL)
    , mpPhasisLayout(NULL)
    , mpDurationLayout(NULL)
    , mpEquipmentLayout(NULL)
    , mpActivityLayout(NULL)
    , mpTitleLabel(NULL)
    , mpPhasisLabel(NULL)
    , mpDurationLabel(NULL)
    , mpEquipmentLabel(NULL)
    , mpActivityLabel(NULL)
    , mpTitle(NULL)
    , mpEquipment(NULL)
    , mpPhasis(NULL)
    , mpDuration(NULL)
    , mpActivity(NULL)
    , mpAction1(NULL)
    , mpAction2(NULL)
    , mpAction3(NULL)
    , mpContainer(NULL)
    , mpContainerLayout(NULL)
{
    setObjectName(name);
    mName = "TeacherBarWidget";

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
    mpTitleLabel->setMinimumWidth(LABEL_MINWIDHT);
    mpTitleLabel->setAlignment(Qt::AlignRight);
    mpTitle = new QLineEdit(mpContainer);
    mpTitle->setObjectName("DockPaletteWidgetLineEdit");
    mpTitleLayout = new QHBoxLayout();
    mpTitleLayout->addWidget(mpTitleLabel, 0);
    mpTitleLayout->addWidget(mpTitle, 1);
    mpLayout->addLayout(mpTitleLayout);

    // Phasis
    mpPhasisLabel = new QLabel(tr("Phasis"), mpContainer);
    mpPhasisLabel->setMinimumWidth(LABEL_MINWIDHT);
    mpPhasisLabel->setAlignment(Qt::AlignRight);
    mpPhasis = new QComboBox(mpContainer);
    mpPhasis->setObjectName("DockPaletteWidgetComboBox");
    mpPhasisLayout = new QHBoxLayout();
    mpPhasisLayout->addWidget(mpPhasisLabel, 0);
    mpPhasisLayout->addWidget(mpPhasis, 1);
    mpLayout->addLayout(mpPhasisLayout);

    // Duration
    mpDurationLabel = new QLabel(tr("Duration"), mpContainer);
    mpDurationLabel->setMinimumWidth(LABEL_MINWIDHT);
    mpDurationLabel->setAlignment(Qt::AlignRight);
    mpDuration = new QComboBox(mpContainer);
    mpDuration->setObjectName("DockPaletteWidgetComboBox");
    mpDurationLayout = new QHBoxLayout();
    mpDurationLayout->addWidget(mpDurationLabel, 0);
    mpDurationLayout->addWidget(mpDuration, 1);
    mpLayout->addLayout(mpDurationLayout);

    // Equipment
    mpEquipmentLabel = new QLabel(tr("Equipment"), mpContainer);
    mpEquipmentLabel->setMinimumWidth(LABEL_MINWIDHT);
    mpEquipmentLabel->setAlignment(Qt::AlignRight);
    mpEquipment = new QLineEdit(mpContainer);
    mpEquipment->setObjectName("DockPaletteWidgetLineEdit");
    mpEquipmentLayout = new QHBoxLayout();
    mpEquipmentLayout->addWidget(mpEquipmentLabel, 0);
    mpEquipmentLayout->addWidget(mpEquipment, 1);
    mpLayout->addLayout(mpEquipmentLayout);

    // Activity
    mpActivityLabel = new QLabel(tr("Activity"), mpContainer);
    mpActivityLabel->setMinimumWidth(LABEL_MINWIDHT);
    mpActivityLabel->setAlignment(Qt::AlignRight);
    mpActivity = new QComboBox(mpContainer);
    mpActivity->setObjectName("DockPaletteWidgetComboBox");
    mpActivityLayout = new QHBoxLayout();
    mpActivityLayout->addWidget(mpActivityLabel, 0);
    mpActivityLayout->addWidget(mpActivity, 1);
    mpLayout->addLayout(mpActivityLayout);

    // Actions
    mpAction1 = new UBTeacherStudentAction(1, mpContainer);
    mpAction2 = new UBTeacherStudentAction(2, mpContainer);
    mpAction3 = new UBTeacherStudentAction(3, mpContainer);

    mpLayout->addWidget(mpAction1);
    mpLayout->addWidget(mpAction2);
    mpLayout->addWidget(mpAction3);

    populateCombos();

    connect(UBApplication::boardController, SIGNAL(activeSceneWillChange()), this, SLOT(saveContent()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(loadContent()));
    connect(mpTitle, SIGNAL(textChanged(QString)), this, SLOT(onValueChanged()));
    connect(mpPhasis, SIGNAL(currentIndexChanged(int)), this, SLOT(onValueChanged()));
    connect(mpDuration, SIGNAL(currentIndexChanged(int)), this, SLOT(onValueChanged()));
    connect(mpEquipment, SIGNAL(textChanged(QString)), this, SLOT(onValueChanged()));
    connect(mpActivity, SIGNAL(currentIndexChanged(int)), this, SLOT(onValueChanged()));
    connect(mpAction1->teacher(), SIGNAL(textChanged()), this, SLOT(onValueChanged()));
    connect(mpAction1->student(), SIGNAL(textChanged()), this, SLOT(onValueChanged()));
    connect(mpAction2->teacher(), SIGNAL(textChanged()), this, SLOT(onValueChanged()));
    connect(mpAction2->student(), SIGNAL(textChanged()), this, SLOT(onValueChanged()));
    connect(mpAction3->teacher(), SIGNAL(textChanged()), this, SLOT(onValueChanged()));
    connect(mpAction3->student(), SIGNAL(textChanged()), this, SLOT(onValueChanged()));
}

UBTeacherBarWidget::~UBTeacherBarWidget()
{
    if(NULL != mpAction3)
    {
        delete mpAction3;
        mpAction3 = NULL;
    }
    if(NULL != mpAction2)
    {
        delete mpAction2;
        mpAction2 = NULL;
    }
    if(NULL != mpAction1)
    {
        delete mpAction1;
        mpAction1 = NULL;
    }
    if(NULL != mpActivityLabel)
    {
        delete mpActivityLabel;
        mpActivityLabel = NULL;
    }
    if(NULL != mpActivity)
    {
        delete mpActivity;
        mpActivity = NULL;
    }
    if(NULL != mpActivityLayout)
    {
        delete mpActivityLayout;
        mpActivityLayout = NULL;
    }
    if(NULL != mpEquipmentLabel)
    {
        delete mpEquipmentLabel;
        mpEquipmentLabel = NULL;
    }
    if(NULL != mpEquipment)
    {
        delete mpEquipment;
        mpEquipment = NULL;
    }
    if(NULL != mpEquipmentLayout)
    {
        delete mpEquipmentLayout;
        mpEquipmentLayout = NULL;
    }
    if(NULL != mpDurationLabel)
    {
        delete mpDurationLabel;
        mpDurationLabel = NULL;
    }
    if(NULL != mpDuration)
    {
        delete mpDuration;
        mpDuration = NULL;
    }
    if(NULL != mpDurationLayout)
    {
        delete mpDurationLayout;
        mpDurationLayout = NULL;
    }
    if(NULL != mpPhasisLabel)
    {
        delete mpPhasisLabel;
        mpPhasisLabel = NULL;
    }
    if(NULL != mpPhasisLayout)
    {
        delete mpPhasisLayout;
        mpPhasisLayout = NULL;
    }
    if(NULL != mpTitleLabel)
    {
        delete mpTitleLabel;
        mpTitleLabel = NULL;
    }
    if(NULL != mpTitle)
    {
        delete mpTitle;
        mpTitle = NULL;
    }
    if(NULL != mpTitleLayout)
    {
        delete mpTitleLayout;
        mpTitleLayout = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
    if(NULL != mpContainer)
    {
        delete mpContainer;
        mpContainer = NULL;
    }
    if(NULL != mpContainerLayout)
    {
        delete mpContainerLayout;
        mpContainerLayout = NULL;
    }
}

void UBTeacherBarWidget::populateCombos()
{
    QStringList qslPhasis;
    qslPhasis << tr("") << tr("I discover") << tr("I experiment") << tr("I train myself") << tr("I play") << tr("I memorize");
    mpPhasis->insertItems(0, qslPhasis);
    mpPhasis->setCurrentIndex(0);

    QStringList qslDuration;
    qslDuration << tr("") << tr("Short") << tr("Middle") << tr("Long");
    mpDuration->insertItems(0, qslDuration);
    mpDuration->setCurrentIndex(0);

    QStringList qslActivity;
    qslActivity << tr("") << tr("Alone") << tr("By Group") << tr("All together");
    mpActivity->insertItems(0, qslActivity);
    mpActivity->setCurrentIndex(0);
}

void UBTeacherBarWidget::saveContent()
{
    sTeacherBarInfos infos;
    infos.title = mpTitle->text();
    infos.phasis = mpPhasis->currentIndex();
    infos.Duration = mpDuration->currentIndex();
    infos.material = mpEquipment->text();
    infos.activity = mpActivity->currentIndex();
    infos.action1Master = mpAction1->teacherText();
    infos.action1Student = mpAction1->studentText();
    infos.action2Master = mpAction2->teacherText();
    infos.action2Student = mpAction2->studentText();
    infos.action3Master = mpAction3->teacherText();
    infos.action3Student = mpAction3->studentText();
    UBPersistenceManager::persistenceManager()->persistTeacherBar(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex(), infos);
}

void UBTeacherBarWidget::loadContent()
{
    sTeacherBarInfos nextInfos = UBPersistenceManager::persistenceManager()->getTeacherBarInfos(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex());
    mpTitle->setText(nextInfos.title);
    mpPhasis->setCurrentIndex(nextInfos.phasis);
    mpDuration->setCurrentIndex(nextInfos.Duration);
    mpEquipment->setText(nextInfos.material);
    mpActivity->setCurrentIndex(nextInfos.activity);
    mpAction1->setTeacherText(nextInfos.action1Master);
    mpAction1->setStudentText(nextInfos.action1Student);
    mpAction2->setTeacherText(nextInfos.action2Master);
    mpAction2->setStudentText(nextInfos.action2Student);
    mpAction3->setTeacherText(nextInfos.action3Master);
    mpAction3->setStudentText(nextInfos.action3Student);
}

void UBTeacherBarWidget::onValueChanged()
{
    if( mpTitle->text() == ""
        && mpDuration->currentIndex() == 0
        && mpPhasis->currentIndex() == 0
        && mpEquipment->text() == ""
        && mpActivity->currentIndex() == 0
        && mpAction1->teacherText() == ""
        && mpAction1->studentText() == ""
        && mpAction2->teacherText() == ""
        && mpAction2->studentText() == ""
        && mpAction3->teacherText() == ""
        && mpAction3->studentText() == "")
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

    //TODO: I'm not able to translate this string using the normal way *qm file why?
    //mpTeacherLabel = new QLabel(tr("Teacher"), this);
    mpTeacherLabel = new QLabel(tr("Enseignant"), this);
    mpTeacherLabel->setAlignment(Qt::AlignTop);
    mpTeacher = new QTextEdit(this);
    mpTeacher->setObjectName("TeacherStudentBox");
    mpTeacher->setStyleSheet("background-color:#FF9F6D");
    mpTeacherLayout->addWidget(mpTeacherLabel, 0);
    mpTeacherLayout->addWidget(mpTeacher, 1);
    mpLayout->addLayout(mpTeacherLayout, 1);

    mpStudentLayout = new QHBoxLayout();
//    mpStudentLabel = new QLabel(tr("Student"), this);
    mpStudentLabel = new QLabel(tr("Élève"), this);
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
