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
            if(startOfParent == std::string::npos)
               break;
            std::vector <Object *> parents = parent->getChildrenByBounds(startOfParent, endOfParent);
            if(parents.size() != 1)
               break;
            parent = parents[0];
            std::string parentContent = parent->getObjectContent();
            unsigned int startOfMediaBox = parentContent.find(_handlerName);
            if(startOfMediaBox == std::string::npos)
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
