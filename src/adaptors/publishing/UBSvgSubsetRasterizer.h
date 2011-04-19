/*
 * UBSvgSubsetRasterizer.h
 *
 *  Created on: 23 mars 2010
 *      Author: Luc
 */

#ifndef UBSVGSUBSETRASTERIZER_H_
#define UBSVGSUBSETRASTERIZER_H_

#include <QtGui>

class UBDocumentProxy;

class UBSvgSubsetRasterizer : QObject
{
    Q_OBJECT;

    public:
        UBSvgSubsetRasterizer(UBDocumentProxy* document, int pageIndex, QObject* parent = 0);
        virtual ~UBSvgSubsetRasterizer();

        bool rasterizeToFile(const QString& filename);

    private:
        UBDocumentProxy* mDocument;
        int mPageIndex;

};

#endif /* UBSVGSUBSETRASTERIZER_H_ */
