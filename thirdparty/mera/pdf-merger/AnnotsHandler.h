#if !defined AnnotsHandler_h
#define AnnotsHandler_h

#include "RemoveHimSelfHandler.h"
//this class is needed to process "Annots" field
//AnnotsHandler parses Annots field of Page object and fills
//annots container with Annots objects
namespace merge_lib
{
   class AnnotsHandler: public RemoveHimselfHandler
   {
   public:
      AnnotsHandler(Object * page, const std::string & handlerName, std::vector<Object *> & annots): 
                        RemoveHimselfHandler(page, handlerName),
                        _annotations(annots)

      {
        _setHandlerName(handlerName);
      }

      
   private:
      //methods
      void _processObjectContent(unsigned int startOfPageElement);
       

      //memebers
      std::vector<Object *> & _annotations;
      
   };
}
#endif

