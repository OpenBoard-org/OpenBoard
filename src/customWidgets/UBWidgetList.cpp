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
   mpEmptyLabel = new QLabel(this);
   mpEmptyLabel->setObjectName("emptyString");
   mpEmptyLabel->setWordWrap(true);
   mpEmptyLabel->setAlignment(Qt::AlignCenter);

   if(eWidgetListOrientation_Vertical == orientation){
       setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
       setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
       mpLayout = new QVBoxLayout(mpContainer);
       mpContainer->resize(mpContainer->width(), mpContainer->height());
   }
   else{
       setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
       setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
       mpLayout = new QHBoxLayout(mpContainer);
       mpContainer->resize(mpContainer->width(), mpContainer->height());
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
        mWidgetInfo[widget] = widget->size();
        qDebug() << __FUNCTION__ << "widget->size () " << widget->size();
        updateView(size());
        mpLayout->addWidget(widget);
    }
}

void UBWidgetList::removeWidget(QWidget *widget)
{
    if(NULL != mpLayout){
        mpLayout->removeWidget(widget);
        mWidgetInfo.remove(widget);
        widget->setVisible(false);
        updateView(size());
        if(0 == mpLayout->count()){
            mpEmptyLabel->setVisible(true);
        }
    }
}


int UBWidgetList::scaleWidgets(QSize pSize)
{
    int result = 0;
    int count = 0;
    foreach(QWidget* eachWidget, mWidgetInfo.keys()){
        qreal scaleFactor = 0;
        int newWidgetWidth =  pSize.width();
        int newWidgetHeight = pSize.height();
        if(eWidgetListOrientation_Vertical == mOrientation){
            scaleFactor = (float)mWidgetInfo[eachWidget].width() / (float)pSize.width();
            newWidgetHeight = mWidgetInfo[eachWidget].height()/scaleFactor;
            result += newWidgetHeight;
        }
        else{
            scaleFactor =  (float)mWidgetInfo[eachWidget].height() / (float)pSize.height();
            newWidgetWidth = mWidgetInfo[eachWidget].width()/scaleFactor;
            result += newWidgetWidth;
        }
#ifndef Q_WS_WIN
		qDebug() << __PRETTY_FUNCTION__ << "widget " << &eachWidget;
        qDebug() << __PRETTY_FUNCTION__ << "count " << count++;
        qDebug() << __PRETTY_FUNCTION__ << "widget orignal size " << mWidgetInfo[eachWidget];
        qDebug() << __PRETTY_FUNCTION__ << "containes size  " << pSize;
        qDebug() << __PRETTY_FUNCTION__ << "scale factor " << scaleFactor;
        qDebug() << __PRETTY_FUNCTION__ << "new height " << result;
#endif 
    }
    return result;
}

void UBWidgetList::scaleContainer(QSize pSize, int updateValue)
{
    if(eWidgetListOrientation_Vertical == mOrientation)
        mpContainer->resize(pSize.width(), updateValue);
    else
        mpContainer->resize(updateValue, pSize.height());
}


void UBWidgetList::updateView(QSize pSize)
{
    // Widgets on list are resized automatically to fit the mpcontainer.
    // so if you want to keep the aspect ratio you have to calculate
    // the sum of the new widget height and give it to the mpContainer.
    // The container resize will trig the widgets resize and the good
    // height permits to respect the aspect ratio.
    int updatedValue = scaleWidgets(pSize);
    scaleContainer(pSize,updatedValue);
}



void UBWidgetList::resizeEvent(QResizeEvent *ev)
{
    mpEmptyLabel->setGeometry((width() - mpEmptyLabel->width()) / 2,
                              (height() - mpEmptyLabel->height()) /2,
                              mpEmptyLabel->width(),
                              mpEmptyLabel->height());
    updateView(size());
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
    return mWidgetInfo.empty();
}

// TODO :   - add onHover 'delete' button

