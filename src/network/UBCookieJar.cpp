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




/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the demonstration applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
** $QT_END_LICENSE$
**
****************************************************************************/

#include "UBCookieJar.h"

#include "UBAutoSaver.h"

#include "core/UBSettings.h"

#include <QtGui>
#include <QWebSettings>
#include "core/memcheck.h"

static const unsigned int JAR_VERSION = 23;

QT_BEGIN_NAMESPACE
QDataStream &operator<<(QDataStream &stream, const QList<QNetworkCookie> &list)
{
    stream << JAR_VERSION;
    stream << quint32(list.size());
    for (int i = 0; i < list.size(); ++i)
        stream << list.at(i).toRawForm();
    return stream;
}

QDataStream &operator>>(QDataStream &stream, QList<QNetworkCookie> &list)
{
    list.clear();

    quint32 version;
    stream >> version;

    if (version != JAR_VERSION)
        return stream;

    quint32 count;
    stream >> count;
    for(quint32 i = 0; i < count; ++i)
    {
        QByteArray value;
        stream >> value;
        QList<QNetworkCookie> newCookies = QNetworkCookie::parseCookies(value);
        if (newCookies.count() == 0 && value.length() != 0) {
            qWarning() << "CookieJar: Unable to parse saved cookie:" << value;
        }
        for (int j = 0; j < newCookies.count(); ++j)
            list.append(newCookies.at(j));
        if (stream.atEnd())
            break;
    }
    return stream;
}
QT_END_NAMESPACE

UBCookieJar::UBCookieJar(QObject *parent)
    : QNetworkCookieJar(parent)
    , mLoaded(false)
    , mSaveTimer(new UBAutoSaver(this))
    , mAcceptCookies(AcceptOnlyFromSitesNavigatedTo)
{
    // NOOP
}

UBCookieJar::~UBCookieJar()
{
    if (mKeepCookies == KeepUntilExit)
        clear();
    mSaveTimer->saveIfNeccessary();
}

void UBCookieJar::clear()
{
    setAllCookies(QList<QNetworkCookie>());
    mSaveTimer->changeOccurred();
    emit cookiesChanged();
}

void UBCookieJar::load()
{
    if (mLoaded)
        return;
    // load cookies and exceptions
    qRegisterMetaTypeStreamOperators<QList<QNetworkCookie> >("QList<QNetworkCookie>");

    QSettings cookieSettings(UBSettings::userDataDirectory() + QLatin1String("/cookies.ini"), QSettings::IniFormat);
    QVariant vCookies = cookieSettings.value(QLatin1String("cookies"));
    QList<QNetworkCookie> cookies = qvariant_cast<QList<QNetworkCookie> >(vCookies);

    setAllCookies(cookies);
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    mExceptionsBlock = cookieSettings.value(QLatin1String("block")).toStringList();
    mExceptionsAllow = cookieSettings.value(QLatin1String("allow")).toStringList();
    mExceptionsAllowForSession = cookieSettings.value(QLatin1String("allowForSession")).toStringList();
    qSort(mExceptionsBlock.begin(), mExceptionsBlock.end());
    qSort(mExceptionsAllow.begin(), mExceptionsAllow.end());
    qSort(mExceptionsAllowForSession.begin(), mExceptionsAllowForSession.end());

    loadSettings();
}

void UBCookieJar::loadSettings()
{
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QByteArray value = settings.value(QLatin1String("acceptCookies"),
                        QLatin1String("AcceptOnlyFromSitesNavigatedTo")).toByteArray();
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    mAcceptCookies = acceptPolicyEnum.keyToValue(value) == -1 ?
                        AcceptOnlyFromSitesNavigatedTo :
                        static_cast<AcceptPolicy>(acceptPolicyEnum.keyToValue(value));

    value = settings.value(QLatin1String("keepCookiesUntil"), QLatin1String("KeepUntilExpire")).toByteArray();
    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    mKeepCookies = keepPolicyEnum.keyToValue(value) == -1 ?
                        KeepUntilExpire :
                        static_cast<KeepPolicy>(keepPolicyEnum.keyToValue(value));

    if (mKeepCookies == KeepUntilExit)
        setAllCookies(QList<QNetworkCookie>());

    mLoaded = true;
    emit cookiesChanged();
}

void UBCookieJar::save()
{
    if (!mLoaded)
        return;
    purgeOldCookies();
    QString directory = UBSettings::userDataDirectory();
    if (directory.isEmpty())
        directory = QDir::homePath() + QLatin1String("/.") + QCoreApplication::applicationName();
    if (!QFile::exists(directory)) {
        QDir dir;
        dir.mkpath(directory);
    }
    QSettings cookieSettings(directory + QLatin1String("/cookies.ini"), QSettings::IniFormat);
    QList<QNetworkCookie> cookies = allCookies();
    for (int i = cookies.count() - 1; i >= 0; --i) {
        if (cookies.at(i).isSessionCookie())
            cookies.removeAt(i);
    }
    cookieSettings.setValue(QLatin1String("cookies"), qVariantFromValue<QList<QNetworkCookie> >(cookies));
    cookieSettings.beginGroup(QLatin1String("Exceptions"));
    cookieSettings.setValue(QLatin1String("block"), mExceptionsBlock);
    cookieSettings.setValue(QLatin1String("allow"), mExceptionsAllow);
    cookieSettings.setValue(QLatin1String("allowForSession"), mExceptionsAllowForSession);

    // save cookie settings
    QSettings settings;
    settings.beginGroup(QLatin1String("cookies"));
    QMetaEnum acceptPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("AcceptPolicy"));
    settings.setValue(QLatin1String("acceptCookies"), QLatin1String(acceptPolicyEnum.valueToKey(mAcceptCookies)));

    QMetaEnum keepPolicyEnum = staticMetaObject.enumerator(staticMetaObject.indexOfEnumerator("KeepPolicy"));
    settings.setValue(QLatin1String("keepCookiesUntil"), QLatin1String(keepPolicyEnum.valueToKey(mKeepCookies)));
}

