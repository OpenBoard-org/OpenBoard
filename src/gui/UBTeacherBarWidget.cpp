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
    connect(UBApplication::boardController, SIGNAL(activeSceneChanged()), this, SLOT(loadContent()));
    connect(UBApplication::mainWindow->actionQuit, SIGNAL(triggered()), this, SLOT(saveContent()));

    connect(mpPreview, SIGNAL(showEditMode()), this, SLOT(onShowEditMode()));
    connect(mpDocPreviewWidget, SIGNAL(changeTBState(eTeacherBarState)), this, SLOT(onTBStateChanged(eTeacherBarState)));
    connect(mpDocEditWidget, SIGNAL(changeTBState(eTeacherBarState)), this, SLOT(onTBStateChanged(eTeacherBarState)));
    connect(mpPageEditWidget, SIGNAL(changeTBState(eTeacherBarState)), this, SLOT(onTBStateChanged(eTeacherBarState)));
}

UBTeacherBarWidget::~UBTeacherBarWidget()
{
    DELETEPTR(mpDocPreviewWidget);
    DELETEPTR(mpDocEditWidget);
    DELETEPTR(mpPageEditWidget);
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
    mpPageEditWidget->saveInfos(&infos);
    UBPersistenceManager::persistenceManager()->persistTeacherBar(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex(), infos);
}

void UBTeacherBarWidget::loadContent()
{
//    clearWidgetLists();
//    sTeacherBarInfos nextInfos = UBPersistenceManager::persistenceManager()->getTeacherBarInfos(UBApplication::boardController->activeDocument(), UBApplication::boardController->activeSceneIndex());

//    mpPageEditWidget->loadInfos(&nextInfos);
//    mpPreview->mediaViewer()->cleanMedia();

//    if(!isEmpty()){
//        // Update the fields of the preview widget
//        onTBStateChanged(eTeacherBarState_PagePreview);
//        mpPreview->clean();
//        mpPreview->loadInfos(&nextInfos);

//        // Add the actions
//        if(!mActionList.empty()){
//            QStringList actions;
//            foreach(UBTeacherStudentAction* action, mActionList){
//                QString desc = QString("%0;%1").arg(action->comboValue()).arg(action->text());
//                actions << desc;
//            }
//            mpPreview->setActions(actions);
//        }

//        // Add the media
//        if(nextInfos.medias.count() > 0){
//            QList<QWidget*> widgetList;
//            widgetList.append(mpPreview->mediaLabel());
//            mpPreview->mediaViewer()->loadWidgets(widgetList,false);
//            int loadedMedia = mpPreview->mediaViewer()->loadMedia(nextInfos.medias);
//            if(loadedMedia)
//                    mpPreview->mediaLabel()->setVisible(true);
//        }


//        // Add the links
//        if(!mUrlList.empty()){
//            QStringList links;
//            foreach(UBUrlWidget* url, mUrlList){
//                QStringList list = url->url().split(";");
//                QString formedlink = "<a href=";
//                if(!list.at(0).startsWith("http://"))
//                    formedlink += "http://";
//                formedlink += list.at(0) + ">" + list.at(1) + "</a>";
//                links << formedlink;
//            }
//            mpPreview->setLinks(links);
//        }
//    }
//    else{
//        // If the document has only one page, show the document edit page
//        if(1 == UBApplication::boardController->activeDocument()->pageCount()){
//            onTBStateChanged(eTeacherBarState_DocumentEdit);
//        }else{
//            onTBStateChanged(eTeacherBarState_PageEdit);
//        }
//    }
}

bool UBTeacherBarWidget::isEmpty()
{
    return  mData.pageTitle() == "" &&
            mData.urls().empty() &&
            mData.actions().empty() &&
            mData.medias().empty() &&
            mData.comments() == "";
}

void UBTeacherBarWidget::onTitleTextChanged(const QString& text)
{
        //mpTitle->setToolTip(text);
}

void UBTeacherBarWidget::clearWidgetLists()
{
//    if(NULL != mpMediaContainer){
//        for(int i=0; i<mMediaList.size(); i++){
//            mpMediaContainer->removeWidget(mMediaList.at(i));
//            delete mMediaList.at(i);
//        }
//        mMediaList.clear();
//        mpMediaContainer->cleanMedias();
//    }

//    if(NULL != mpActions){
//        for(int i=0; i<mActionList.size(); i++){
//            mpActions->removeWidget(mActionList.at(i));
//            delete mActionList.at(i);
//        }
//        mActionList.clear();
//    }

//    if(NULL != mpLinks){
//        for(int i=0; i<mUrlList.size(); i++){
//            mpLinks->removeWidget(mUrlList.at(i));
//            delete mUrlList.at(i);
//        }
//        mUrlList.clear();
//    }
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

