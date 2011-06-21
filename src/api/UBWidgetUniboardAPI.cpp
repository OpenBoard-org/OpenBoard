#include "UBWidgetUniboardAPI.h"

#include <QWebView>

#include "core/UB.h"
#include "core/UBApplication.h"
#include "core/UBSettings.h"

#include "document/UBDocumentProxy.h"

#include "board/UBBoardController.h"
#include "board/UBDrawingController.h"

#include "domain/UBGraphicsScene.h"
#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBAbstractWidget.h"

#include "adaptors/UBThumbnailAdaptor.h"

#include "UBWidgetMessageAPI.h"


UBWidgetUniboardAPI::UBWidgetUniboardAPI(UBGraphicsScene *pScene, UBGraphicsWidgetItem *widget)
    : QObject(pScene)
    , mScene(pScene)
    , mGraphicsWidget(widget)
    , mIsVisible(false)
    , mMessagesAPI(0)
    , mDatastoreAPI(0)
{
    UBGraphicsW3CWidgetItem* w3CGraphicsWidget = dynamic_cast<UBGraphicsW3CWidgetItem*>(widget);

    if (w3CGraphicsWidget)
    {
        mMessagesAPI = new UBWidgetMessageAPI(w3CGraphicsWidget->w3cWidget());
        mDatastoreAPI = new UBDatastoreAPI(w3CGraphicsWidget);
    }
}


UBWidgetUniboardAPI::~UBWidgetUniboardAPI()
{
    // NOOP
}

QObject* UBWidgetUniboardAPI::messages()
{
    return mMessagesAPI;
}


QObject* UBWidgetUniboardAPI::datastore()
{
    return mDatastoreAPI;
}


void UBWidgetUniboardAPI::setTool(const QString& toolString)
{
    if (UBApplication::boardController->activeScene() != mScene)
        return;

    const QString lower = toolString.toLower();

    if (lower == "pen")
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Pen);
    }
    else if (lower == "marker")
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Marker);
    }
    else if (lower == "arrow")
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Selector);
    }
    else if (lower == "line")
    {
        UBDrawingController::drawingController()->setStylusTool(UBStylusTool::Line);
    }
}


void UBWidgetUniboardAPI::setPenColor(const QString& penColor)
{
    if (UBApplication::boardController->activeScene() != mScene)
        return;

    UBSettings* settings = UBSettings::settings();

    bool conversionState = false;

    int index = penColor.toInt(&conversionState) - 1;

    if (conversionState && index > 0 && index <= 4)
    {
        UBApplication::boardController->setPenColorOnDarkBackground(settings->penColors(true).at(index - 1));
        UBApplication::boardController->setPenColorOnLightBackground(settings->penColors(false).at(index - 1));
    }
    else
    {
        QColor svgColor;
        svgColor.setNamedColor(penColor);
        if (svgColor.isValid())
        {
            UBApplication::boardController->setPenColorOnDarkBackground(svgColor);
            UBApplication::boardController->setPenColorOnLightBackground(svgColor);
        }
    }
}


void UBWidgetUniboardAPI::setMarkerColor(const QString& penColor)
{
    if (UBApplication::boardController->activeScene() != mScene)
        return;

    UBSettings* settings = UBSettings::settings();

    bool conversionState = false;

    int index = penColor.toInt(&conversionState);

    if (conversionState && index > 0 && index <= 4)
    {
        UBApplication::boardController->setMarkerColorOnDarkBackground(settings->markerColors(true).at(index - 1));
        UBApplication::boardController->setMarkerColorOnLightBackground(settings->markerColors(false).at(index - 1));
    }
    else
    {
        QColor svgColor;
        svgColor.setNamedColor(penColor);
        if (svgColor.isValid())
        {
            UBApplication::boardController->setMarkerColorOnDarkBackground(svgColor);
            UBApplication::boardController->setMarkerColorOnLightBackground(svgColor);
        }
    }
}


