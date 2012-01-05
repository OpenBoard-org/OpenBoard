#include <QDebug>
#include "UBWidgetList.h"

UBWidgetList::UBWidgetList(QWidget* parent, eWidgetListOrientation orientation, const char* name):QScrollArea(parent)
  , mpLayout(NULL)
  , mpContainer(NULL)
  , mMargin(5)
  , mpEmptyLabel(NULL)
{
   setObjectName(name);
   mOrientation = orientation;
   mpContainer = new QWidget(this);
   mWidgets.clear();
   mpEmptyLabel = new QLabel(this);
   mpEmptyLabel->setObjectName("emptyString");

   if(eWidgetListOrientation_Vertical == orientation){
       setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
       setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
       mpLayout = new QVBoxLayout(mpContainer);
       mpContainer->resize(width(), mpContainer->height());
   }else{
       setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
       setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
       mpLayout = new QHBoxLayout(mpContainer);
       mpContainer->resize(mpContainer->width(), height());
   }
   mpLayout->setContentsMargins(margin(), margin(), margin(), margin());
   mpContainer->setLayout(mpLayout);
   setWidget(mpContainer);
}

UBWidgetList::~UBWidgetList()
{
    if(NULL != mpEmptyLabel){
        delete mpEmptyLabel;
        mpEmptyLabel = NULL;
    }
    if(NULL != mpLayout){
        delete mpLayout;
        mpLayout = NULL;
    }
    if(NULL != mpContainer){
        delete mpContainer;
        mpContainer = NULL;
    }
}

void UBWidgetList::addWidget(QWidget *widget)
{
    if(NULL != mpLayout){
        mpEmptyLabel->setVisible(false);
        mWidgets << widget;
        updateSize(true, widget);
        mpLayout->addWidget(widget);
    }
}

void UBWidgetList::removeWidget(QWidget *widget)
{
    if(NULL != mpLayout){
        mpLayout->removeWidget(widget);
        mWidgets.remove(mWidgets.indexOf(widget));
        updateSize(false, widget);
        widget->setVisible(false);
        if(0 == mpLayout->count()){
            mpEmptyLabel->setVisible(true);
        }
    }
}

void UBWidgetList::updateSize(bool widgetAdded, QWidget *widget)
{
    float scaleFactor;
    int newWidgetWidth;
    int newWidgetHeight;

    if(eWidgetListOrientation_Vertical == mOrientation){
        scaleFactor = (float)widget->width() / (float)mpContainer->width();
    }else{
        scaleFactor = (float)widget->height() / (float)mpContainer->height();
    }

    newWidgetWidth = widget->width()/scaleFactor;
    newWidgetHeight = widget->height()/scaleFactor;

    widget->resize(newWidgetWidth, newWidgetHeight);


    // Now we have to update the container
    if(eWidgetListOrientation_Vertical == mOrientation){
        if(widgetAdded){
            mpContainer->resize(mpContainer->width(), mpContainer->height() + newWidgetHeight);
        }
        else{
            mpContainer->resize(mpContainer->width(), mpContainer->height() - newWidgetHeight);
        }
    }
    else{
        if(widgetAdded){
            mpContainer->resize(mpContainer->width() + newWidgetWidth, mpContainer->height());
        }
        else{
            mpContainer->resize(mpContainer->width() - newWidgetWidth, mpContainer->height());
        }
    }
}

void UBWidgetList::resizeEvent(QResizeEvent *ev)
{
    mpEmptyLabel->setGeometry((width() - mpEmptyLabel->width()) / 2,
                              (height() - mpEmptyLabel->height()) /2,
                              mpEmptyLabel->width(),
                              mpEmptyLabel->height());
    if(ev->oldSize().width() >= 0 && ev->oldSize().height() >= 0){
        float scale;
        if(eWidgetListOrientation_Vertical == mOrientation){
            scale = (float)ev->size().width() / (float)ev->oldSize().width();
            if(scale != 0 && scale < 10){
                updateAllWidgetsize(scale);
                mpContainer->resize(width() - 2, mpContainer->height()*scale);
            }
        }
        else{
            scale = (float)ev->size().height() / (float)ev->oldSize().height();
            if(scale != 0 && scale < 10){
                updateAllWidgetsize(scale);
                mpContainer->resize(mpContainer->width()*scale, height() - 2);
            }
        }
    }
}

void UBWidgetList::updateAllWidgetsize(float scale)
{
    for(int i=0; i<mWidgets.size(); i++){
        mWidgets.at(i)->resize(mWidgets.at(i)->width()*scale, mWidgets.at(i)->height()*scale);
    }
}

void UBWidgetList::setMargin(int margin)
{
    mMargin = margin;
}

int UBWidgetList::margin()
{
    return mMargin;
}

void UBWidgetList::setEmptyText(const QString &text)
{
    if(NULL != mpEmptyLabel){
        mpEmptyLabel->setText(text);
    }
}

bool UBWidgetList::empty()
{
    return mWidgets.empty();
}

// TODO :   - add onHover 'delete' button

