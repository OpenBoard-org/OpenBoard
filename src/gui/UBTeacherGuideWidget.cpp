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

#include "core/UBApplication.h"
#include "core/UBPersistenceManager.h"

#include "globals/UBGlobals.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBBoardPaletteManager.h"

#include "gui/UBStylusPalette.h"
#include "gui/UBActionPalette.h"

#include "web/UBWebController.h"

#include "document/UBDocumentProxy.h"
#include "document/UBDocumentController.h"

#define UBTG_SEPARATOR_FIXED_HEIGHT 3


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
  , mpTreeDelegate(NULL)
{
    setObjectName(name);

    mpLayout = new QVBoxLayout(this);
    mpPageNumberLabel = new QLabel(this);
    mpPageNumberLabel->setAlignment(Qt::AlignRight);
    mpPageNumberLabel->setObjectName("UBTGEditionPageNumberLabel");
    mpLayout->addWidget(mpPageNumberLabel);
    // tree basic configuration
    mpDocumentTitle = new QLabel(this);
    mpDocumentTitle->setText("Document title");
    mpDocumentTitle->setObjectName("UBTGEditionDocumentTitle");
    mpLayout->addWidget(mpDocumentTitle);

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

    mpTreeDelegate = new UBTGWidgetTreeDelegate();

    mpRootWidgetItem = mpTreeWidget->invisibleRootItem();
    //mpTreeWidget->setItemDelegate(mpTreeDelegate);
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
    DELETEPTR(mpTreeDelegate);
    DELETEPTR(mpTreeWidget)
    DELETEPTR(mpLayout);
}

void UBTeacherGuideEditionWidget::showEvent(QShowEvent* event)
{
    mpPageTitle->setFocus();
    mpComment->setFocus();
    setFocus();
    QWidget::showEvent(event);
}

void UBTeacherGuideEditionWidget::onActiveSceneChanged()
{
    cleanData();
    mpPageNumberLabel->setText(tr("Page: %0").arg(UBApplication::boardController->activeSceneIndex() + 1));
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
        tUBGEElementNode* node = dynamic_cast<iUBTGSavableData*>(mpTreeWidget->itemWidget(widgetItem,0))->saveData();
        if(node)
            result << node;
    }
    return result;
}

