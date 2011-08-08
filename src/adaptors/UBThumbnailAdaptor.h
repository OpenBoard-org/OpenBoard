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
#ifndef UBTHUMBNAILADAPTOR_H
#define UBTHUMBNAILADAPTOR_H

#include <QtCore>

class UBDocument;
class UBDocumentProxy;
class UBGraphicsScene;

class UBThumbnailAdaptor : public QObject
{
    Q_OBJECT;

public:

    UBThumbnailAdaptor(QObject *parent = 0);
    ~UBThumbnailAdaptor();

    static void persistScene(const QString& pDocPath, UBGraphicsScene* pScene, const int pageIndex,  const bool overrideModified = false);

    static QList<QPixmap> load(UBDocumentProxy* proxy);

    static QUrl thumbnailUrl(UBDocumentProxy* proxy, const int pageIndex);

};

#endif // UBTHUMBNAILADAPTOR_H
