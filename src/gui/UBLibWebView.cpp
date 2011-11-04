#include <QDomDocument>

#include "core/UBApplication.h"

#include "UBLibWebView.h"

UBLibWebView::UBLibWebView(QWidget* parent, const char* name):QWidget(parent)
    , mpView(NULL)
    , mpWebSettings(NULL)
    , mpLayout(NULL)
{
    setObjectName(name);

    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(UBApplication::globalStyleSheet());

    mpLayout = new QVBoxLayout();
    setLayout(mpLayout);

    mpView = new QWebView(this);
    mpView->setObjectName("SearchEngineView");

    mpWebSettings = QWebSettings::globalSettings();
    mpWebSettings->setAttribute(QWebSettings::JavaEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::PluginsEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::JavaEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    mpWebSettings->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    mpWebSettings->setAttribute(QWebSettings::DnsPrefetchEnabled, true);

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