void UBWidgetUniboardAPI::addObject(QString pUrl, int width, int height, int x, int y, bool background)
{
    // not great, but make it easily scriptable --
    //
    // download url should be moved to the scene from the controller
    //

    if (UBApplication::boardController->activeScene() != mScene)
        return;

    UBApplication::boardController->downloadURL(QUrl(pUrl), QPointF(x, y), QSize(width, height), background);

}


void UBWidgetUniboardAPI::setBackground(bool pIsDark, bool pIsCrossed)
{
    if (mScene)
    mScene->setBackground(pIsDark, pIsCrossed);
}


void UBWidgetUniboardAPI::moveTo(const qreal x, const qreal y)
{
    if (qIsNaN(x) || qIsNaN(y)
        || qIsInf(x) || qIsInf(y))
        return;

    if (mScene)
    mScene->moveTo(QPointF(x, y));
}


void UBWidgetUniboardAPI::drawLineTo(const qreal x, const qreal y, const qreal pWidth)
{
    if (qIsNaN(x) || qIsNaN(y) || qIsNaN(pWidth)
        || qIsInf(x) || qIsInf(y) || qIsInf(pWidth))
        return;

    if (mScene)
    mScene->drawLineTo(QPointF(x, y), pWidth, 
		UBDrawingController::drawingController()->stylusTool() == UBStylusTool::Line);
}


void UBWidgetUniboardAPI::eraseLineTo(const qreal x, const qreal y, const qreal pWidth)
{
    if (qIsNaN(x) || qIsNaN(y) || qIsNaN(pWidth)
       || qIsInf(x) || qIsInf(y) || qIsInf(pWidth))
       return;

    if (mScene)
    mScene->eraseLineTo(QPointF(x, y), pWidth);
}


void UBWidgetUniboardAPI::clear()
{
    if (mScene)
            mScene->clearItemsAndAnnotations();
}


void UBWidgetUniboardAPI::zoom(const qreal factor, const qreal x, const qreal y)
{
   if (qIsNaN(factor) || qIsNaN(x) || qIsNaN(y)
       || qIsInf(factor) || qIsInf(x) || qIsInf(y))
       return;


    if (UBApplication::boardController->activeScene() != mScene)
        return;

    UBApplication::boardController->zoom(factor, QPointF(x, y));
}


void UBWidgetUniboardAPI::centerOn(const qreal x, const qreal y)
{
   if (qIsNaN(x) || qIsNaN(y)
       || qIsInf(x) || qIsInf(y))
       return;

    if (UBApplication::boardController->activeScene() != mScene)
        return;

    UBApplication::boardController->centerOn(QPointF(x, y));
}


void UBWidgetUniboardAPI::move(const qreal x, const qreal y)
{
    if (qIsNaN(x) || qIsNaN(y)
        || qIsInf(x) || qIsInf(y))
        return;

    if (UBApplication::boardController->activeScene() != mScene)
        return;

    UBApplication::boardController->handScroll(x, y);
}


void UBWidgetUniboardAPI::addText(const QString& text, const qreal x, const qreal y, const int size, const QString& font
        , bool bold, bool italic)
{
    if (qIsNaN(x) || qIsNaN(y)
        || qIsInf(x) || qIsInf(y))
        return;

    if (UBApplication::boardController->activeScene() != mScene)
        return;

    if (mScene)
        mScene->addTextWithFont(text, QPointF(x, y), size, font, bold, italic);

}


int UBWidgetUniboardAPI::pageCount()
{
    if (mScene && mScene->document())
        return mScene->document()->pageCount();
    else
        return -1;
}


int UBWidgetUniboardAPI::currentPageNumber()
{
    // TODO UB 4.x widget find a better way to get the current page number

    if (UBApplication::boardController->activeScene() != mScene)
        return -1;

    return UBApplication::boardController->activeSceneIndex() + 1;
}


