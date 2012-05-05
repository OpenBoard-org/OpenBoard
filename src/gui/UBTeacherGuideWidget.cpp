/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QDebug>
#include <QLabel>
#include <QVBoxLayout>
#include <QHeaderView>
#include <QTreeWidget>
#include <QPushButton>
#include <QDomDocument>

#include "UBTeacherGuideWidget.h"

#include "adaptors/UBSvgSubsetAdaptor.h"

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"
#include "core/UBSettings.h"

#include "globals/UBGlobals.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBBoardPaletteManager.h"

#include "gui/UBStylusPalette.h"
#include "gui/UBActionPalette.h"

#include "web/UBWebController.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#include "domain/UBGraphicsTextItem.h"

#define UBTG_SEPARATOR_FIXED_HEIGHT 3

typedef enum
{
    eUBTGAddSubItemWidgetType_None,
    eUBTGAddSubItemWidgetType_Action ,
    eUBTGAddSubItemWidgetType_Media,
    eUBTGAddSubItemWidgetType_Url
}eUBTGAddSubItemWidgetType;



/***************************************************************************
 *               class    UBTeacherGuideEditionWidget                      *
 ***************************************************************************/

UBTeacherGuideEditionWidget::UBTeacherGuideEditionWidget(QWidget *parent, const char* name) :
    QWidget(parent)
  , mpLayout(NULL)
  , mpDocumentTitle(NULL)
  , mpPageNumberLabel(NULL)
  , mpPageTitle(NULL)
  , mpComment(NULL)
  , mpSeparator(NULL)
  , mpTreeWidget(NULL)
  , mpRootWidgetItem(NULL)
  , mpAddAnActionItem(NULL)
  , mpAddAMediaItem(NULL)
  , mpAddALinkItem(NULL)
{
    setObjectName(name);

    mpLayout = new QVBoxLayout(this);
    mpPageNumberLabel = new QLabel(this);
    mpPageNumberLabel->setAlignment(Qt::AlignRight);
    mpPageNumberLabel->setObjectName("UBTGPageNumberLabel");
    mpLayout->addWidget(mpPageNumberLabel);
    // tree basic configuration

    if(UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool()){
        mpDocumentTitle = new QLabel(this);
        mpDocumentTitle->setObjectName("UBTGPresentationDocumentTitle");
        mpLayout->addWidget(mpDocumentTitle);
    }

    mpPageTitle = new UBTGAdaptableText(0,this);
    mpPageTitle->setObjectName("UBTGEditionPageTitle");
    mpPageTitle->setPlaceHolderText(tr("Type title here ..."));
    mpLayout->addWidget(mpPageTitle);

    mpComment = new UBTGAdaptableText(0,this);
    mpComment->setObjectName("UBTGEditionComment");
    mpComment->setPlaceHolderText(tr("Type comment here ..."));
    mpLayout->addWidget(mpComment);

    mpSeparator = new QFrame(this);
    mpSeparator->setObjectName("UBTGSeparator");
    mpSeparator->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpLayout->addWidget(mpSeparator);

    mpTreeWidget = new QTreeWidget(this);
    mpLayout->addWidget(mpTreeWidget);

    mpRootWidgetItem = mpTreeWidget->invisibleRootItem();
    mpTreeWidget->setRootIsDecorated(false);
    mpTreeWidget->setIndentation(0);
    mpTreeWidget->setDropIndicatorShown(false);
    mpTreeWidget->header()->close();
    mpTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mpTreeWidget->setColumnCount(2);
    mpTreeWidget->header()->setStretchLastSection(false);
    mpTreeWidget->header()->setResizeMode(0, QHeaderView::Stretch);
    mpTreeWidget->header()->setResizeMode(1, QHeaderView::Fixed);
    mpTreeWidget->header()->setDefaultSectionSize(18);

    connect(mpTreeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(onAddItemClicked(QTreeWidgetItem*,int)));
    connect(UBApplication::boardController,SIGNAL(activeSceneChanged()),this,SLOT(onActiveSceneChanged()));

    mpAddAnActionItem = new UBAddItem(tr("Add an action"),eUBTGAddSubItemWidgetType_Action,mpTreeWidget);
    mpAddAMediaItem = new UBAddItem(tr("Add a media"),eUBTGAddSubItemWidgetType_Media,mpTreeWidget);
    mpAddALinkItem = new UBAddItem(tr("Add a link"),eUBTGAddSubItemWidgetType_Url,mpTreeWidget);

    mpRootWidgetItem->addChild(mpAddAnActionItem);
    mpRootWidgetItem->addChild(mpAddAMediaItem);
    mpRootWidgetItem->addChild(mpAddALinkItem);

    if(UBSettings::settings()->teacherGuideLessonPagesActivated->get().toBool()){
        UBSvgSubsetAdaptor::addElementToBeStored(QString("teacherGuide"),this);
        connect(UBApplication::boardController,SIGNAL(activeDocumentChanged()),this,SLOT(onActiveDocumentChanged()));
    }
}

UBTeacherGuideEditionWidget::~UBTeacherGuideEditionWidget()
{
    DELETEPTR(mpDocumentTitle);
    DELETEPTR(mpPageNumberLabel);
    DELETEPTR(mpPageTitle);
    DELETEPTR(mpComment);
    DELETEPTR(mpSeparator);
    DELETEPTR(mpAddAnActionItem);
    DELETEPTR(mpAddAMediaItem);
    DELETEPTR(mpAddALinkItem);
    DELETEPTR(mpTreeWidget)
    DELETEPTR(mpLayout);
}

void UBTeacherGuideEditionWidget::showEvent(QShowEvent* event)
{
//    mpPageTitle->setFocus();
//    mpComment->setFocus();
    setFocus();
    QWidget::showEvent(event);
}

void UBTeacherGuideEditionWidget::onActiveDocumentChanged()
{
    load(UBSvgSubsetAdaptor::sTeacherGuideNode);
}

