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
