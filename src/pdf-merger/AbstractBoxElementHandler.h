/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#if !defined AbstractBoxElementHandler_h
#define AbstractBoxElementHandler_h

#include "PageElementHandler.h"


namespace merge_lib
{
   //class for processing MediaBox field of Page object
   class AbstractBoxElementHandler: public PageElementHandler
   {
   public:
      AbstractBoxElementHandler(Object * page): PageElementHandler(page)
      {        
      }
      virtual ~AbstractBoxElementHandler()
      {
      }

   protected:

      void _retrieveBoxFromParent()
      {                  
         std::string content = _page->getObjectContent();
         std::string mediaBox;
         Object * parent = _page;
         while(1)
         {
            unsigned int startOfParent = content.find("/Parent");
            unsigned int endOfParent = content.find(" R", startOfParent);
            if((int)startOfParent == -1)
               break;
            std::vector <Object *> parents = parent->getChildrenByBounds(startOfParent, endOfParent);
            if(parents.size() != 1)
               break;
            parent = parents[0];
            std::string parentContent = parent->getObjectContent();
            unsigned int startOfMediaBox = parentContent.find(_handlerName);
            if((int)startOfMediaBox == -1)
            {
               content = parentContent;
               continue;
            }
            unsigned int endOfMediaBox = parentContent.find("]", startOfMediaBox);
            mediaBox = parentContent.substr(startOfMediaBox, endOfMediaBox - startOfMediaBox + 1);
            break;
         }
         if(!mediaBox.empty())
         {
            unsigned int startOfMediaBox = _page->getObjectContent().rfind(">>");
            _page->insertToContent(startOfMediaBox, mediaBox);
            _changeObjectContent(startOfMediaBox);            
         }            
      }
   private:
      virtual void _changeObjectContent(unsigned int startOfPageElement) = 0;
   };
}
#endif