void UBTeacherGuideEditionWidget::onAddItemClicked(QTreeWidgetItem* widget, int column)
{
    int addSubItemWidgetType = widget->data(column,Qt::UserRole).toInt();
    if(column == 0 && addSubItemWidgetType != eUBTGAddSubItemWidgetType_None){
        QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(widget);
        newWidgetItem->setData(column,Qt::UserRole,eUBTGAddSubItemWidgetType_None);
        newWidgetItem->setData(1,Qt::UserRole,eUBTGAddSubItemWidgetType_None);
        newWidgetItem->setIcon(1,QIcon(":images/close.svg"));
        switch(addSubItemWidgetType)
        {
        case eUBTGAddSubItemWidgetType_Action:
            mpTreeWidget->setItemWidget(newWidgetItem,0,new UBTGActionWidget(widget));
            break;
        case eUBTGAddSubItemWidgetType_Media:
            mpTreeWidget->setItemWidget(newWidgetItem,0,new UBTGMediaWidget(widget));
            break;
        case eUBTGAddSubItemWidgetType_Url:
            mpTreeWidget->setItemWidget(newWidgetItem,0,new UBTGUrlWidget());
            break;
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
    mpPageNumberLabel->setObjectName("UBTGPresentationPageNumberLabel");

    mpLayout->addWidget(mpPageNumberLabel);

    mpButtonTitleLayout = new QHBoxLayout(0);

    mpModePushButton = new QPushButton(this);
    mpModePushButton->setIcon(QIcon(":images/pencil.svg"));
    mpModePushButton->setMaximumWidth(32);

    connect(mpModePushButton,SIGNAL(clicked()),parentWidget(),SLOT(changeMode()));

    mpDocumentTitle = new QLabel(this);
    mpDocumentTitle->setObjectName("UBTGPresentationDocumentTitle");
    mpDocumentTitle->setText(tr("Document title"));

    mpButtonTitleLayout->addWidget(mpModePushButton);
    mpButtonTitleLayout->addWidget(mpDocumentTitle);

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

    mpTreeWidget = new QTreeWidget(this);
    mpLayout->addWidget(mpTreeWidget);

    mpRootWidgetItem = mpTreeWidget->invisibleRootItem();
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
    mpPageNumberLabel->setText(tr("Page: %0").arg(UBApplication::boardController->activeSceneIndex() + 1));
}

void UBTeacherGuidePresentationWidget::createMediaButtonItem()
{
    if(!mpMediaSwitchItem){
        //create the media button
        mpMediaSwitchItem = new QTreeWidgetItem(mpRootWidgetItem);
        //mpMediaSwitchItem->setIcon(0,QIcon(":images/plus.svg"));
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
UBTeacherGuidePageZeroEditionWidget::UBTeacherGuidePageZeroEditionWidget(QWidget* parent, const char* name): QWidget(parent)
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
{
    setObjectName(name);

    mpLayout = new QVBoxLayout(this);
    mpPageNumberLabel = new QLabel(this);
    mpPageNumberLabel->setAlignment(Qt::AlignRight);
    mpPageNumberLabel->setObjectName("UBTGPresentationPageNumberLabel");
    mpPageNumberLabel->setText(tr("Page 0"));
    mpLayout->addWidget(mpPageNumberLabel);

    mpButtonTitleLayout = new QHBoxLayout(0);

    mpModePushButton = new QPushButton(this);
    mpModePushButton->setIcon(QIcon(":images/pencil.svg"));
    mpModePushButton->setMaximumWidth(32);
    mpButtonTitleLayout->addWidget(mpModePushButton);
    connect(mpModePushButton,SIGNAL(clicked()),this,SLOT(switchToMode()));

    mpSessionTitle = new UBTGAdaptableText(0,this);
    mpSessionTitle->setPlaceHolderText(tr("Type session title here ..."));
    mpSessionTitle->setObjectName("UBTGEditionModeSessionTitle");
    mpButtonTitleLayout->addWidget(mpSessionTitle);

    mpLayout->addLayout(mpButtonTitleLayout);

    mpSeparatorSessionTitle = new QFrame(this);
    mpSeparatorSessionTitle->setFixedHeight(UBTG_SEPARATOR_FIXED_HEIGHT);
    mpSeparatorSessionTitle->setObjectName("UBTGSeparator");
    mpLayout->addWidget(mpSeparatorSessionTitle);

    mpAuthorsLabel = new QLabel(this);
    mpAuthorsLabel->setObjectName("UBTGZeroPageEditionModeTitle");
    mpAuthorsLabel->setText(tr("Author(s)"));
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
    mpLayout->addWidget(mpIndexLabel);

    mpKeywordsLabel = new QLabel(this);
    mpKeywordsLabel->setObjectName("UBTGZeroPageItemLabel");
    mpKeywordsLabel->setText(tr("Keywords:"));
    mpLayout->addWidget(mpKeywordsLabel);
    mpKeywords = new UBTGAdaptableText(0,this);
    mpKeywords->setPlaceHolderText(tr("Type keywords here ..."));
    mpLayout->addWidget(mpKeywords);

    mpSchoolLevelItemLabel = new QLabel(this);
    mpSchoolLevelItemLabel->setObjectName("UBTGZeroPageItemLabel");
    mpSchoolLevelItemLabel->setText(tr("Level:"));
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
    mpLayout->addWidget(mpSchoolBranchItemLabel);
    mpSchoolBranchBox = new QComboBox(this);
    mpSchoolBranchBox->setObjectName("DockPaletteWidgetComboBox");
    mpLayout->addWidget(mpSchoolBranchBox);
    mpSchoolBranchValueLabel = new QLabel(this);
    mpLayout->addWidget(mpSchoolBranchValueLabel);

    mpSchoolTypeItemLabel = new QLabel(this);
    mpSchoolTypeItemLabel->setObjectName("UBTGZeroPageItemLabel");
    mpSchoolTypeItemLabel->setText(tr("Type:"));
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
    mpLicenceLabel->setText(tr("Licence:"));
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
    connect(UBApplication::boardController,SIGNAL(activeSceneChanged()), this, SLOT(onActiveSceneChanged()));
    fillComboBoxes();
}

UBTeacherGuidePageZeroEditionWidget::~UBTeacherGuidePageZeroEditionWidget()
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

void UBTeacherGuidePageZeroEditionWidget::fillComboBoxes()
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
}

void UBTeacherGuidePageZeroEditionWidget::onSchoolLevelChanged(QString schoolLevel)
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

void UBTeacherGuidePageZeroEditionWidget::onActiveSceneChanged()
{
    UBDocumentProxy* documentProxy = UBApplication::documentController ? UBApplication::documentController->getCurrentDocument() : 0;
    if(UBApplication::documentController && UBApplication::boardController->activeSceneIndex() == 0){
        QDateTime creationDate = documentProxy->documentDate();
        mpCreationLabel->setText(tr("Created the:") + creationDate.toString(Qt::SystemLocaleShortDate));
        QDateTime updatedDate = documentProxy->lastUpdate();
        mpLastModifiedLabel->setText(tr("Updated the:") + updatedDate.toString(Qt::SystemLocaleShortDate));
    }
}


void UBTeacherGuidePageZeroEditionWidget::switchToMode(tUBTGZeroPageMode mode)
{
    if(mode == tUBTGZeroPageMode_EDITION){
        mpModePushButton->hide();
        mpSessionTitle->setReadOnly(false);
        mpAuthors->setReadOnly(false);
        mpGoals->setReadOnly(false);
        mpKeywords->setReadOnly(false);
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
        mpModePushButton->show();
        mpSessionTitle->setReadOnly(true);
        mpAuthors->setReadOnly(true);
        mpGoals->setReadOnly(true);
        mpKeywords->setReadOnly(true);
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
        QStringList licenceIconList;
        licenceIconList << ":images/licenses/ccby.png" << ":images/licenses/ccbynd.png" << ":images/licenses/ccbysa.png" << ":images/licenses/ccbync.png" << ":images/licenses/ccbyncnd.png" << ":images/licenses/ccbyncsa.png";
        int licenceBoxCurrentIndex = mpLicenceBox->currentIndex();
        // -1 is return if there is no values on the box
        if(licenceBoxCurrentIndex > 0 && licenceBoxCurrentIndex < licenceIconList.count()){
            mpLicenceIcon->setPixmap(licenceIconList.at(licenceBoxCurrentIndex));
            mpLicenceIcon->show();
        }
        mpLicenceValueLabel->show();
        mpLicenceBox->hide();
    }
}

QVector<tUBGEElementNode*> UBTeacherGuidePageZeroEditionWidget::getData()
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
  , mpPageZeroEditonWidget(NULL)
  , mpEditionWidget(NULL)
  , mpPresentationWidget(NULL)
{
    setObjectName(name);

    mpPageZeroEditonWidget = new UBTeacherGuidePageZeroEditionWidget(this);
    addWidget(mpPageZeroEditonWidget);
    setCurrentWidget(mpPageZeroEditonWidget);
    //    mpEditionWidget = new UBTeacherGuideEditionWidget(this);
//    addWidget(mpEditionWidget);
//    mpPresentationWidget = new UBTeacherGuidePresentationWidget(this);
//    addWidget(mpPresentationWidget);
//    setCurrentWidget(mpPresentationWidget);


    connect(UBApplication::boardController->controlView(),SIGNAL(clickOnBoard()),this,SLOT(showPresentationMode()));
    connectToStylusPalette();
}


UBTeacherGuideWidget::~UBTeacherGuideWidget()
{
    DELETEPTR(mpEditionWidget);
    DELETEPTR(mpPresentationWidget);
}

void UBTeacherGuideWidget::connectToStylusPalette()
{
    if(UBApplication::boardController->paletteManager())
        connect(UBApplication::boardController->paletteManager()->stylusPalette(),SIGNAL(itemOnActionPaletteChanged()),this,SLOT(showPresentationMode()));
    else
        QTimer::singleShot(500,this,SLOT(connectToStylusPalette()));
}

void UBTeacherGuideWidget::showPresentationMode()
{
    if(currentWidget()==mpPageZeroEditonWidget){
        mCurrentData = mpPageZeroEditonWidget->getData();
        mpPageZeroEditonWidget->switchToMode(tUBTGZeroPageMode_PRESENTATION);
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
