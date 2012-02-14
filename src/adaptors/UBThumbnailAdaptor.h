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

class UBThumbnailAdaptor //static class
{
private: UBThumbnailAdaptor() {}
public:
    static void persistScene(const QString& pDocPath, UBGraphicsScene* pScene, int pageIndex, bool overrideModified = false);

    static QList<QPixmap> load(UBDocumentProxy* proxy);
	static QPixmap load(UBDocumentProxy* proxy, int index);

    static QUrl thumbnailUrl(UBDocumentProxy* proxy, int pageIndex);
};

#endif // UBTHUMBNAILADAPTOR_H
