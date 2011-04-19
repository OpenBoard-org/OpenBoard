/*
 * UBWebKitUtils.h
 *
 *  Created on: 24 feb. 2009
 *      Author: Luc
 */

#ifndef UBWEBKITUTILS_H_
#define UBWEBKITUTILS_H_

#include <QtWebKit>

class UBWebKitUtils
{
    public:
        UBWebKitUtils();
        virtual ~UBWebKitUtils();

        class HtmlObject
        {

            public:
                HtmlObject(const QString& pSource, int pWidth, int pHeight)
                    : source(pSource)
                    , width(pWidth)
                    , height(pHeight)
                {
                    // NOOP
                }

                QString source;
                int width;
                int height;

        };

        static QList<UBWebKitUtils::HtmlObject> objectsInFrame(QWebFrame* frame);

};

#endif /* UBWEBKITUTILS_H_ */
