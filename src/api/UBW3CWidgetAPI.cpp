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

#include "UBW3CWidgetAPI.h"

#include <QtGui>
#include <QtWebKit>

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"

#include "web/UBWebController.h"

#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsProxyWidget.h"

#include "UBWidgetMessageAPI.h"

#include "core/memcheck.h"

UBW3CWidgetAPI::UBW3CWidgetAPI(UBGraphicsW3CWidgetItem *graphicsWidget)
    : QObject(graphicsWidget)
    , mGraphicsW3CWidget(graphicsWidget)
    , mW3CWidget(mGraphicsW3CWidget->w3cWidget())
{
    mPreferencesAPI = new UBW3CWidgetPreferenceAPI(graphicsWidget);

}


UBW3CWidgetAPI::UBW3CWidgetAPI(UBW3CWidget *widget)
    : QObject(widget)
    , mGraphicsW3CWidget(0)
    , mW3CWidget(widget)
{
    mPreferencesAPI = new UBW3CWidgetPreferenceAPI(widget);
}


UBW3CWidgetAPI::~UBW3CWidgetAPI()
{
    // NOOP
}


QString UBW3CWidgetAPI::uuid()
{
    if (mGraphicsW3CWidget)
        return mGraphicsW3CWidget->uuid();
    else
        return "";
}


int UBW3CWidgetAPI::width()
{
    return mW3CWidget->geometry().width();
}


int UBW3CWidgetAPI::height()
{
    return mW3CWidget->geometry().height();
}


QString UBW3CWidgetAPI::id()
{
    return mW3CWidget->metadatas().id;
}


QString UBW3CWidgetAPI::name()
{
    return mW3CWidget->metadatas().name;
}


QString UBW3CWidgetAPI::description()
{
    return mW3CWidget->metadatas().description;
}


QString UBW3CWidgetAPI::author()
{
    return mW3CWidget->metadatas().author;
}


QString UBW3CWidgetAPI::authorEmail()
{
    return mW3CWidget->metadatas().authorEmail;
}


QString UBW3CWidgetAPI::authorHref()
{
    return mW3CWidget->metadatas().authorHref;
}


QString UBW3CWidgetAPI::version()
{
    return mW3CWidget->metadatas().version;
}

QObject* UBW3CWidgetAPI::preferences()
{
    return mPreferencesAPI;
}


void UBW3CWidgetAPI::openURL(const QString& url)
{
    UBApplication::webController->loadUrl(QUrl(url));
}


UBW3CWidgetPreferenceAPI::UBW3CWidgetPreferenceAPI(UBGraphicsW3CWidgetItem *graphicsWidget)
    : UBW3CWebStorage(graphicsWidget)
    , mGraphicsW3CWidget(graphicsWidget)
    , mW3CWidget(graphicsWidget->w3cWidget())
{
    // NOOP
}

UBW3CWidgetPreferenceAPI::UBW3CWidgetPreferenceAPI(UBW3CWidget *widget)
    : UBW3CWebStorage(widget)
    , mGraphicsW3CWidget(0)
    , mW3CWidget(widget)
{
    // NOOP
}



UBW3CWidgetPreferenceAPI::~UBW3CWidgetPreferenceAPI()
{
    // NOOP
}


QString UBW3CWidgetPreferenceAPI::key(int index)
{
   QMap<QString, UBW3CWidget::PreferenceValue> w3CPrefs = mW3CWidget->preferences();

   if (index < w3CPrefs.size())
       return w3CPrefs.keys().at(index);
   else
       return "";

}


QString UBW3CWidgetPreferenceAPI::getItem(const QString& key)
{
   if (mGraphicsW3CWidget)
   {
       QMap<QString, QString> docPref = mGraphicsW3CWidget->preferences();

       if (docPref.contains(key))
           return docPref.value(key);
   }

   QMap<QString, UBW3CWidget::PreferenceValue> w3cPrefs = mW3CWidget->preferences();

   if (w3cPrefs.contains(key))
   {
       UBW3CWidget::PreferenceValue pref = w3cPrefs.value(key);

       return pref.value;
   }
   else
   {
       return "";
   }

}


int UBW3CWidgetPreferenceAPI::length()
{
   QMap<QString, UBW3CWidget::PreferenceValue> w3cPrefs = mW3CWidget->preferences();

   return w3cPrefs.size();
}


void UBW3CWidgetPreferenceAPI::setItem(const QString& key, const QString& value)
{
   if (mGraphicsW3CWidget)
   {
       QMap<QString, UBW3CWidget::PreferenceValue> w3cPrefs = mW3CWidget->preferences();

       if (w3cPrefs.contains(key) && !w3cPrefs.value(key).readonly)
           mGraphicsW3CWidget->setPreference(key, value);
   }
   else
   {
       // No state for non graphics widget (aka tool widget)
   }
}


void UBW3CWidgetPreferenceAPI::removeItem(const QString& key)
{
   if (mGraphicsW3CWidget)
       mGraphicsW3CWidget->removePreference(key);
   else
   {
       // No state for non graphics widget (aka tool widget)
   }
}


void UBW3CWidgetPreferenceAPI::clear()
{
    if (mGraphicsW3CWidget)
        mGraphicsW3CWidget->removeAllPreferences();
    else
    {
       // No state for non graphics widget (aka tool widget)
    }
}




