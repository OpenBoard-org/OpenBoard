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
