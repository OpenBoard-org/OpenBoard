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




#ifndef UBLIBRARYAPI_H_
#define UBLIBRARYAPI_H_

#include <QtGui>
#include <QWebView>

class UBLibraryAPI : public QObject
{
    Q_OBJECT;

    public:
        UBLibraryAPI(QWebView *pWebView = 0);
        virtual ~UBLibraryAPI();

    public slots:

        /**
         * add any supported objects (pictures/video/widget) centered at scene position x/y.
         * width and height may be supplied, this is useful for flash (.swf) objects
         * if background is true, the object is not selectable and sits in the lowest z pos possible
         *
         */
        void addObject(QString pUrl, int width = 0, int height = 0, int x = 0, int y = 0, bool background = false);

        /**
         * trigger a drag & drop of the given url content
         *
         */
        void startDrag(QString pUrl);


        void addTool(QString pUrl, int x = 0, int y = 0);


    signals:

       void downloadTriggered(const QUrl& url, const QPointF& pos, const QSize& pSize, bool background);


    private:
        QWebView* mWebView;


};

#endif /* UBLIBRARYAPI_H_ */
