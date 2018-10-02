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




#if !defined RotationHandler_h
#define RotationHandler_h

#include <QtGlobal>

#include "PageElementHandler.h"
#include "Page.h"

namespace merge_lib
{
   //This class remove field from Page object's content.
   class RotationHandler: public PageElementHandler
   {
   public:
      RotationHandler(Object * page, const std::string & handlerName, Page & basePage): 
         PageElementHandler(page),
            _basePage(basePage)
         {
            _setHandlerName(handlerName);        
         }
         virtual ~RotationHandler()
         {
         }
   private:
      //methods
      virtual void _processObjectContent(unsigned int startOfPageElement)
      {
         unsigned int endOfElement = _findEndOfElementContent(startOfPageElement);
         std::string rotationField = _page->getObjectContent().substr(startOfPageElement, endOfElement - startOfPageElement);
         std::string numbers("1234567890");
         unsigned int startOfNumber = rotationField.find_first_of(numbers);
         if( startOfNumber > 0 )
         {
            if( rotationField[startOfNumber-1] == '-' )
            {
               startOfNumber--; // negative number
            }
         }
         unsigned int endOfNumber = rotationField.find_first_not_of(numbers, startOfNumber + 1);
         std::string rotationStr = rotationField.substr(startOfNumber, endOfNumber - startOfNumber + 1);
         int rotation = 0;
         std::stringstream strin(rotationStr);
         strin>>rotation;
         _basePage.setRotation(rotation);
      }
      virtual void _changeObjectContent(unsigned int startOfPageElement) {Q_UNUSED(startOfPageElement);};

      //members
      Page & _basePage;
   };
}
#endif

