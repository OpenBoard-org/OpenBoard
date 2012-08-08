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

#include "UBAbstractWidget.h"

#include <QtNetwork>
#include <QtXml>

#include "frameworks/UBFileSystemUtils.h"

#include "core/UBApplicationController.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "network/UBNetworkAccessManager.h"

#include "web/UBWebPage.h"
#include "web/UBWebKitUtils.h"
#include "web/UBWebController.h"

#include "core/memcheck.h"

QStringList UBAbstractWidget::sInlineJavaScripts;
bool UBAbstractWidget::sInlineJavaScriptLoaded = false;

UBAbstractWidget::UBAbstractWidget(const QUrl& pWidgetUrl, QWidget *parent)
    : UBRoutedMouseEventWebView(parent)
    , mWidgetUrl(pWidgetUrl)
    , mIsResizable(false)
    , mInitialLoadDone(false)
    , mLoadIsErronous(false)
    , mIsFreezable(true)
    , mCanBeContent(0)
    , mCanBeTool(0)
    , mIsFrozen(false)
    , mIsTakingSnapshot(false)
{
    setAcceptDrops(true);
    setPage(new UBWebPage(this));
    QWebView::settings()->setAttribute(QWebSettings::JavaEnabled, true);
    QWebView::settings()->setAttribute(QWebSettings::PluginsEnabled, true);
    QWebView::settings()->setAttribute(QWebSettings::LocalStorageDatabaseEnabled, true);
    QWebView::settings()->setAttribute(QWebSettings::OfflineWebApplicationCacheEnabled, true);
    QWebView::settings()->setAttribute(QWebSettings::OfflineStorageDatabaseEnabled, true);
    QWebView::settings()->setAttribute(QWebSettings::JavascriptCanAccessClipboard, true);
    QWebView::settings()->setAttribute(QWebSettings::DnsPrefetchEnabled, true);

    QWebView::page()->setNetworkAccessManager(UBNetworkAccessManager::defaultAccessManager());

    setAutoFillBackground(false);

    QPalette pagePalette = QWebView::page()->palette();
    pagePalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    pagePalette.setBrush(QPalette::Window, QBrush(Qt::transparent));
    QWebView::page()->setPalette(pagePalette);

    QPalette viewPalette = palette();
    pagePalette.setBrush(QPalette::Base, QBrush(Qt::transparent));
    viewPalette.setBrush(QPalette::Window, QBrush(Qt::transparent));
    setPalette(viewPalette);

    connect(QWebView::page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(javaScriptWindowObjectCleared()));
    connect(QWebView::page(), SIGNAL(geometryChangeRequested(const QRect&)), this, SIGNAL(geometryChangeRequested(const QRect&)));
    connect(this, SIGNAL(loadFinished(bool)), this, SLOT(mainFrameLoadFinished (bool)));

    setMouseTracking(true);
}

bool UBAbstractWidget::canBeContent()
{
    // if we under MAC OS
    #if defined(Q_OS_MAC)
        return mCanBeContent & UBAbstractWidget::type_MAC;
    #endif

    // if we under UNIX OS
    #if defined(Q_OS_UNIX)
        return mCanBeContent & UBAbstractWidget::type_UNIX;
    #endif

    // if we under WINDOWS OS
    #if defined(Q_OS_WIN)
        return mCanBeContent & UBAbstractWidget::type_WIN;
    #endif
}

bool UBAbstractWidget::canBeTool()
{
    // if we under MAC OS
    #if defined(Q_OS_MAC)
        return mCanBeTool & UBAbstractWidget::type_MAC;
    #endif

        // if we under UNIX OS
    #if defined(Q_OS_UNIX)
        return mCanBeTool & UBAbstractWidget::type_UNIX;
    #endif

        // if we under WINDOWS OS
    #if defined(Q_OS_WIN)
        return mCanBeTool & UBAbstractWidget::type_WIN;
    #endif
}

UBAbstractWidget::~UBAbstractWidget()
{
    // NOOP
}

void UBAbstractWidget::loadMainHtml()
{
    mInitialLoadDone = false;
    QWebView::load(mMainHtmlUrl);
}

