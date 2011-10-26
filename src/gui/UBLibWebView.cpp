#include "UBLibWebView.h"

UBLibWebView::UBLibWebView(QWidget* parent, const char* name):QWidget(parent)
    , mpView(NULL)
    , mpLayout(NULL)
{
    setObjectName(name);

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    mpView = new QWebView(this);
    mpLayout->addWidget(mpView);
}

UBLibWebView::~UBLibWebView()
{
    if(NULL != mpView)
    {
        delete mpView;
        mpView = NULL;
    }
    if(NULL != mpLayout)
    {
        delete mpLayout;
        mpLayout = NULL;
    }
}

void UBLibWebView::setElement(UBLibElement *elem)
{
    if(NULL != elem)
    {
        mpView->load(elem->path());
    }
}
