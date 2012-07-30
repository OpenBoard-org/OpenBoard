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

#ifndef UBRESOURCES_H_
#define UBRESOURCES_H_

#include <QtGui>

class UBResources : public QObject
{
    Q_OBJECT;

    public:
         static UBResources* resources();
         QStringList customFontList() { return mCustomFontList; }


    private:
         UBResources(QObject* pParent = 0);
         virtual ~UBResources();

         void init();

         static UBResources* sSingleton;
         void buildFontList();
         QStringList mCustomFontList;

    public:

         QCursor penCursor;
         QCursor eraserCursor;
         QCursor markerCursor;
         QCursor pointerCursor;
         QCursor handCursor;
         QCursor zoomInCursor;
         QCursor zoomOutCursor;
         QCursor arrowCursor;
         QCursor playCursor;
         QCursor textCursor;
         QCursor rotateCursor;
		 QCursor drawLineRulerCursor;
};

#endif /* UBRESOURCES_H_ */