bool UBAbstractWidget::event(QEvent *event)
{
    if (event->type() == QEvent::ContextMenu)
    {
        event->accept();
        return true;
    } else {
        return QWebView::event(event);
    }
}


void UBAbstractWidget::mainFrameLoadFinished (bool ok)
{
    mInitialLoadDone = true;
    mLoadIsErronous = !ok;

    update();
}


bool UBAbstractWidget::hasEmbededObjects()
{
    if (QWebView::page()->mainFrame())
    {
        QList<UBWebKitUtils::HtmlObject> htmlObjects = UBWebKitUtils::objectsInFrame(QWebView::page()->mainFrame());
        return htmlObjects.length() > 0;
    }

    return false;
}


bool UBAbstractWidget::hasEmbededFlash()
{
    if (hasEmbededObjects())
    {
        return QWebView::page()->mainFrame()->toHtml().contains("application/x-shockwave-flash");
    }
    else
    {
        return false;
    }
}


void UBAbstractWidget::resize(qreal width, qreal height)
{
    qreal w = qMax((qreal)mNominalSize.width(), width);
    qreal h = qMax((qreal)mNominalSize.height(), height);
    QWebView::page()->setViewportSize(QSize(w, h));
    QWebView::setFixedSize(QSize(w, h));
}


QString UBAbstractWidget::iconFilePath(const QUrl& pUrl)
{
    // TODO UB 4.x read config.xml widget.icon param first

    QStringList files;

    files << "icon.svg";  // W3C widget default 1
    files << "icon.ico";  // W3C widget default 2
    files << "icon.png";  // W3C widget default 3
    files << "icon.gif";  // W3C widget default 4

    files << "Icon.png";  // Apple widget default

    QString file = UBFileSystemUtils::getFirstExistingFileFromList(pUrl.toLocalFile(), files);

    // default
    if (file.length() == 0)
    {
        file = QString(":/images/defaultWidgetIcon.png");
    }

    return file;
}



QString UBAbstractWidget::widgetName(const QUrl& widgetPath)
{
    QString name;
    QString version;

    QFile w3CConfigFile(widgetPath.toLocalFile() + "/config.xml");
    QFile appleConfigFile(widgetPath.toLocalFile() + "/Info.plist");

    if (w3CConfigFile.exists() && w3CConfigFile.open(QFile::ReadOnly))
    {
        QDomDocument doc;
        doc.setContent(w3CConfigFile.readAll());
        QDomElement root = doc.firstChildElement("widget");
        if (!root.isNull())
        {
            QDomElement nameElement = root.firstChildElement("name");
            if (!nameElement.isNull())
                name = nameElement.text();

            version = root.attribute("version", "");
        }

        w3CConfigFile.close();
    }
    else if (appleConfigFile.exists() && appleConfigFile.open(QFile::ReadOnly))
    {
        QDomDocument doc;
        doc.setContent(appleConfigFile.readAll());
        QDomElement root = doc.firstChildElement("plist");
        if (!root.isNull())
        {
            QDomElement dictElement = root.firstChildElement("dict");
            if (!dictElement.isNull())
            {
                QDomNodeList childNodes  = dictElement.childNodes();

                // looking for something like
                //  ..
                //  <key>CFBundleDisplayName</key>
                //  <string>brain scans</string>
                //  ..

                for(int i = 0; i < childNodes.count() - 1; i++)
                {
                    if (childNodes.at(i).isElement())
                    {
                        QDomElement elKey = childNodes.at(i).toElement();
                        if (elKey.text() == "CFBundleDisplayName")
                        {
                            if (childNodes.at(i + 1).isElement())
                            {
                               QDomElement elValue = childNodes.at(i + 1).toElement();
                               name = elValue.text();
                            }
                        }
                        else if (elKey.text() == "CFBundleShortVersionString")
                        {
                            if (childNodes.at(i + 1).isElement())
                            {
                               QDomElement elValue = childNodes.at(i + 1).toElement();
                               version = elValue.text();
                            }
                        }
                    }
                }
            }
        }

        appleConfigFile.close();
    }

    QString result;

    if (name.length() > 0)
    {
        result = name;
        if (version.length() > 0)
        {
            result += " ";
            result += version;
        }
    }

    return result;
}


