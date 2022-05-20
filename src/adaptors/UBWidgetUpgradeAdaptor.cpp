/*
 * Copyright (C) 2015-2022 Département de l'Instruction Publique (DIP-SEM)
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


#include "UBWidgetUpgradeAdaptor.h"

#include "core/UBPersistenceManager.h"
#include "document/UBDocumentProxy.h"
#include "frameworks/UBPlatformUtils.h"

#include <QCryptographicHash>
#include <QDomDocument>

#include "core/memcheck.h"

UBWidgetUpgradeAdaptor::UBWidgetUpgradeAdaptor()
{

}

void UBWidgetUpgradeAdaptor::upgradeWidgets(UBDocumentProxy *proxy)
{
    if (libraryWidgets.empty())
    {
        fillLibraryWidgets();
    }

    // prepare a list of widgets and interactivities mapping the name to the list of provided features
    // this should be a one-time effort
    // get list of widgets in document
    QStringList widgetPaths = UBPersistenceManager::persistenceManager()->allWidgets(proxy->persistencePath() +  "/" + UBPersistenceManager::widgetDirectory);

    for (QString wigetPath : widgetPaths) {
        Widget widget = Widget(wigetPath);

        if (widget.valid() && libraryWidgets.contains(widget.id()))
        {
            Widget libraryWidget = libraryWidgets[widget.id()];

            if (widget.version() < "2.0" && libraryWidget.version() >= "2.0")
            {
                // upgrade needed
                copyDir(widget.path(), libraryWidget.path());
            }
        }
    }
}

void UBWidgetUpgradeAdaptor::copyDir(QDir target, QDir source)
{
    // copy all files from source to target
    QFileInfoList infoList = source.entryInfoList(QDir::Files | QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QFileInfo& info : infoList) {
        if (info.isDir()) {
            // optionally create directory in target and then copy content
            target.mkdir(info.fileName());
            target.cd(info.fileName());
            source.cd(info.fileName());
            copyDir(target, source);
            source.cdUp();
            target.cdUp();
        }
        else
        {
            // copy content of file
            QFile file(info.filePath());
            QString to = target.path() + "/" + info.fileName();
            QFile::remove(to);
            file.copy(to);
        }
    }
}

void UBWidgetUpgradeAdaptor::fillLibraryWidgets()
{
    QStringList widgetPaths = UBPersistenceManager::persistenceManager()->allWidgets(UBSettings::settings()->applicationApplicationsLibraryDirectory());
    widgetPaths << UBPersistenceManager::persistenceManager()->allWidgets(UBSettings::settings()->applicationInteractivesDirectory());

    for (QString wigetPath : widgetPaths) {
        Widget widget = Widget(wigetPath);

        if (widget.valid())
        {
            libraryWidgets.insert(widget.id(), widget);
        }
    }
}


UBWidgetUpgradeAdaptor::Widget::Widget(const QString &dir) : m_path(dir)
{
    // widgetHashes are used to identify widgets by the MD% sum of their config.xml
    // neccessary because many interactivities share the same id
    static QMap<QByteArray, QString> widgetHashes;

    if (widgetHashes.empty())
    {
        QFile widgetsMd5sum(UBPlatformUtils::applicationResourcesDirectory() + "/etc/widgets.md5sum");

        if (widgetsMd5sum.open(QIODevice::ReadOnly))
        {
            while (!widgetsMd5sum.atEnd()) {
                QByteArray line = widgetsMd5sum.readLine();
                QList<QByteArray> tokens = line.split(',');

                if (tokens.size() == 3)
                {
                    QByteArray md5sum = QByteArray::fromHex(tokens[0]);
                    widgetHashes[md5sum] = tokens[1];
                }
            }
        }
    }

    // read config.xml
    QString configPath = QString("%0/config.xml").arg(dir);

    QFile f(configPath);

    if (f.open(QIODevice::ReadOnly))
    {
        QByteArray data = f.readAll();

        // compute md5sum
        QByteArray hash = QCryptographicHash::hash(data, QCryptographicHash::Md5);

        if (widgetHashes.contains(hash))
        {
            // take id from hash list, not from config.xml
            m_id = widgetHashes[hash];
        }

        QDomDocument domDoc;
        domDoc.setContent(data);
        f.close();
        QDomElement root = domDoc.documentElement();

        if (m_id.isEmpty())
        {
            m_id = root.attribute("id", root.attribute("identifier", ""));
        }

        m_version = root.attribute("version", "");
    }
}

bool UBWidgetUpgradeAdaptor::Widget::operator==(const UBWidgetUpgradeAdaptor::Widget &other) const
{
    bool same = m_id == other.m_id && m_version == other.m_version;

    return same;
}

bool UBWidgetUpgradeAdaptor::Widget::valid()
{
    return !m_id.isEmpty() && !m_version.isEmpty();
}

QString UBWidgetUpgradeAdaptor::Widget::path() const
{
    return m_path;
}

QString UBWidgetUpgradeAdaptor::Widget::id() const
{
    return m_id;
}

QString UBWidgetUpgradeAdaptor::Widget::version() const
{
    return m_version;
}
