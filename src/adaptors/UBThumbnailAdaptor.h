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
#ifndef UBTHUMBNAILADAPTOR_H
#define UBTHUMBNAILADAPTOR_H

#include <QtCore>

class UBDocument;
class UBDocumentProxy;
class UBGraphicsScene;

class UBThumbnailAdaptor //static class
{
	Q_DECLARE_TR_FUNCTIONS(UBThumbnailAdaptor)

public:
    static QUrl thumbnailUrl(UBDocumentProxy* proxy, int pageIndex);

    static void persistScene(UBDocumentProxy* proxy, UBGraphicsScene* pScene, int pageIndex, bool overrideModified = false);

    static const QPixmap* get(UBDocumentProxy* proxy, int index);
    static void load(UBDocumentProxy* proxy, QList<const QPixmap*>& list);

private:
    static void generateMissingThumbnails(UBDocumentProxy* proxy);

    UBThumbnailAdaptor() {}
};

#endif // UBTHUMBNAILADAPTOR_H
