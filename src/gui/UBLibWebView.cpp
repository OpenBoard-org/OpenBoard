#include <QDomDocument>

#include "core/UBApplication.h"

#include "board/UBBoardController.h"

#include "globals/UBGlobals.h"

#include "UBLibWebView.h"

#include "core/memcheck.h"

UBLibWebView::UBLibWebView(QWidget* parent, const char* name):QWidget(parent)
    , mpView(NULL)
    , mpWebSettings(NULL)
    , mpLayout(NULL)
    , mpSankoreAPI(NULL)
{
    setObjectName(name);

    SET_STYLE_SHEET();

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    mpView = new QWebView(this);
    mpView->setObjectName("SearchEngineView");
    mpSankoreAPI = new UBWidgetUniboardAPI(UBApplication::boardController->activeScene());
    connect(mpView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));

    mpLayout->addWidget(mpView);
}

UBLibWebView::~UBLibWebView()
{
    if(NULL != mpSankoreAPI){
        delete mpSankoreAPI;
        mpSankoreAPI = NULL;
    }
    if(NULL != mpView){
        delete mpView;
        mpView = NULL;
    }
    if(NULL != mpLayout){
        delete mpLayout;
        mpLayout = NULL;
    }
}

void UBLibWebView::setElement(UBLibElement *elem)
{
    if(NULL != elem)
    {
        QString qsWidgetName;
        QString path = elem->path().toLocalFile();

        QString qsConfigPath = QString("%0/config.xml").arg(path);

        if(QFile::exists(qsConfigPath))
        {
            QFile f(qsConfigPath);
            if(f.open(QIODevice::ReadOnly))
            {
                QDomDocument domDoc;
                domDoc.setContent(QString(f.readAll()));
                QDomElement root = domDoc.documentElement();

                QDomNode node = root.firstChild();
                while(!node.isNull())
                {
                    if(node.toElement().tagName() == "content")
                    {
                        QDomAttr srcAttr = node.toElement().attributeNode("src");
                        qsWidgetName = srcAttr.value();
                        break;
                    }
                    node = node.nextSibling();
                }
                f.close();
            }
        }
        mpView->load(QUrl::fromLocalFile(QString("%0/%1").arg(path).arg(qsWidgetName)));
    }
}

void UBLibWebView::javaScriptWindowObjectCleared()
{
    mpView->page()->mainFrame()->addToJavaScriptWindowObject("sankore", mpSankoreAPI);
}