void UBTeacherGuideEditionWidget::load(QString element)
{
    cleanData();
    QDomDocument doc("TeacherGuide");
    doc.setContent(element);

    for(QDomElement element = doc.documentElement().firstChildElement(); !element.isNull(); element = element.nextSiblingElement()) {
        QString tagName = element.tagName();
        if(tagName == "title")
            mpPageTitle->setInitialText(element.attribute("value"));
        else if(tagName == "comment")
            mpComment->setInitialText(element.attribute("value"));
        else if(tagName == "media")
            onAddItemClicked(mpAddAMediaItem,0,&element);
        else if(tagName == "link")
            onAddItemClicked(mpAddALinkItem,0,&element);
        else if(tagName == "action")
            onAddItemClicked(mpAddAnActionItem,0,&element);
    }
}



QDomElement* UBTeacherGuideEditionWidget::save(QDomElement* parentElement)
{
    qDebug() << parentElement;
    return 0;
}

void UBTeacherGuideEditionWidget::onActiveSceneChanged()
{
    load(UBSvgSubsetAdaptor::sTeacherGuideNode);
    qDebug() << "UBSvgSubsetAdaptor::sTeacherGuideNode " << UBSvgSubsetAdaptor::sTeacherGuideNode;
    mpPageNumberLabel->setText(tr("Page: %0").arg(UBApplication::boardController->currentPage()));
    UBDocumentProxy* documentProxy = UBApplication::boardController->activeDocument();
    if(mpDocumentTitle)
        mpDocumentTitle->setText(documentProxy->metaData(UBSettings::sessionTitle).toString());
}

void UBTeacherGuideEditionWidget::cleanData()
{
    mpPageTitle->setText("");
    mpComment->setText("");
    QList<QTreeWidgetItem*> children = mpAddAnActionItem->takeChildren();
    children << mpAddAMediaItem->takeChildren();
    children << mpAddALinkItem->takeChildren();

    foreach(QTreeWidgetItem* item, children){
        DELETEPTR(item);
    }
}

QList<QTreeWidgetItem*> UBTeacherGuideEditionWidget::getChildrenList(QTreeWidgetItem* widgetItem)
{
    QList<QTreeWidgetItem*>result;
    for(int i=0;i<widgetItem->childCount();i+=1)
        result << widgetItem->child(i);
    return result;
}

QVector<tUBGEElementNode*> UBTeacherGuideEditionWidget::getPageAndCommentData()
{
    QVector<tUBGEElementNode*>result;
    tUBGEElementNode* pageTitle = new tUBGEElementNode();
    pageTitle->type = "pageTitle";
    pageTitle->attributes.insert("value",mpPageTitle->text());
    result << pageTitle;

    tUBGEElementNode* comment = new tUBGEElementNode();
    comment->type = "comment";
    comment->attributes.insert("value",mpComment->text());
    result << comment;
    return result;
}

QVector<tUBGEElementNode*> UBTeacherGuideEditionWidget::getData()
{
    QVector<tUBGEElementNode*>result;
    QList<QTreeWidgetItem*> children = getChildrenList(mpAddAnActionItem);
    children << getChildrenList(mpAddAMediaItem);
    children << getChildrenList(mpAddALinkItem);
    result << getPageAndCommentData();
    foreach(QTreeWidgetItem* widgetItem, children){
        tUBGEElementNode* node = dynamic_cast<iUBTGSaveData*>(mpTreeWidget->itemWidget(widgetItem,0))->saveData();
        if(node)
            result << node;
    }
    return result;
}

void UBTeacherGuideEditionWidget::onAddItemClicked(QTreeWidgetItem* widget, int column, QDomElement *element)
{
    int addSubItemWidgetType = widget->data(column,Qt::UserRole).toInt();
    if(addSubItemWidgetType != eUBTGAddSubItemWidgetType_None){
        QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(widget);
        newWidgetItem->setData(column,Qt::UserRole,eUBTGAddSubItemWidgetType_None);
        newWidgetItem->setData(1,Qt::UserRole,eUBTGAddSubItemWidgetType_None);
        newWidgetItem->setIcon(1,QIcon(":images/close.svg"));

        switch(addSubItemWidgetType)
        {
        case eUBTGAddSubItemWidgetType_Action:{
            UBTGActionWidget* actionWidget = new UBTGActionWidget(widget);
            if(element) actionWidget->initializeWithDom(*element);
            mpTreeWidget->setItemWidget(newWidgetItem,0,actionWidget);
            break;
        }
        case eUBTGAddSubItemWidgetType_Media:{
            UBTGMediaWidget* mediaWidget = new UBTGMediaWidget(widget);
            if(element) mediaWidget->initializeWithDom(*element);
            mpTreeWidget->setItemWidget(newWidgetItem,0,mediaWidget);
            break;
        }
        case eUBTGAddSubItemWidgetType_Url:{
            UBTGUrlWidget* urlWidget = new UBTGUrlWidget();
            if(element) urlWidget->initializeWithDom(*element);
            mpTreeWidget->setItemWidget(newWidgetItem,0,urlWidget);
            break;
        }
        default:
            delete newWidgetItem;
            qCritical() << "onAddItemClicked no action set";
            return;
        }

        if(addSubItemWidgetType != eUBTGAddSubItemWidgetType_None && !widget->isExpanded() )
            widget->setExpanded(true);
        else{
            //to update the tree and subtrees
            widget->setExpanded(false);
            widget->setExpanded(true);
        }
    }
    else if(column == 1 && addSubItemWidgetType == eUBTGAddSubItemWidgetType_None){
        int index = mpTreeWidget->currentIndex().row();
        QTreeWidgetItem* toBeDeletedWidgetItem = widget->parent()->takeChild(index);
        delete toBeDeletedWidgetItem;
    }
}

/***************************************************************************
 *           class    UBTeacherGuidePresentationWidget                     *
 ***************************************************************************/
typedef enum
{
    tUBTGActionAssociateOnClickItem_NONE,
    tUBTGActionAssociateOnClickItem_URL,
    tUBTGActionAssociateOnClickItem_MEDIA,
    tUBTGActionAssociateOnClickItem_EXPAND
}tUBTGActionAssociateOnClickItem;

