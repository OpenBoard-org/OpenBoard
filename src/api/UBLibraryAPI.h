/*
 * UBLibraryAPI.h
 *
 *  Created on: 9 juil. 2009
 *      Author: Luc
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
