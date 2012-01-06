/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "UBAppleWidget.h"

#include <QtNetwork>

#include "web/UBWebKitUtils.h"

#include "network/UBNetworkAccessManager.h"

#include "core/memcheck.h"

UBAppleWidget::UBAppleWidget(const QUrl& pWidgetUrl, QWidget *parent)
    : UBAbstractWidget(pWidgetUrl, parent)
{
    QString path = pWidgetUrl.toLocalFile();

    if (!path.endsWith(".wdgt") && !path.endsWith(".wdgt/"))
    {
        int lastSlashIndex = path.lastIndexOf("/");
        if (lastSlashIndex > 0)
        {
            path = path.mid(0, lastSlashIndex + 1);
        }
    }

    QFile plistFile(path + "/Info.plist");
    plistFile.open(QFile::ReadOnly);

    QByteArray plistBin = plistFile.readAll();
    QString plist = QString::fromUtf8(plistBin);

    int mainHtmlIndex = plist.indexOf("MainHTML");
    int mainHtmlIndexStart = plist.indexOf("<string>", mainHtmlIndex);
    int mainHtmlIndexEnd = plist.indexOf("</string>", mainHtmlIndexStart);

    if (mainHtmlIndex > -1 && mainHtmlIndexStart > -1 && mainHtmlIndexEnd > -1)
    {
        mMainHtmlFileName = plist.mid(mainHtmlIndexStart + 8, mainHtmlIndexEnd - mainHtmlIndexStart - 8);
    }

    mMainHtmlUrl = pWidgetUrl;
    mMainHtmlUrl.setPath(pWidgetUrl.path() + "/" + mMainHtmlFileName);

    QWebView::load(mMainHtmlUrl);

    QPixmap defaultPixmap(pWidgetUrl.toLocalFile() + "/Default.png");

    setFixedSize(defaultPixmap.size());

    mNominalSize = defaultPixmap.size();
}


UBAppleWidget::~UBAppleWidget()
{
    // NOOP
}