typedef enum
{
    tUBTGTreeWidgetItemRole_HasAnAction = Qt::UserRole,
    tUBTGTreeWidgetItemRole_HasAnUrl
}tUBTGTreeWidgetItemRole;


UBTeacherGuidePresentationWidget::UBTeacherGuidePresentationWidget(QWidget *parent, const char *name) : QWidget(parent)
  , mpPageTitle(NULL)
  , mpComment(NULL)
  , mpLayout(NULL)
  , mpButtonTitleLayout(NULL)
  , mpDocumentTitle(NULL)
  , mpPageNumberLabel(NULL)
  , mpSeparator(NULL)
  , mpModePushButton(NULL)
  , mpTreeWidget(NULL)
  , mpRootWidgetItem(NULL)
  , mpMediaSwitchItem(NULL)
{
    setObjectName(name);

    mpLayout = new QVBoxLayout(this);

    mpPageNumberLabel = new QLabel(this);
    mpPageNumberLabel->setAlignment(Qt::AlignRight);
    mpPageNumberLabel->setObjectName("UBTGPageNumberLabel");

    mpLayout->addWidget(mpPageNumberLabel);

    mpButtonTitleLayout = new QHBoxLayout(0);

    mpModePushButton = new QPushButton(this);
    mpModePushButton->setIcon(QIcon(":images/pencil.svg"));
    mpModePushButton->setMaximumWidth(32);
    mpModePushButton->installEventFilter(this);

    connect(mpModePushButton,SIGNAL(clicked()),parentWidget(),SLOT(changeMode()));
    mpButtonTitleLayout->addWidget(mpModePushButton);

    if(UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool()){
        mpDocumentTitle = new QLabel(this);
        mpDocumentTitle->setObjectName("UBTGPresentationDocumentTitle");
        mpButtonTitleLayout->addWidget(mpDocumentTitle);
    }

    mpLayout->addLayout(mpButtonTitleLayout);

    mpPageTitle = new UBTGAdaptableText(0,this);
    mpPageTitle->setObjectName("UBTGPresentationPageTitle");
    mpPageTitle->setReadOnly(true);
    mpPageTitle->setStyleSheet("background-color:transparent");
    mpLayout->addWidget(mpPageTitle);

    mpComment = new UBTGAdaptableText(0,this);
    mpComment->setObjectName("UBTGPresentationComment");
    mpComment->setReadOnly(true);
    mpComment->setStyleSheet("background-color:transparent");
    mpLayout->addWidget(mpComment);

    mpSeparator = new QFrame(this);
    mpSeparator->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparator->setObjectName("UBTGSepartor");
    mpLayout->addWidget(mpSeparator);

    mpTreeWidget = new UBTGDraggableTreeItem(this);
    mpLayout->addWidget(mpTreeWidget);

    mpRootWidgetItem = mpTreeWidget->invisibleRootItem();
    mpTreeWidget->setDragEnabled(true);
    mpTreeWidget->setRootIsDecorated(false);
    mpTreeWidget->setIndentation(0);
    mpTreeWidget->setDropIndicatorShown(false);
    mpTreeWidget->header()->close();
    mpTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    connect(mpTreeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(onAddItemClicked(QTreeWidgetItem*,int)));
    connect(UBApplication::boardController,SIGNAL(activeSceneChanged()),this,SLOT(onActiveSceneChanged()));
}

UBTeacherGuidePresentationWidget::~UBTeacherGuidePresentationWidget()
{
    DELETEPTR(mpComment);
    DELETEPTR(mpPageTitle);
    DELETEPTR(mpPageNumberLabel);
    DELETEPTR(mpSeparator);
    DELETEPTR(mpMediaSwitchItem);
    DELETEPTR(mpModePushButton);
    DELETEPTR(mpDocumentTitle);
    DELETEPTR(mpButtonTitleLayout);
    DELETEPTR(mpTreeWidget);
    DELETEPTR(mpLayout);
}

bool UBTeacherGuidePresentationWidget::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);
    if(event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove || event->type() == QEvent::HoverLeave)
        return true;
    return false;
}

void UBTeacherGuidePresentationWidget::cleanData()
{
    mpPageTitle->showText("");
    mpComment->showText("");
    //tree clean
    QList<QTreeWidgetItem*> itemToRemove = mpRootWidgetItem->takeChildren();
    foreach(QTreeWidgetItem* eachItem, itemToRemove){
        DELETEPTR(eachItem);
    }
    // the mpMediaSwitchItem is deleted by the previous loop but the pointer is not set to zero
    mpMediaSwitchItem = NULL;
}

void UBTeacherGuidePresentationWidget::onActiveSceneChanged()
{
    cleanData();
    mpPageNumberLabel->setText(tr("Page: %0").arg(UBApplication::boardController->currentPage()));
    UBDocumentProxy* documentProxy = UBApplication::boardController->activeDocument();
    if(mpDocumentTitle)
        mpDocumentTitle->setText(documentProxy->metaData(UBSettings::sessionTitle).toString());
}

void UBTeacherGuidePresentationWidget::createMediaButtonItem()
{
    if(!mpMediaSwitchItem){
        mpMediaSwitchItem = new QTreeWidgetItem(mpRootWidgetItem);
        mpMediaSwitchItem->setText(0,"+");
        mpMediaSwitchItem->setExpanded(false);
        mpMediaSwitchItem->setData(0,tUBTGTreeWidgetItemRole_HasAnAction,tUBTGActionAssociateOnClickItem_EXPAND);
        mpMediaSwitchItem->setData(0,Qt::BackgroundRole,QVariant(QColor(200,200,200)));
        mpMediaSwitchItem->setData(0,Qt::FontRole, QVariant(QFont(QApplication::font().family(),16)));
        mpMediaSwitchItem->setData(0,Qt::TextAlignmentRole,QVariant(Qt::AlignCenter));
        mpRootWidgetItem->addChild(mpMediaSwitchItem);
    }
}


