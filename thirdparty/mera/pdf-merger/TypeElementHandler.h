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
