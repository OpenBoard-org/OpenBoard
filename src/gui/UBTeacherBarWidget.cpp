#include "UBTeacherBarWidget.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "UBMainWindow.h"

#include "document/UBDocumentController.h"
#include "document/UBDocumentProxy.h"

#include "board/UBBoardController.h"
#include "board/UBBoardPaletteManager.h"

#include "gui/UBMediaPlayer.h"

#include "customWidgets/UBDraggableLabel.h"
#include "customWidgets/UBMediaWidget.h"
#include "customWidgets/UBGlobals.h"

#include "core/memcheck.h"

UBTeacherBarWidget::UBTeacherBarWidget(QWidget *parent, const char *name):UBDockPaletteWidget(parent)
    , mpStackWidget(NULL)
    , mpPreview(NULL)
    , mpDocPreviewWidget(NULL)
    , mpDocEditWidget(NULL)
  {
    setObjectName(name);
    mName = "TeacherBarWidget";
    mVisibleState = true;
    mData.clearLists();

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    mIconToLeft = QPixmap(":images/teacher_open.png");
    mIconToRight = QPixmap(":images/teacher_close.png");

    // Create the GUI
    setLayout(&mLayout);

    mpPageEditWidget = new UBTBPageEditWidget(&mData, this);
    mpPreview = new UBTeacherBarPreviewWidget(&mData, this);
    mpDocPreviewWidget = new UBTBDocumentPreviewWidget(&mData, this);
    mpDocEditWidget = new UBTBDocumentEditWidget(&mData, this);

    mpStackWidget = new QStackedWidget(this);
    mLayout.addWidget(mpStackWidget);
    mpStackWidget->addWidget(mpPageEditWidget);
    mpStackWidget->addWidget(mpPreview);
    mpStackWidget->addWidget(mpDocPreviewWidget);
    mpStackWidget->addWidget(mpDocEditWidget);

    connect(UBApplication::boardController, SIGNAL(activeSceneWillChange()), this, SLOT(saveContent()));
    connect(UBApplication::mainWindow->actionQuit, SIGNAL(triggered()), this, SLOT(saveContent()));
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(loadContentInfos()));
    connect(UBApplication::boardController, SIGNAL(activeDocumentChanged()), this, SLOT(onActiveDocumentChanged()));

    connect(mpPreview, SIGNAL(showEditMode()), this, SLOT(onShowEditMode()));
    connect(mpDocPreviewWidget, SIGNAL(changeTBState(eTeacherBarState)), this, SLOT(onTBStateChanged(eTeacherBarState)));
    connect(mpDocEditWidget, SIGNAL(changeTBState(eTeacherBarState)), this, SLOT(onTBStateChanged(eTeacherBarState)));
    connect(mpPageEditWidget, SIGNAL(changeTBState(eTeacherBarState)), this, SLOT(onTBStateChanged(eTeacherBarState)));
    connect(mpPageEditWidget, SIGNAL(valueChanged()), this, SLOT(onValueChanged()));
    connect(mpDocEditWidget, SIGNAL(valueChanged()), this, SLOT(onValueChanged()));
}

UBTeacherBarWidget::~UBTeacherBarWidget()
{
    DELETEPTR(mpDocPreviewWidget);
    DELETEPTR(mpDocEditWidget);
    DELETEPTR(mpPageEditWidget);
    DELETEPTR(mpPreview);
    DELETEPTR(mpStackWidget);
}

void UBTeacherBarWidget::onActiveDocumentChanged()
{
    loadContent(true);
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
    mpPageEditWidget->saveFields();
    mData.saveContent();
}

void UBTeacherBarWidget::loadContentInfos()
{
    loadContent(false);
}

void UBTeacherBarWidget::loadContent(bool docChanged)
{
    // Clear the old datas
    mpPageEditWidget->clearFields();
    mpPreview->clearFields();
    if(docChanged){
        mpDocEditWidget->clearFields();
        mpDocPreviewWidget->clearFields();
    }

    // Update the datas
    mData.loadContent(docChanged);

    // Update the fields
    mpPageEditWidget->updateFields();
    mpPreview->updateFields();
    if(docChanged){
        mpDocEditWidget->updateFields();
        mpDocPreviewWidget->updateFields();
    }

    if(!isEmpty()){
        onTBStateChanged(eTeacherBarState_PagePreview);
    }else{
        if(1 == UBApplication::boardController->activeDocument()->pageCount()){
           onTBStateChanged(eTeacherBarState_DocumentEdit);
        }else{
           onTBStateChanged(eTeacherBarState_PageEdit);
        }
    }
}

bool UBTeacherBarWidget::isEmpty()
{
    return  mData.pageTitle() == "" &&
            mData.urls()->empty() &&
            mData.actions()->empty() &&
            mData.medias()->empty() &&
            mData.comments() == "";
}

void UBTeacherBarWidget::onShowEditMode()
{
    mpStackWidget->setCurrentWidget(mpPageEditWidget);
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
        mpStackWidget->setCurrentWidget(mpPageEditWidget);
        break;
    case eTeacherBarState_PagePreview:
        mpStackWidget->setCurrentWidget(mpPreview);
        break;
    }
}