void UBTeacherGuidePresentationWidget::showData(QVector<tUBGEElementNode*> data)
{
    cleanData();

    foreach(tUBGEElementNode* element, data){
        if(element->type == "pageTitle")
            mpPageTitle->showText(element->attributes.value("value"));
        else if (element->type == "comment")
            mpComment->showText(element->attributes.value("value"));
        else if(element->type == "action"){
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpRootWidgetItem);
            newWidgetItem->setText(0,element->attributes.value("task"));
            newWidgetItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            QString colorString = element->attributes.value("owner").toInt() == 0 ? "red":"green";
            UBTGAdaptableText* textWidget = new UBTGAdaptableText(newWidgetItem,0);
            textWidget->bottomMargin(14);
            textWidget->setStyleSheet("QWidget {background: #EEEEEE; border:none; color:" + colorString + ";}");
            textWidget->showText(element->attributes.value("task"));
            textWidget->document()->setDefaultFont(QFont(QApplication::font().family(),11));
            mpTreeWidget->setItemWidget(newWidgetItem,0,textWidget);

            mpRootWidgetItem->addChild(newWidgetItem);
        }
        else if(element->type == "media"){
            createMediaButtonItem();
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpMediaSwitchItem);
            newWidgetItem->setIcon(0,QIcon(":images/teacherGuide/"+ element->attributes.value("mediaType") +".png"));
            newWidgetItem->setText(0,element->attributes.value("title"));
            newWidgetItem->setData(0,tUBTGTreeWidgetItemRole_HasAnAction,tUBTGActionAssociateOnClickItem_MEDIA);
            newWidgetItem->setData(0,Qt::FontRole, QVariant(QFont(QApplication::font().family(),11)));
            newWidgetItem->setData(0, TG_USER_ROLE_MIME_TYPE, element->attributes.value("relativePath"));
            newWidgetItem->setFlags(Qt::ItemIsDragEnabled | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpRootWidgetItem->addChild(newWidgetItem);

            QTreeWidgetItem* mediaItem = new QTreeWidgetItem(newWidgetItem);
            mediaItem->setData(0,tUBTGTreeWidgetItemRole_HasAnAction,tUBTGActionAssociateOnClickItem_NONE);
            UBTGMediaWidget* mediaWidget = new UBTGMediaWidget(element->attributes.value("relativePath"),newWidgetItem);
            newWidgetItem->setExpanded(false);
            mpTreeWidget->setItemWidget(mediaItem,0,mediaWidget);
        }
        else if(element->type == "link"){
            createMediaButtonItem();
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpMediaSwitchItem);
            newWidgetItem->setIcon(0,QIcon(":images/teacherGuide/link.png"));
            newWidgetItem->setText(0,element->attributes.value("title"));
            newWidgetItem->setData(0,tUBTGTreeWidgetItemRole_HasAnAction,tUBTGActionAssociateOnClickItem_URL);
            newWidgetItem->setData(0,tUBTGTreeWidgetItemRole_HasAnUrl,QVariant(element->attributes.value("url")));
            newWidgetItem->setData(0,Qt::FontRole, QVariant(QFont(QApplication::font().family(),11)));
            newWidgetItem->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable);
            mpRootWidgetItem->addChild(newWidgetItem);
        }
    }
}

void UBTeacherGuidePresentationWidget::onAddItemClicked(QTreeWidgetItem* widget, int column)
{
    int associateAction = widget->data(column,tUBTGTreeWidgetItemRole_HasAnAction).toInt();
    if(column == 0 && associateAction != tUBTGActionAssociateOnClickItem_NONE){
        switch(associateAction)
        {
        case tUBTGActionAssociateOnClickItem_EXPAND:
            widget->setExpanded(!widget->isExpanded());
            if(widget->isExpanded())
                 mpMediaSwitchItem->setText(0,"-");
            else
                 mpMediaSwitchItem->setText(0,"+");
            break;
        case tUBTGActionAssociateOnClickItem_URL:
            widget->data(column,tUBTGTreeWidgetItemRole_HasAnUrl).toString();
            UBApplication::webController->loadUrl(QUrl(widget->data(column,tUBTGTreeWidgetItemRole_HasAnUrl).toString()));
            break;
        case tUBTGActionAssociateOnClickItem_MEDIA:
            widget->setExpanded(!widget->isExpanded());
            break;
        default:
            qDebug() << "associateAction no action set " << associateAction;
        }
    }
}


/***************************************************************************
 *              class   UBTeacherGuidePageZeroEditionWidget                *
 ***************************************************************************/