int UBAbstractWidget::widgetType(const QUrl& pUrl)
{
    QString mime = UBFileSystemUtils::mimeTypeFromFileName(pUrl.toString());

    if (mime == "application/vnd.apple-widget")
    {
        return UBWidgetType::Apple;
    }
    else if (mime == "application/widget")
    {
        return UBWidgetType::W3C;
    }
    else
    {
        return UBWidgetType::Other;
    }
}


void UBAbstractWidget::mousePressEvent(QMouseEvent *event)
{
    if(mIsFrozen)
    {
        event->accept();
        return;
    }
    UBRoutedMouseEventWebView::mousePressEvent(event);
    mMouseIsPressed = true;
}


void UBAbstractWidget::mouseMoveEvent(QMouseEvent *event)
{

    if(mIsFrozen)
    {
        event->accept();
        return;
    }

    // TODO UB 4.x fix web kit mouse move routing

    if (mFirstReleaseAfterMove)
    {
        mFirstReleaseAfterMove = false;
    }
    else
    {
        UBRoutedMouseEventWebView::mouseMoveEvent(event);
    }
}


void UBAbstractWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if(mIsFrozen)
    {
        event->accept();
        return;
    }

    UBRoutedMouseEventWebView::mouseReleaseEvent(event);
    mMouseIsPressed = false;
    mFirstReleaseAfterMove = true;
}

QWebView * UBAbstractWidget::createWindow(QWebPage::WebWindowType type)
{
    if (type == QWebPage::WebBrowserWindow)
    {
        UBApplication::applicationController->showInternet();
        return UBApplication::webController->createNewTab();
    }
    else
    {
        return this;
    }
}


void UBAbstractWidget::injectInlineJavaScript()
{
    if (!sInlineJavaScriptLoaded)
    {
        sInlineJavaScripts = UBApplication::applicationController->widgetInlineJavaScripts();
        sInlineJavaScriptLoaded = true;
    }

    foreach(QString script, sInlineJavaScripts)
    {
        QWebView::page()->mainFrame()->evaluateJavaScript(script);
    }
}


void UBAbstractWidget::javaScriptWindowObjectCleared()
{
    injectInlineJavaScript();
}


void UBAbstractWidget::paintEvent(QPaintEvent * event)
{
    if (mIsFrozen)
    {
        QPainter p(this);
        p.drawPixmap(0, 0, mSnapshot);
    }
    else
    {
        QWebView::paintEvent(event);
    }
    if (!mInitialLoadDone || mLoadIsErronous)
    {
         QPainter p(this);
         QString message = tr("Loading ...");

         // this is the right way of doing but we receive two callback and the one return always that the
         // load as failed... to check
         if (mInitialLoadDone && mLoadIsErronous)
             message = tr("Cannot load content");
         else
             message = tr("Loading ...");

         p.setFont(QFont("Arial", 12));

         QFontMetrics fm = p.fontMetrics();
         QRect txtBoundingRect = fm.boundingRect(message);

         txtBoundingRect.moveCenter(rect().center());
         txtBoundingRect.adjust(-10, -5, 10, 5);

         p.setPen(Qt::NoPen);
         p.setBrush(UBSettings::paletteColor);
         p.drawRoundedRect(txtBoundingRect, 3, 3);

         p.setPen(Qt::white);
         p.drawText(rect(), Qt::AlignCenter, message);
    }
}
void UBAbstractWidget::dropEvent(QDropEvent *event)
{
    QWebView::dropEvent(event);
}

QPixmap UBAbstractWidget::takeSnapshot()
{
    mIsTakingSnapshot = true;

    QPixmap pix(size());
    pix.fill(Qt::transparent);

    render(&pix);

    mIsTakingSnapshot = false;

    return pix;
}


void UBAbstractWidget::setSnapshot(const QPixmap& pix)
{
    mSnapshot = pix;
}


void UBAbstractWidget::freeze()
{
    QPixmap pix = takeSnapshot();
    mIsFrozen = true;
    setSnapshot(pix);
    update();
}


void UBAbstractWidget::unFreeze()
{
    mIsFrozen = false;
    update();
}

