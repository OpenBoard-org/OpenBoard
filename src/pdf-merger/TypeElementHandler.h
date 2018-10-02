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




#if !defined TypeElementHandler_h
#define TypeElementHandler_h

#include "PageElementHandler.h"

namespace merge_lib
{
   class TypeElementHandler: public PageElementHandler
   {
   public:
      TypeElementHandler(Object * page): PageElementHandler(page)
      {
         _setHandlerName("/Type");
      }
      
   private:
      virtual void _changeObjectContent(unsigned int startOfPageElement)
      {
         std::string searchPattern("/Page");
         unsigned int startOfPage = _pageContent.find(searchPattern, startOfPageElement);
         _page->eraseContent(startOfPage, searchPattern.size());
         std::string xObject = " /XObject\n";
         _page->insertToContent(startOfPage, xObject);
         static std::string subtype("/Subtype /Form\n");
         _page->insertToContent(startOfPage + xObject.size(), subtype);
      }
   };
}

#endif