UBTeacherGuidePageZeroWidget::UBTeacherGuidePageZeroWidget(QWidget* parent, const char* name): QWidget(parent)
  , mpLayout(NULL)
  , mpButtonTitleLayout(NULL)
  , mpModePushButton(NULL)
  , mpPageNumberLabel(NULL)
  , mpSessionTitle(NULL)
  , mpSeparatorSessionTitle(NULL)
  , mpAuthorsLabel(NULL)
  , mpAuthors(NULL)
  , mpSeparatorAuthors(NULL)
  , mpCreationLabel(NULL)
  , mpLastModifiedLabel(NULL)
  , mpGoalsLabel(NULL)
  , mpGoals(NULL)
  , mpSeparatorGoals(NULL)
  , mpIndexLabel(NULL)
  , mpKeywordsLabel(NULL)
  , mpKeywords(NULL)
  , mpSchoolLevelItemLabel(NULL)
  , mpSchoolLevelBox(NULL)
  , mpSchoolLevelValueLabel(NULL)
  , mpSchoolBranchItemLabel(NULL)
  , mpSchoolBranchBox(NULL)
  , mpSchoolBranchValueLabel(NULL)
  , mpSchoolTypeItemLabel(NULL)
  , mpSchoolTypeBox(NULL)
  , mpSchoolTypeValueLabel(NULL)
  , mpSeparatorIndex(NULL)
  , mpLicenceLabel(NULL)
  , mpLicenceBox(NULL)
  , mpLicenceIcon(NULL)
  , mpLicenceLayout(NULL)
  , mpSceneItemSessionTitle(NULL)
{
    setObjectName(name);
    QString chapterStyle("QLabel {font-size:16px; font-weight:bold;}");

    mpLayout = new QVBoxLayout(this);
    mpPageNumberLabel = new QLabel(this);
    mpPageNumberLabel->setAlignment(Qt::AlignRight);
    mpPageNumberLabel->setObjectName("UBTGPageNumberLabel");
    mpPageNumberLabel->setText(tr("Page 0"));
    mpLayout->addWidget(mpPageNumberLabel);

    mpButtonTitleLayout = new QHBoxLayout(0);

    mpModePushButton = new QPushButton(this);
    mpModePushButton->setIcon(QIcon(":images/pencil.svg"));
    mpModePushButton->setMaximumWidth(32);
    mpModePushButton->installEventFilter(this);
    mpButtonTitleLayout->addWidget(mpModePushButton);
    connect(mpModePushButton,SIGNAL(clicked()),this,SLOT(switchToMode()));

    mpSessionTitle = new UBTGAdaptableText(0,this,"UBTGSessionTitle");
    mpSessionTitle->setPlaceHolderText(tr("Type session title here ..."));
    mpButtonTitleLayout->addWidget(mpSessionTitle);

    mpLayout->addLayout(mpButtonTitleLayout);

    mpSeparatorSessionTitle = new QFrame(this);
    mpSeparatorSessionTitle->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorSessionTitle->setObjectName("UBTGSeparator");
    mpLayout->addWidget(mpSeparatorSessionTitle);

    mpAuthorsLabel = new QLabel(this);
    mpAuthorsLabel->setObjectName("UBTGZeroPageEditionModeTitle");
    mpAuthorsLabel->setText(tr("Author(s)"));
    mpAuthorsLabel->setStyleSheet(chapterStyle);
    mpLayout->addWidget(mpAuthorsLabel);

    mpAuthors = new UBTGAdaptableText(0,this);
    mpAuthors->setObjectName("UBTGZeroPageInputText");
    mpAuthors->setPlaceHolderText(tr("Type authors here ..."));
    mpLayout->addWidget(mpAuthors);

    mpCreationLabel = new QLabel(this);
    mpCreationLabel->setObjectName("UBTGZeroPageDateLabel");
    mpLayout->addWidget(mpCreationLabel);

    mpLastModifiedLabel = new QLabel(this);
    mpLastModifiedLabel->setObjectName("UBTGZeroPageDateLabel");
    mpLayout->addWidget(mpLastModifiedLabel);

    mpSeparatorAuthors = new QFrame(this);
    mpSeparatorAuthors->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorAuthors->setObjectName("UBTGSeparator");
    mpLayout->addWidget(mpSeparatorAuthors);

    mpGoalsLabel = new QLabel(this);
    mpGoalsLabel->setObjectName("UBTGZeroPageEditionModeTitle");
    mpGoalsLabel->setText(tr("Goal(s)"));
    mpGoalsLabel->setStyleSheet(chapterStyle);
    mpLayout->addWidget(mpGoalsLabel);

    mpGoals = new UBTGAdaptableText(0,this);
    mpGoals->setObjectName("UBTGZeroPageInputText");
    mpGoals->setPlaceHolderText(tr("Type goals here..."));
    mpLayout->addWidget(mpGoals);

    mpSeparatorGoals = new QFrame(this);
    mpSeparatorGoals->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorGoals->setObjectName("UBTGSeparator");
    mpLayout->addWidget(mpSeparatorGoals);

    mpIndexLabel = new QLabel(this);
    mpIndexLabel->setObjectName("UBTGZeroPageEditionModeTitle");
    mpIndexLabel->setText(tr("Resource indexing"));
    mpIndexLabel->setStyleSheet(chapterStyle);
    mpLayout->addWidget(mpIndexLabel);

    mpKeywordsLabel = new QLabel(this);
    mpKeywordsLabel->setObjectName("UBTGZeroPageItemLabel");
    mpKeywordsLabel->setText(tr("Keywords:"));
    mpKeywordsLabel->setStyleSheet(chapterStyle);
    mpLayout->addWidget(mpKeywordsLabel);
    mpKeywords = new UBTGAdaptableText(0,this);
    mpKeywords->setPlaceHolderText(tr("Type keywords here ..."));
    mpLayout->addWidget(mpKeywords);

    mpSchoolLevelItemLabel = new QLabel(this);
    mpSchoolLevelItemLabel->setObjectName("UBTGZeroPageItemLabel");
    mpSchoolLevelItemLabel->setText(tr("Level:"));
    mpSchoolLevelItemLabel->setStyleSheet(chapterStyle);
    mpLayout->addWidget(mpSchoolLevelItemLabel);
    mpSchoolLevelBox = new QComboBox(this);
    mpSchoolLevelBox->setObjectName("DockPaletteWidgetComboBox");
    connect(mpSchoolLevelBox,SIGNAL(currentIndexChanged(QString)),this,SLOT(onSchoolLevelChanged(QString)));
    mpLayout->addWidget(mpSchoolLevelBox);
    mpSchoolLevelValueLabel = new QLabel(this);
    mpLayout->addWidget(mpSchoolLevelValueLabel);

    mpSchoolBranchItemLabel = new QLabel(this);
    mpSchoolBranchItemLabel->setObjectName("UBTGZeroPageItemLabel");
    mpSchoolBranchItemLabel->setText(tr("Branch:"));
    mpSchoolBranchItemLabel->setStyleSheet(chapterStyle);
    mpLayout->addWidget(mpSchoolBranchItemLabel);
    mpSchoolBranchBox = new QComboBox(this);
    mpSchoolBranchBox->setObjectName("DockPaletteWidgetComboBox");
    mpLayout->addWidget(mpSchoolBranchBox);
    mpSchoolBranchValueLabel = new QLabel(this);
    mpLayout->addWidget(mpSchoolBranchValueLabel);

    mpSchoolTypeItemLabel = new QLabel(this);
    mpSchoolTypeItemLabel->setObjectName("UBTGZeroPageItemLabel");
    mpSchoolTypeItemLabel->setText(tr("Type:"));
    mpSchoolTypeItemLabel->setStyleSheet(chapterStyle);
    mpLayout->addWidget(mpSchoolTypeItemLabel);
    mpSchoolTypeBox = new QComboBox(this);
    mpSchoolTypeBox->setObjectName("DockPaletteWidgetComboBox");
    mpLayout->addWidget(mpSchoolTypeBox);
    mpSchoolTypeValueLabel = new QLabel(this);
    mpLayout->addWidget(mpSchoolTypeValueLabel);

    mpSeparatorIndex = new QFrame(this);
    mpSeparatorIndex->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorIndex->setObjectName("UBTGSeparator");
    mpLayout->addWidget(mpSeparatorIndex);

    mpLicenceLabel = new QLabel(this);
    mpLicenceLabel->setObjectName("UBTGZeroPageItemLabel");
    mpLicenceLabel->setText(tr("Licence"));
    mpLicenceLabel->setStyleSheet(chapterStyle);
    mpLayout->addWidget(mpLicenceLabel);
    mpLicenceBox = new QComboBox(this);
    mpLicenceBox->setObjectName("DockPaletteWidgetComboBox");
    mpLayout->addWidget(mpLicenceBox);
    mpLicenceLayout = new QHBoxLayout(0);
    mpLicenceIcon = new QLabel(this);
    mpLicenceLayout->addWidget(mpLicenceIcon);
    mpLicenceValueLabel = new QLabel(this);
    mpLicenceLayout->addWidget(mpLicenceValueLabel);
    mpLayout->addLayout(mpLicenceLayout);
    mpLayout->addStretch(1);

    setLayout(mpLayout);
    connect(UBApplication::boardController,SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));
    fillComboBoxes();
}

