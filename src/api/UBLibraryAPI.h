/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#ifndef UBLIBRARYAPI_H_
#define UBLIBRARYAPI_H_

#include <QtGui>
#include <QtWebKit>

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