void UBWidgetUniboardAPI::showMessage(const QString& message)
{
    UBApplication::boardController->showMessage(message, false);
}


QString UBWidgetUniboardAPI::pageThumbnail(const int pageNumber)
{
    if (UBApplication::boardController->activeScene() != mScene)
        return "";

    UBDocumentProxy *doc = UBApplication::boardController->activeDocument();

    if (!doc)
        return "";

    if (pageNumber > doc->pageCount())
        return "";

    QUrl url = UBThumbnailAdaptor::thumbnailUrl(doc, pageNumber - 1);

    return url.toString();

}


void UBWidgetUniboardAPI::resize(qreal width, qreal height)
{
    if (qIsNaN(width) || qIsNaN(height)
        || qIsInf(width) || qIsInf(height))
        return;

    if (mGraphicsWidget)
    {
        mGraphicsWidget->resize(width, height);
    }
}


void UBWidgetUniboardAPI::setPreference(const QString& key, QString value)
{
    if (mGraphicsWidget)
    {
            mGraphicsWidget->setPreference(key, value);
    }
}


QString UBWidgetUniboardAPI::preference(const QString& key , const QString& pDefault)
{
    if (mGraphicsWidget && mGraphicsWidget->preferences().contains(key))
    {
        return mGraphicsWidget->preference(key);
    }
    else
    {
        return pDefault;
    }
}


QStringList UBWidgetUniboardAPI::preferenceKeys()
{
    QStringList keys;

    if (mGraphicsWidget)
        keys = mGraphicsWidget->preferences().keys();

    return keys;
}


QString UBWidgetUniboardAPI::uuid()
{
    if (mGraphicsWidget)
        return UBStringUtils::toCanonicalUuid(mGraphicsWidget->uuid());
    else
        return "";
}


QString UBWidgetUniboardAPI::locale()
{
    return QLocale().name();
}

QString UBWidgetUniboardAPI::lang()
{
    QString lang = QLocale().name();

    if (lang.length() > 2)
        lang[2] = QLatin1Char('-');

    return lang;
}



UBDocumentDatastoreAPI::UBDocumentDatastoreAPI(UBGraphicsW3CWidgetItem *graphicsWidget)
    : UBW3CWebStorage(graphicsWidget)
    , mGraphicsW3CWidget(graphicsWidget)
{
    // NOOP
}



UBDocumentDatastoreAPI::~UBDocumentDatastoreAPI()
{
    // NOOP
}


QString UBDocumentDatastoreAPI::key(int index)
{
   QMap<QString, QString> entries = mGraphicsW3CWidget->datastoreEntries();

   if (index < entries.size())
       return entries.keys().at(index);
   else
       return "";

}


QString UBDocumentDatastoreAPI::getItem(const QString& key)
{
    QMap<QString, QString> entries = mGraphicsW3CWidget->datastoreEntries();
    if (entries.contains(key))
    {
        return entries.value(key);
    }
    else
    {
        return "";
    }
}


int UBDocumentDatastoreAPI::length()
{
   return mGraphicsW3CWidget->datastoreEntries().size();
}


void UBDocumentDatastoreAPI::setItem(const QString& key, const QString& value)
{
    if (mGraphicsW3CWidget)
    {
        mGraphicsW3CWidget->setDatastoreEntry(key, value);
    }
}


void UBDocumentDatastoreAPI::removeItem(const QString& key)
{
    mGraphicsW3CWidget->removeDatastoreEntry(key);
}


void UBDocumentDatastoreAPI::clear()
{
    mGraphicsW3CWidget->removeAllDatastoreEntries();
}


UBDatastoreAPI::UBDatastoreAPI(UBGraphicsW3CWidgetItem *widget)
    : QObject(widget)
{
    mDocumentDatastore = new UBDocumentDatastoreAPI(widget);
}


QObject* UBDatastoreAPI::document()
{
    return mDocumentDatastore;
}