UBTeacherGuidePageZeroWidget::~UBTeacherGuidePageZeroWidget()
{
    DELETEPTR(mpPageNumberLabel);
    DELETEPTR(mpSessionTitle);
    DELETEPTR(mpSeparatorSessionTitle);
    DELETEPTR(mpAuthorsLabel);
    DELETEPTR(mpAuthors);
    DELETEPTR(mpSeparatorAuthors);
    DELETEPTR(mpCreationLabel);
    DELETEPTR(mpLastModifiedLabel);
    DELETEPTR(mpGoalsLabel);
    DELETEPTR(mpGoals);
    DELETEPTR(mpSeparatorGoals);
    DELETEPTR(mpIndexLabel);
    DELETEPTR(mpKeywordsLabel);
    DELETEPTR(mpKeywords);
    DELETEPTR(mpSchoolLevelItemLabel);
    DELETEPTR(mpSchoolLevelBox);
    DELETEPTR(mpSchoolBranchItemLabel);
    DELETEPTR(mpSchoolBranchBox);
    DELETEPTR(mpSchoolTypeItemLabel);
    DELETEPTR(mpSchoolTypeBox);
    DELETEPTR(mpSeparatorIndex);
    DELETEPTR(mpLicenceLabel);
    DELETEPTR(mpLicenceBox);
    DELETEPTR(mpLicenceValueLabel);
    DELETEPTR(mpLicenceIcon);
    DELETEPTR(mpModePushButton);
    DELETEPTR(mpLicenceLayout);
    DELETEPTR(mpButtonTitleLayout);
    DELETEPTR(mpLayout);
}

bool UBTeacherGuidePageZeroWidget::eventFilter(QObject* object, QEvent* event)
{
    Q_UNUSED(object);
    if(event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverMove || event->type() == QEvent::HoverLeave)
        return true;
    return false;
}

void UBTeacherGuidePageZeroWidget::fillComboBoxes()
{
    QString parametersConfigFilePath = UBSettings::settings()->applicationCustomizationDirectory() + "/teacherGuide/indexingParameters.xml";
    QFile parametersFile(parametersConfigFilePath);
    if(!parametersFile.exists()){
        qCritical() << "UBTeacherGuidePageZeroEditionWidget fillComboBoxes file not found " << parametersConfigFilePath;
        return;
    }

    parametersFile.open(QFile::ReadOnly);

    QDomDocument doc;
    doc.setContent(parametersFile.readAll());
    QDomElement rootElement = doc.elementsByTagName("teacherGuide").at(0).toElement();

    QDomNodeList subjects = rootElement.elementsByTagName("subjects");
    for(int baseLevelCounter = 0; baseLevelCounter < subjects.count(); baseLevelCounter += 1){
        QDomNode subjectsForBaseLevel = subjects.at(baseLevelCounter);
        QDomNodeList subjectsList = subjectsForBaseLevel.childNodes();
        QStringList subjectsRelatedToBaseLevel;
        for(int j = 0; j < subjectsList.count(); j += 1){
            subjectsRelatedToBaseLevel.append(subjectsList.at(j).toElement().attribute("label"));
        }
        mSubjects.insert(subjectsForBaseLevel.toElement().attribute("baseLevel"),subjectsRelatedToBaseLevel);
    }

    QDomNodeList gradeLevels = rootElement.elementsByTagName("gradeLevels").at(0).childNodes();
    for(int i=0; i<gradeLevels.count();i+=1){
        mGradeLevelsMap.insert(gradeLevels.at(i).toElement().attribute("label"),gradeLevels.at(i).toElement().attribute("baseLevel"));
        mpSchoolLevelBox->addItem(gradeLevels.at(i).toElement().attribute("label"));
    }


    QDomNodeList types = rootElement.elementsByTagName("types").at(0).childNodes();
    for(int i=0; i<types.count();i+=1)
        mpSchoolTypeBox->addItem(types.at(i).toElement().attribute("label"));

    parametersFile.close();

    QStringList licences;
    licences << tr("Attribution CC BY") << tr("Attribution-NoDerivs CC BY-ND") << tr("Attribution-ShareAlike CC BY-SA") << tr("Attribution-NonCommercial CC BY-NC") << tr("Attribution-NonCommercial-NoDerivs CC BY-NC-ND") << tr("Attribution-NonCommercial-ShareAlike CC BY-NC-SA") << tr("Public domain") << tr("Copyright");
    mpLicenceBox->addItems(licences);
    QStringList licenceIconList;
    licenceIconList << ":images/licenses/ccby.png" << ":images/licenses/ccbynd.png" << ":images/licenses/ccbysa.png" << ":images/licenses/ccbync.png" << ":images/licenses/ccbyncnd.png" << ":images/licenses/ccbyncsa.png";
    for(int i = 0; i < licenceIconList.count(); i+=1)
        mpLicenceBox->setItemData(i,licenceIconList.at(i));
}

