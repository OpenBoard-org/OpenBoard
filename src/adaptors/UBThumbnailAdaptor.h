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
