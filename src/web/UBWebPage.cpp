/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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

#include "UBWebPage.h"

#include <QtCore>
#include <QWebPluginFactory>

#include "pdf/UBWebPluginPDFWidget.h"

#include "core/memcheck.h"

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


