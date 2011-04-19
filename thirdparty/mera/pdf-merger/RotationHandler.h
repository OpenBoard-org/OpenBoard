#if !defined RotationHandler_h
#define RotationHandler_h

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
      virtual void _changeObjectContent(unsigned int startOfPageElement) {};

      //members
      Page & _basePage;
   };
}
#endif

