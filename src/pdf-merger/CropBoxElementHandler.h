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




#ifndef CROPBOX_ELEMENT_HANDLER_H
#define CROPBOX_ELEMENT_HANDLER_H

#include "AbstractBoxElementHandler.h"
#include "Rectangle.h"

namespace merge_lib
{
   //class for processing CropBox field of Page object
   class CropBoxElementHandler: public AbstractBoxElementHandler
   {
   public:
      CropBoxElementHandler(Object * page): AbstractBoxElementHandler(page)
      {
         _setHandlerName("/CropBox");
      }
      virtual ~CropBoxElementHandler()
      {
      }
   private:

      //replace CropBox with BBox
      virtual void _changeObjectContent(unsigned int startOfPageElement)
      {
         Rectangle mediaBox("/CropBox", _page->getObjectContent());

         double shiftX = Utils::doubleEquals(mediaBox.x1,0)?0:-mediaBox.x1;
         double shiftY = Utils::doubleEquals(mediaBox.y1,0)?0:-mediaBox.y1;

         mediaBox.setNewRectangleName("/BBox");

         unsigned int endOfElement = _findEndOfElementContent(startOfPageElement);
         _page->forgetAboutChildren(startOfPageElement,endOfElement);
         _page->eraseContent(startOfPageElement,endOfElement-startOfPageElement);

         std::string newContent;
         mediaBox.appendRectangleToString(newContent," ");
         _page->insertToContent(startOfPageElement, newContent);

         std::stringstream matrix;
         matrix<<"/Matrix [ 1 0 0 1 "<<shiftX<<" "<< shiftY<<" ]\n";
         _page->insertToContent(startOfPageElement, matrix.str());
      }
      void _pageElementNotFound()
      {                  
         _retrieveBoxFromParent();
      }
   };
}

#endif // CROPBOX_ELEMENT_HANDLER_H

