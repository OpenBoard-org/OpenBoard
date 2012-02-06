#include <QDebug>
#include <QScrollBar>

#include "globals/UBGlobals.h"
#include "UBWidgetList.h"

UBWidgetList::UBWidgetList(QWidget* parent, eWidgetListOrientation orientation, const char* name):QScrollArea(parent)
  , mpLayout(NULL)
  , mpContainer(NULL)
  , mMargin(5)
  , mListElementsSpacing(10)
  , mpEmptyLabel(NULL)
  , mCanRemove(true)
  , mpCurrentWidget(NULL)
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
   }
   else{
       setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
       setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
       mpLayout = new QHBoxLayout(mpContainer);
   }
   mpLayout->setContentsMargins(margin(), margin(), margin(), margin());
   mpContainer->setLayout(mpLayout);
   setWidget(mpContainer);
}

UBWidgetList::~UBWidgetList()
{
    DELETEPTR(mpEmptyLabel);
    DELETEPTR(mpLayout);
    DELETEPTR(mpContainer);
}

void UBWidgetList::addWidget(QWidget *widget)
{
    if(NULL != mpLayout && NULL != widget){
        widget->setParent(mpContainer);
        mpEmptyLabel->setVisible(false);
        mWidgetInfo[widget] = widget->size();
        updateView(size());
        mpLayout->addWidget(widget);

        // This call is used only to refresh the size of the widgets
        updateSizes();
    }
}

void UBWidgetList::removeWidget(QWidget *widget)
{
    if(NULL != mpLayout && NULL != widget){
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
    // to remove the first spacing that shouldn't be there.
    int result = -mListElementsSpacing;
    foreach(QWidget* eachWidget, mWidgetInfo.keys()){
        qreal scaleFactor = 0;
        int newWidgetWidth =  pSize.width();
        int newWidgetHeight = pSize.height();
        if(eWidgetListOrientation_Vertical == mOrientation){
            scaleFactor = (float)mWidgetInfo[eachWidget].width() / (float)pSize.width();
            newWidgetHeight = mWidgetInfo[eachWidget].height()/scaleFactor;
            result += newWidgetHeight;
            eachWidget->setMinimumHeight(newWidgetHeight- 1);
            eachWidget->setMaximumHeight(newWidgetHeight);
        }
        else{
            scaleFactor = (float)mWidgetInfo[eachWidget].height() / (float)pSize.height();
            newWidgetWidth = mWidgetInfo[eachWidget].width()/scaleFactor;
            result += newWidgetWidth;
            eachWidget->setMinimumWidth(newWidgetWidth - 1);
            eachWidget->setMaximumWidth(newWidgetWidth);
        }
        //Adding a vertical/horizontal space between each element of the list
        result += mListElementsSpacing;
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
    Q_UNUSED(ev);
    mpEmptyLabel->setGeometry((width() - mpEmptyLabel->width()) / 2,
                              (height() - mpEmptyLabel->height()) /2,
                              mpEmptyLabel->width(),
                              mpEmptyLabel->height());
    updateView(size());
    updateSizes();
}

void UBWidgetList::mousePressEvent(QMouseEvent *ev)
{
    if(mCanRemove){

    }
}

void UBWidgetList::updateSizes()
{
    // Resize all the widgets
    foreach(QWidget* eachWidget, mWidgetInfo.keys()){
        if(NULL != eachWidget){
            QSize originalSize = mWidgetInfo[eachWidget];
            int currentWidth = mpContainer->width();
            int currentHeight = mpContainer->height();
            if(eWidgetListOrientation_Vertical == mOrientation){
                if(verticalScrollBar()->isVisible()){
                    currentWidth -= verticalScrollBar()->width();
                    eachWidget->setStyleSheet(QString("margin-right:%0;").arg(verticalScrollBar()->width()));
                }
                float scaleFactor = (float)currentWidth/(float)originalSize.width();
                currentHeight = originalSize.height()*scaleFactor;
            }else{
                if(horizontalScrollBar()->isVisible()){
                    currentHeight -= horizontalScrollBar()->height();
                    eachWidget->setStyleSheet(QString("padding-bottom:%0;").arg(horizontalScrollBar()->height()));
                }
                float scaleFactor = (float)currentHeight/(float)originalSize.height();
                currentWidth = originalSize.width()*scaleFactor;
            }

            eachWidget->resize(currentWidth, currentHeight);
        }
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
    return mWidgetInfo.empty();
}

// TODO :   - add onHover 'delete' button