void UBCookieJar::purgeOldCookies()
{
    QList<QNetworkCookie> cookies = allCookies();
    if (cookies.isEmpty())
        return;
    int oldCount = cookies.count();
    QDateTime now = QDateTime::currentDateTime();
    for (int i = cookies.count() - 1; i >= 0; --i) {
        if (!cookies.at(i).isSessionCookie() && cookies.at(i).expirationDate() < now)
            cookies.removeAt(i);
    }
    if (oldCount == cookies.count())
        return;
    setAllCookies(cookies);
    emit cookiesChanged();
}

QList<QNetworkCookie> UBCookieJar::cookiesForUrl(const QUrl &url) const
{
    UBCookieJar *that = const_cast<UBCookieJar*>(this);
    if (!mLoaded)
        that->load();

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled)) {
        QList<QNetworkCookie> noCookies;
        return noCookies;
    }

    return QNetworkCookieJar::cookiesForUrl(url);
}

bool UBCookieJar::setCookiesFromUrl(const QList<QNetworkCookie> &cookieList, const QUrl &url)
{
    if (!mLoaded)
        load();

    QWebSettings *globalSettings = QWebSettings::globalSettings();
    if (globalSettings->testAttribute(QWebSettings::PrivateBrowsingEnabled))
        return false;

    QString host = url.host();
    bool eBlock = qBinaryFind(mExceptionsBlock.begin(), mExceptionsBlock.end(), host) != mExceptionsBlock.end();
    bool eAllow = qBinaryFind(mExceptionsAllow.begin(), mExceptionsAllow.end(), host) != mExceptionsAllow.end();
    bool eAllowSession = qBinaryFind(mExceptionsAllowForSession.begin(), mExceptionsAllowForSession.end(), host) != mExceptionsAllowForSession.end();

    bool addedCookies = false;
    // pass exceptions
    bool acceptInitially = (mAcceptCookies != AcceptNever);
    if ((acceptInitially && !eBlock)
        || (!acceptInitially && (eAllow || eAllowSession))) {
        // pass url domain == cookie domain
        QDateTime soon = QDateTime::currentDateTime();
        soon = soon.addDays(90);
        foreach(QNetworkCookie cookie, cookieList) {
            QList<QNetworkCookie> lst;
            if (mKeepCookies == KeepUntilTimeLimit
                && !cookie.isSessionCookie()
                && cookie.expirationDate() > soon) {
                    cookie.setExpirationDate(soon);
            }
            lst += cookie;
            if (QNetworkCookieJar::setCookiesFromUrl(lst, url)) {
                addedCookies = true;
            } else {
                // finally force it in if wanted
                if (mAcceptCookies == AcceptAlways) {
                    QList<QNetworkCookie> cookies = allCookies();
                    cookies += cookie;
                    setAllCookies(cookies);
                    addedCookies = true;
                }
#if 0
                else
                    qWarning() << "setCookiesFromUrl failed" << url << cookieList.value(0).toRawForm();
#endif
            }
        }
    }

    if (addedCookies) {
        mSaveTimer->changeOccurred();
        emit cookiesChanged();
    }
    return addedCookies;
}

UBCookieJar::AcceptPolicy UBCookieJar::acceptPolicy() const
{
    if (!mLoaded)
        (const_cast<UBCookieJar*>(this))->load();
    return mAcceptCookies;
}

void UBCookieJar::setAcceptPolicy(AcceptPolicy policy)
{
    if (!mLoaded)
        load();
    if (policy == mAcceptCookies)
        return;
    mAcceptCookies = policy;
    mSaveTimer->changeOccurred();
}

UBCookieJar::KeepPolicy UBCookieJar::keepPolicy() const
{
    if (!mLoaded)
        (const_cast<UBCookieJar*>(this))->load();
    return mKeepCookies;
}

void UBCookieJar::setKeepPolicy(KeepPolicy policy)
{
    if (!mLoaded)
        load();
    if (policy == mKeepCookies)
        return;
    mKeepCookies = policy;
    mSaveTimer->changeOccurred();
}

QStringList UBCookieJar::blockedCookies() const
{
    if (!mLoaded)
        (const_cast<UBCookieJar*>(this))->load();
    return mExceptionsBlock;
}

QStringList UBCookieJar::allowedCookies() const
{
    if (!mLoaded)
        (const_cast<UBCookieJar*>(this))->load();
    return mExceptionsAllow;
}

QStringList UBCookieJar::allowForSessionCookies() const
{
    if (!mLoaded)
        (const_cast<UBCookieJar*>(this))->load();
    return mExceptionsAllowForSession;
}

void UBCookieJar::setBlockedCookies(const QStringList &list)
{
    if (!mLoaded)
        load();
    mExceptionsBlock = list;
    qSort(mExceptionsBlock.begin(), mExceptionsBlock.end());
    mSaveTimer->changeOccurred();
}

void UBCookieJar::setAllowedCookies(const QStringList &list)
{
    if (!mLoaded)
        load();
    mExceptionsAllow = list;
    qSort(mExceptionsAllow.begin(), mExceptionsAllow.end());
    mSaveTimer->changeOccurred();
}

void UBCookieJar::setAllowForSessionCookies(const QStringList &list)
{
    if (!mLoaded)
        load();
    mExceptionsAllowForSession = list;
    qSort(mExceptionsAllowForSession.begin(), mExceptionsAllowForSession.end());
    mSaveTimer->changeOccurred();
}

