/*
 * Copyright (C) 2012 Webdoc SA
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
         _setHandlerName("/MediaBox");
      }
      virtual ~MediaBoxElementHandler()
      {
      }

   private:

      //replace MediaBox with BBox
      virtual void _changeObjectContent(unsigned int startOfPageElement)
      {
         if(_wasCropBoxHandlerCalled())
         {
            PageElementHandler * tempNextHandler = _nextHandler;            
            _nextHandler = new RemoveHimselfHandler(_page, _handlerName);
            _nextHandler->addNextHandler(tempNextHandler);
            return;
         }
         _page->eraseContent(startOfPageElement, _handlerName.size());
         static std::string bbox("/BBox");
         static std::string matrix("/Matrix [ 1 0 0 1 0 0 ]\n");
         _page->insertToContent(startOfPageElement, bbox);
         _page->insertToContent(startOfPageElement, matrix);
      }
      void _pageElementNotFound()
      {          
         if(_wasCropBoxHandlerCalled())
            return;
         _retrieveBoxFromParent();
      }
      bool _wasCropBoxHandlerCalled()
      {
         return ((int)_page->getObjectContent().find("/BBox") != -1) ? true : false;
      }
   };
}
#endif
