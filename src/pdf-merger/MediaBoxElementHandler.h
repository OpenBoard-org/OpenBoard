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
         return (_page->getObjectContent().find("/BBox") != std::string::npos) ? true : false;
      }
   };
}
#endif