void UBTeacherGuidePageZeroWidget::onSchoolLevelChanged(QString schoolLevel)
{
    QStringList subjects = mSubjects.value(mGradeLevelsMap.value(schoolLevel));
    mpSchoolBranchBox->clear();
    if(subjects.count()){
        mpSchoolBranchItemLabel->setEnabled(true);
        mpSchoolBranchBox->setEnabled(true);
        mpSchoolBranchBox->addItems(subjects);
    }
    else{
        mpSchoolBranchItemLabel->setDisabled(true);
        mpSchoolBranchBox->setDisabled(true);
    }
}

void UBTeacherGuidePageZeroWidget::onActiveSceneChanged()
{
    UBDocumentProxy* documentProxy = UBApplication::boardController->activeDocument();
    if(documentProxy && UBApplication::boardController->currentPage() == 0){
        QDateTime creationDate = documentProxy->documentDate();
        mpCreationLabel->setText(tr("Created the:\n") + creationDate.toString(Qt::SystemLocaleShortDate));
        QDateTime updatedDate = documentProxy->lastUpdate();
        mpLastModifiedLabel->setText(tr("Updated the:\n") + updatedDate.toString(Qt::SystemLocaleShortDate));
        loadData();
        UBApplication::boardController->activeScene()->textForObjectName(mpSessionTitle->text());
    }
}

void UBTeacherGuidePageZeroWidget::hideEvent ( QHideEvent * event )
{
    persistData();
    QWidget::hideEvent(event);
}

void UBTeacherGuidePageZeroWidget::loadData()
{
    UBDocumentProxy* documentProxy = UBApplication::boardController->activeDocument();
    mpSessionTitle->setText(documentProxy->metaData(UBSettings::sessionTitle).toString());
    mpAuthors->setText(documentProxy->metaData(UBSettings::sessionAuthors).toString());
    mpGoals->setText(documentProxy->metaData(UBSettings::sessionGoals).toString());
    mpKeywords->setText(documentProxy->metaData(UBSettings::sessionKeywords).toString());

    int currentIndex = mpSchoolLevelBox->findText(documentProxy->metaData(UBSettings::sessionGradeLevel).toString());
    mpSchoolLevelBox->setCurrentIndex((currentIndex!=-1) ? currentIndex : 0);

    currentIndex = mpSchoolBranchBox->findText(documentProxy->metaData(UBSettings::sessionBranch).toString());
    mpSchoolBranchBox->setCurrentIndex((currentIndex!=-1) ? currentIndex : 0);

    currentIndex = mpSchoolTypeBox->findText(documentProxy->metaData(UBSettings::sessionType).toString());
    mpSchoolTypeBox->setCurrentIndex((currentIndex!=-1) ? currentIndex : 0);

    currentIndex = mpLicenceBox->findText(documentProxy->metaData(UBSettings::sessionLicence).toString());
    mpLicenceBox->setCurrentIndex((currentIndex!=-1) ? currentIndex : 0);
}

void UBTeacherGuidePageZeroWidget::persistData()
{
    UBDocumentProxy* documentProxy = UBApplication::boardController->activeDocument();
    documentProxy->setMetaData(UBSettings::sessionTitle,mpSessionTitle->text());
    documentProxy->setMetaData(UBSettings::sessionAuthors, mpAuthors->text());
    documentProxy->setMetaData(UBSettings::sessionGoals,mpGoals->text());
    documentProxy->setMetaData(UBSettings::sessionKeywords,mpKeywords->text());
    documentProxy->setMetaData(UBSettings::sessionGradeLevel,mpSchoolLevelBox->currentText());
    documentProxy->setMetaData(UBSettings::sessionBranch,mpSchoolBranchBox->currentText());
    documentProxy->setMetaData(UBSettings::sessionType,mpSchoolTypeBox->currentText());
    documentProxy->setMetaData(UBSettings::sessionLicence,mpLicenceBox->currentText());
}

