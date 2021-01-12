/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include "UBW3CWidgetAPI.h"

#include <QtGui>

#include "core/UBApplication.h"
#include "core/UBApplicationController.h"

#include "web/UBWebController.h"

#include "domain/UBGraphicsWidgetItem.h"
#include "domain/UBGraphicsProxyWidget.h"

#include "UBWidgetMessageAPI.h"

#include "core/memcheck.h"

UBW3CWidgetAPI::UBW3CWidgetAPI(UBGraphicsW3CWidgetItem *graphicsWidget, QObject *parent)
    : QObject(parent)
    , mGraphicsW3CWidget(graphicsWidget)
{
    mPreferencesAPI = new UBW3CWidgetPreferenceAPI(graphicsWidget, parent);

}

UBW3CWidgetAPI::~UBW3CWidgetAPI()
{
    // NOOP
}


QString UBW3CWidgetAPI::uuid()
{
    if (mGraphicsW3CWidget)
        return mGraphicsW3CWidget->uuid().toString();
    else
        return "";
}


int UBW3CWidgetAPI::width()
{
    return mGraphicsW3CWidget->nominalSize().width();
}


int UBW3CWidgetAPI::height()
{
    return mGraphicsW3CWidget->nominalSize().height();
}


QString UBW3CWidgetAPI::id()
{
    return mGraphicsW3CWidget->metadatas().id;
}


QString UBW3CWidgetAPI::name()
{
    return mGraphicsW3CWidget->metadatas().name;
}


QString UBW3CWidgetAPI::description()
{
    return mGraphicsW3CWidget->metadatas().description;
}


QString UBW3CWidgetAPI::author()
{
    return mGraphicsW3CWidget->metadatas().author;
}


QString UBW3CWidgetAPI::authorEmail()
{
    return mGraphicsW3CWidget->metadatas().authorEmail;
}


QString UBW3CWidgetAPI::authorHref()
{
    return mGraphicsW3CWidget->metadatas().authorHref;
}


QString UBW3CWidgetAPI::version()
{
    return mGraphicsW3CWidget->metadatas().version;
}

QObject* UBW3CWidgetAPI::preferences()
{
    return mPreferencesAPI;
}


void UBW3CWidgetAPI::openURL(const QString& url)
{
    UBApplication::webController->loadUrl(QUrl(url));
}


UBW3CWidgetPreferenceAPI::UBW3CWidgetPreferenceAPI(UBGraphicsW3CWidgetItem *graphicsWidget, QObject *parent)
    : UBW3CWebStorage(parent)
    , mGraphicsW3CWidget(graphicsWidget)
{
    // NOOP
}

UBW3CWidgetPreferenceAPI::~UBW3CWidgetPreferenceAPI()
{
    // NOOP
}


QString UBW3CWidgetPreferenceAPI::key(int index)
{
  QMap<QString, UBGraphicsW3CWidgetItem::PreferenceValue> w3CPrefs = mGraphicsW3CWidget->preferences();

  if (index < w3CPrefs.size())
    return w3CPrefs.keys().at(index);
  else
    return "";
}

QString UBW3CWidgetPreferenceAPI::getItem(const QString& key)
{
  if (mGraphicsW3CWidget) {
    QMap<QString, QString> docPref = mGraphicsW3CWidget->UBGraphicsWidgetItem::preferences();
    if (docPref.contains(key))
      return docPref.value(key);
  

    QMap<QString, UBGraphicsW3CWidgetItem::PreferenceValue> w3cPrefs = mGraphicsW3CWidget->preferences();

    if (w3cPrefs.contains(key)) {
      UBGraphicsW3CWidgetItem::PreferenceValue pref = w3cPrefs.value(key);
      return pref.value;
    }
  }
  return QString();
}

int UBW3CWidgetPreferenceAPI::length()
{
   QMap<QString, UBGraphicsW3CWidgetItem::PreferenceValue> w3cPrefs = mGraphicsW3CWidget->preferences();

   return w3cPrefs.size();
}


void UBW3CWidgetPreferenceAPI::setItem(const QString& key, const QString& value)
{
  if (mGraphicsW3CWidget) {
    QMap<QString, UBGraphicsW3CWidgetItem::PreferenceValue> w3cPrefs = mGraphicsW3CWidget->preferences();

    if (w3cPrefs.contains(key) && !w3cPrefs.value(key).readonly)
      mGraphicsW3CWidget->setPreference(key, value);
  }
}




void UBW3CWidgetPreferenceAPI::removeItem(const QString& key)
{
  if (mGraphicsW3CWidget)
    mGraphicsW3CWidget->removePreference(key);
}


void UBW3CWidgetPreferenceAPI::clear()
{
  if (mGraphicsW3CWidget)
    mGraphicsW3CWidget->removeAllPreferences();
}




