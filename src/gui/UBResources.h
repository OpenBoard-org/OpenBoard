/*
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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
