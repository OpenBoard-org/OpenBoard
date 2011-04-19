
#include "UBWebPage.h"

#include <QtCore>
#include <QWebPluginFactory>

#include "pdf/UBWebPluginPDFWidget.h"

class UBWebPluginFactory : public QWebPluginFactory
{
    public:

    UBWebPluginFactory(QObject *parent = 0) : QWebPluginFactory(parent)
    {
        // NOOP
    }

    QList<Plugin> plugins() const
    {
        QStringList pdfExtensions;
        pdfExtensions << "pdf";
        MimeType pdfMimeType = {"application/x-ub-pdf", "Portable Document Format", pdfExtensions};
        QList<MimeType> mimeTypes;
        mimeTypes << pdfMimeType;
        Plugin pdfPlugin = {"PDF Plugin", "Display PDF files", mimeTypes};
        QList<Plugin> plugins;
        plugins << pdfPlugin;
        return plugins;
    }

    QObject* create(const QString &mimeType, const QUrl &url, const QStringList &argumentNames, const QStringList &argumentValues) const
    {
        Q_UNUSED(url);
                Q_UNUSED(argumentNames);
                Q_UNUSED(argumentValues);

        if (mimeType == "application/x-ub-pdf")
        {
            UBWebPluginPDFWidget *pdfWidget = new UBWebPluginPDFWidget(url);
            pdfWidget->setObjectName("PDFWebPluginWidget");
            return pdfWidget;
        }
        return 0;
    }
};



UBWebPage::UBWebPage(QObject *parent)
    : QWebPage(parent)
    , mPluginFactory(0)
{
    mCachedUserAgentString = QWebPage::userAgentForUrl(QUrl());
    //mPluginFactory = new UBWebPluginFactory();
    //setPluginFactory(mPluginFactory);

    //qDebug() << "caching user agent string" << mCachedUserAgentString;
}

UBWebPage::~UBWebPage()
{
//    delete mPluginFactory;
}


void UBWebPage::javaScriptConsoleMessage(const QString &message, int lineNumber, const QString &sourceID)
{
    qDebug("JavaScript> %s (%s:%d)", qPrintable(message), qPrintable(sourceID), lineNumber);
}


QString UBWebPage::userAgentForUrl(const QUrl& url) const
{
    Q_UNUSED(url);
    return mCachedUserAgentString;
}


