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

#include "UBTeacherGuideWidget.h"

#include "core/UBApplication.h"

#include "globals/UBGlobals.h"

#include "board/UBBoardController.h"
#include "board/UBBoardView.h"
#include "board/UBBoardPaletteManager.h"

#include "gui/UBStylusPalette.h"
#include "gui/UBActionPalette.h"

#include "web/UBWebController.h"

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
  , mpPageTitle(NULL)
  , mpComment(NULL)
  , mpTreeWidget(NULL)
  , mpRootWidgetItem(NULL)
  , mpAddAnActionItem(NULL)
  , mpAddAMediaItem(NULL)
  , mpAddALinkItem(NULL)
{
    setObjectName(name);

    mpLayout = new QVBoxLayout(this);
    // tree basic configuration
    mpDocumentTitle = new QLabel(this);
    mpDocumentTitle->setText("title document");
    mpDocumentTitle->setStyleSheet( "QWidget {background-color: white}");
    mpLayout->addWidget(mpDocumentTitle);

    mpPageTitle = new UBTGAdaptableText(0,this);
    mpLayout->addWidget(mpPageTitle);

    mpComment = new UBTGAdaptableText(0,this);
    mpLayout->addWidget(mpComment);

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
    mpTreeWidget->header()->setDefaultSectionSize(32);

    connect(mpTreeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(onAddItemClicked(QTreeWidgetItem*,int)));

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
    DELETEPTR(mpPageTitle);
    DELETEPTR(mpComment);
    DELETEPTR(mpAddAnActionItem);
    DELETEPTR(mpAddAMediaItem);
    DELETEPTR(mpAddALinkItem);
    DELETEPTR(mpTreeWidget)
    DELETEPTR(mpLayout);
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
    pageTitle->attributes.insert("value",mpPageTitle->toPlainText());
    result << pageTitle;

    tUBGEElementNode* comment = new tUBGEElementNode();
    comment->type = "comment";
    comment->attributes.insert("value",mpComment->toPlainText());
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
  , mpDocumentTile(NULL)
  , mpModePushButton(NULL)
  , mpTreeWidget(NULL)
  , mpRootWidgetItem(NULL)
  , mpMediaSwitchItem(NULL)
{
    setObjectName(name);
    mpLayout = new QVBoxLayout(this);
    mpButtonTitleLayout = new QHBoxLayout(0);

    mpModePushButton = new QPushButton(this);
    mpModePushButton->setIcon(QIcon(":images/pencil.svg"));
    connect(mpModePushButton,SIGNAL(clicked()),parentWidget(),SLOT(changeMode()));

    mpDocumentTile = new QLabel(this);
    mpDocumentTile->setText("Document title");

    mpButtonTitleLayout->addWidget(mpModePushButton);
    mpButtonTitleLayout->addWidget(mpDocumentTile);

    mpLayout->addLayout(mpButtonTitleLayout);

    mpPageTitle = new UBTGAdaptableText(0,this);
    mpPageTitle->setReadOnly(true);
    mpLayout->addWidget(mpPageTitle);

    mpComment = new UBTGAdaptableText(0,this);
    mpComment->setReadOnly(true);
    mpLayout->addWidget(mpComment);

    mpTreeWidget = new QTreeWidget(this);
    mpLayout->addWidget(mpTreeWidget);

    mpRootWidgetItem = mpTreeWidget->invisibleRootItem();
    mpTreeWidget->setRootIsDecorated(false);
    mpTreeWidget->setIndentation(0);
    mpTreeWidget->setDropIndicatorShown(false);
    mpTreeWidget->header()->close();
    mpTreeWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
//    mpTreeWidget->setColumnCount(1);
    connect(mpTreeWidget,SIGNAL(itemClicked(QTreeWidgetItem*,int)),this,SLOT(onAddItemClicked(QTreeWidgetItem*,int)));

}

UBTeacherGuidePresentationWidget::~UBTeacherGuidePresentationWidget()
{
    DELETEPTR(mpComment);
    DELETEPTR(mpPageTitle);
    DELETEPTR(mpMediaSwitchItem);
    DELETEPTR(mpModePushButton);
    DELETEPTR(mpDocumentTile);
    DELETEPTR(mpButtonTitleLayout);
    DELETEPTR(mpTreeWidget);
    DELETEPTR(mpLayout);
}

void UBTeacherGuidePresentationWidget::createMediaButtonItem()
{
    if(!mpMediaSwitchItem){
        //create the media button
        mpMediaSwitchItem = new QTreeWidgetItem(mpRootWidgetItem);
        mpMediaSwitchItem->setIcon(0,QIcon(":images/plus.svg"));
        mpMediaSwitchItem->setExpanded(false);
        mpMediaSwitchItem->setData(0,tUBTGTreeWidgetItemRole_HasAnAction,tUBTGActionAssociateOnClickItem_EXPAND);
        mpRootWidgetItem->addChild(mpMediaSwitchItem);
    }
}


void UBTeacherGuidePresentationWidget::showData(QVector<tUBGEElementNode*> data)
{
    //tree clean
    QList<QTreeWidgetItem*> itemToRemove = mpRootWidgetItem->takeChildren();
    foreach(QTreeWidgetItem* eachItem, itemToRemove){
        DELETEPTR(eachItem);
    }
    // the mpMediaSwitchItem is deleted by the previous loop but the pointer is not set to zero
    mpMediaSwitchItem = NULL;

    foreach(tUBGEElementNode* element, data){
        if(element->type == "pageTitle")
            mpPageTitle->showText(element->attributes.value("value"));
        else if (element->type == "comment")
            mpComment->showText(element->attributes.value("value"));
        else if(element->type == "action"){
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpRootWidgetItem);
            newWidgetItem->setText(0,element->attributes.value("task"));
            QColor color = element->attributes.value("owner").toInt()?QColor().red():QColor().green();
            newWidgetItem->setTextColor(0,color);
            newWidgetItem->setData(0,tUBTGTreeWidgetItemRole_HasAnAction,tUBTGActionAssociateOnClickItem_NONE);
            mpRootWidgetItem->addChild(newWidgetItem);
        }
        else if(element->type == "media"){
            createMediaButtonItem();
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpMediaSwitchItem);
            newWidgetItem->setText(0,element->attributes.value("title"));
            newWidgetItem->setData(0,tUBTGTreeWidgetItemRole_HasAnAction,tUBTGActionAssociateOnClickItem_MEDIA);
            mpRootWidgetItem->addChild(newWidgetItem);

            QTreeWidgetItem* mediaItem = new QTreeWidgetItem(newWidgetItem);
            mediaItem->setText(0,element->attributes.value("title"));
            mediaItem->setData(0,tUBTGTreeWidgetItemRole_HasAnAction,tUBTGActionAssociateOnClickItem_NONE);
            UBTGMediaWidget* mediaWidget = new UBTGMediaWidget(element->attributes.value("relativePath"),newWidgetItem);
            newWidgetItem->setExpanded(false);
            mpTreeWidget->setItemWidget(mediaItem,0,mediaWidget);
        }
        else if(element->type == "link"){
            createMediaButtonItem();
            QTreeWidgetItem* newWidgetItem = new QTreeWidgetItem(mpMediaSwitchItem);
            newWidgetItem->setText(0,element->attributes.value("title"));
            newWidgetItem->setData(0,tUBTGTreeWidgetItemRole_HasAnAction,tUBTGActionAssociateOnClickItem_URL);
            newWidgetItem->setData(0,tUBTGTreeWidgetItemRole_HasAnUrl,QVariant(element->attributes.value("url")));
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
 *                    class    UBTeacherGuideWidget                        *
 ***************************************************************************/
UBTeacherGuideWidget::UBTeacherGuideWidget(QWidget *parent, const char *name): QStackedWidget(parent)
  , mpEditionWidget(NULL)
  , mpPresentationWidget(NULL)
{
    setObjectName(name);
    mpEditionWidget = new UBTeacherGuideEditionWidget(this);
    addWidget(mpEditionWidget);
    mpPresentationWidget = new UBTeacherGuidePresentationWidget(this);
    addWidget(mpPresentationWidget);

    setCurrentWidget(mpEditionWidget);
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
    if(currentWidget()!=mpPresentationWidget){
        currentData = mpEditionWidget->getData();
        mpPresentationWidget->showData(currentData);
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
