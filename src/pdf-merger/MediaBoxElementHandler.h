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




#if !defined MediaBoxElementHandler_h
#define MediaBoxElementHandler_h

#include "AbstractBoxElementHandler.h"
#include "RemoveHimSelfHandler.h"
#include <memory>

namespace merge_lib
{
   //class for processing MediaBox field of Page object
   class MediaBoxElementHandler: public AbstractBoxElementHandler
   {
   public:
      MediaBoxElementHandler(Object * page): AbstractBoxElementHandler(page)
      {
         setHandlerName("/MediaBox");
      }
      virtual ~MediaBoxElementHandler()
      {
      }

   private:

      //replace MediaBox with BBox
      virtual void changeObjectContentImpl(unsigned int startOfPageElement)
      {
         if(wasCropBoxHandlerCalled())
         {
            PageElementHandler * tempNextHandler = m_nextHandler;
            m_nextHandler = new RemoveHimselfHandler(m_page, m_handlerName);
            m_nextHandler->addNextHandler(tempNextHandler);
            return;
         }
         m_page->eraseContent(startOfPageElement, m_handlerName.size());
         static std::string bbox("/BBox");
         static std::string matrix("/Matrix [ 1 0 0 1 0 0 ]\n");
         m_page->insertToContent(startOfPageElement, bbox);
         m_page->insertToContent(startOfPageElement, matrix);
      }
      void pageElementNotFound()
      {          
         if(wasCropBoxHandlerCalled())
            return;
         retrieveBoxFromParent();
      }
      bool wasCropBoxHandlerCalled()
      {
         return ((int)m_page->getObjectContent().find("/BBox") != -1) ? true : false;
      }
   };
}
#endif