void UBTeacherGuidePageZeroWidget::switchToMode(tUBTGZeroPageMode mode)
{
    if(mode == tUBTGZeroPageMode_EDITION){
        QString inputStyleSheet("QTextEdit { background: white; border-radius: 10px; border: 2px;}");
        mpModePushButton->hide();
        mpSessionTitle->setReadOnly(false);
        mpSessionTitle->setStyleSheet(inputStyleSheet);
        QFont titleFont(QApplication::font().family(),11,-1);
        mpSessionTitle->document()->setDefaultFont(titleFont);
        mpAuthors->setReadOnly(false);
        mpAuthors->setStyleSheet(inputStyleSheet);
        mpGoals->setReadOnly(false);
        mpGoals->setStyleSheet(inputStyleSheet);
        mpKeywords->setReadOnly(false);
        mpKeywords->setStyleSheet(inputStyleSheet);
        mpSchoolLevelValueLabel->hide();
        mpSchoolLevelBox->show();
        mpSchoolBranchValueLabel->hide();
        mpSchoolBranchBox->show();
        mpSchoolTypeValueLabel->hide();
        mpSchoolTypeBox->show();
        mpLicenceIcon->hide();
        mpLicenceValueLabel->hide();
        mpLicenceBox->show();
    }
    else{
        QString inputStyleSheet("QTextEdit { background: transparent; border: none;}");
        mpModePushButton->show();
        mpSessionTitle->setReadOnly(true);
        UBApplication::boardController->activeScene()->textForObjectName(mpSessionTitle->text());
        mpSessionTitle->setStyleSheet(inputStyleSheet);
        mpSessionTitle->setTextColor(QColor(Qt::black));
        QFont titleFont(QApplication::font().family(),14,1);
        mpSessionTitle->document()->setDefaultFont(titleFont);
        mpAuthors->setReadOnly(true);
        mpAuthors->setStyleSheet(inputStyleSheet);
        mpAuthors->setTextColor(QColor(Qt::black));
        mpGoals->setReadOnly(true);
        mpGoals->setStyleSheet(inputStyleSheet);
        mpGoals->setTextColor(QColor(Qt::black));
        mpKeywords->setReadOnly(true);
        mpKeywords->setStyleSheet(inputStyleSheet);
        mpKeywords->setTextColor(QColor(Qt::black));
        mpSchoolLevelValueLabel->setText(mpSchoolLevelBox->currentText());
        mpSchoolLevelValueLabel->show();
        mpSchoolLevelBox->hide();
        mpSchoolBranchValueLabel->setText(mpSchoolBranchBox->currentText());
        mpSchoolBranchValueLabel->show();
        mpSchoolBranchBox->hide();
        mpSchoolTypeValueLabel->setText(mpSchoolTypeBox->currentText());
        mpSchoolTypeValueLabel->show();
        mpSchoolTypeBox->hide();
        mpLicenceValueLabel->setText(mpLicenceBox->currentText());
        QString licenceIconPath = mpLicenceBox->itemData(mpLicenceBox->currentIndex()).toString();
        if(!licenceIconPath.isEmpty()){
            mpLicenceIcon->setPixmap(QPixmap(licenceIconPath));
            mpLicenceIcon->show();
        }
        mpLicenceValueLabel->show();
        mpLicenceBox->hide();
        persistData();
    }
    update();
}

QVector<tUBGEElementNode*> UBTeacherGuidePageZeroWidget::getData()
{
    QVector<tUBGEElementNode*>result;
    tUBGEElementNode* elementNode = new tUBGEElementNode();
    elementNode->type = "sessionTitle";
    elementNode->attributes.insert("value",mpSessionTitle->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->type = "authors";
    elementNode->attributes.insert("value",mpAuthors->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->type = "creationDate";
    elementNode->attributes.insert("value",mpCreationLabel->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->type = "lastModifiedDate";
    elementNode->attributes.insert("value",mpLastModifiedLabel->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->type = "goals";
    elementNode->attributes.insert("value",mpGoals->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->type = "keywords";
    elementNode->attributes.insert("value",mpKeywords->text());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->type = "schoolLevel";
    elementNode->attributes.insert("value",mpSchoolLevelBox->currentText());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->type = "schoolBranch";
    elementNode->attributes.insert("value",mpSchoolBranchBox->currentText());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->type = "schoolType";
    elementNode->attributes.insert("value",mpSchoolTypeBox->currentText());
    result << elementNode;

    elementNode = new tUBGEElementNode();
    elementNode->type = "licence";
    elementNode->attributes.insert("value",mpLicenceBox->currentText());
    result << elementNode;
    return result;
}

/***************************************************************************
 *                    class    UBTeacherGuideWidget                        *
 ***************************************************************************/
UBTeacherGuideWidget::UBTeacherGuideWidget(QWidget* parent, const char* name): QStackedWidget(parent)
  , mpPageZeroWidget(NULL)
  , mpEditionWidget(NULL)
  , mpPresentationWidget(NULL)
{
    setObjectName(name);
    if(UBSettings::settings()->teacherGuidePageZeroActivated->get().toBool()){
        mpPageZeroWidget = new UBTeacherGuidePageZeroWidget(this);
        addWidget(mpPageZeroWidget);
    }
    if(UBSettings::settings()->teacherGuideLessonPagesActivated->get().toBool()){
        mpEditionWidget = new UBTeacherGuideEditionWidget(this);
        addWidget(mpEditionWidget);
        mpPresentationWidget = new UBTeacherGuidePresentationWidget(this);
        addWidget(mpPresentationWidget);
    }

    connect(UBApplication::boardController->controlView(),SIGNAL(clickOnBoard()),this,SLOT(showPresentationMode()));
    connectToStylusPalette();
    connect(UBApplication::boardController,SIGNAL(activeSceneChanged()),this,SLOT(onActiveSceneChanged()));
}


UBTeacherGuideWidget::~UBTeacherGuideWidget()
{
    DELETEPTR(mpPageZeroWidget);
    DELETEPTR(mpEditionWidget);
    DELETEPTR(mpPresentationWidget);
}


void UBTeacherGuideWidget::onActiveSceneChanged()
{
    if(UBApplication::boardController->currentPage() == 0){
        setCurrentWidget(mpPageZeroWidget);
        mpPageZeroWidget->switchToMode(tUBTGZeroPageMode_EDITION);
    }else
        setCurrentWidget(mpEditionWidget);

}

void UBTeacherGuideWidget::connectToStylusPalette()
{
    if(UBApplication::boardController->paletteManager())
        connect(UBApplication::boardController->paletteManager()->stylusPalette(),SIGNAL(itemOnActionPaletteChanged()),this,SLOT(showPresentationMode()));
    else
        QTimer::singleShot(100,this,SLOT(connectToStylusPalette()));
}

void UBTeacherGuideWidget::showPresentationMode()
{
    if(currentWidget()==mpPageZeroWidget){
        mCurrentData = mpPageZeroWidget->getData();
        mpPageZeroWidget->switchToMode(tUBTGZeroPageMode_PRESENTATION);
    }
    else if(currentWidget()==mpEditionWidget){
        mCurrentData = mpEditionWidget->getData();
        mpPresentationWidget->showData(mCurrentData);
        setCurrentWidget(mpPresentationWidget);
    }
}

void UBTeacherGuideWidget::changeMode()
{
    if(currentWidget() == mpEditionWidget)
        setCurrentWidget(mpPresentationWidget);
    else
        setCurrentWidget(mpEditionWidget);

}
