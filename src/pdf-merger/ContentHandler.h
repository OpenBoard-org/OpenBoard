#if !defined ContentHandler_h
#define ContentHandler_h

#include "PageElementHandler.h"

namespace merge_lib
{
   //this class is needed to process "Content" field of Page object
   class ContentHandler: public PageElementHandler
   {
   public:
      ContentHandler(Object * page, const std::string & handlerName): 
                PageElementHandler(page)
                     
      {
         _setHandlerName(handlerName);
      }
      virtual ~ContentHandler(){};

      
   private:
      //methods

      //concatedate stream of all objects which contains Content of Page
      void _processObjectContent(unsigned int startOfPageElement);

      //write concatenated stream to Page object
      void _changeObjectContent(unsigned int startOfPageElement);
      //get content of stream 
      // object - object with stream
      //leftBound - left bound of object's content
      //rightBound - right bound of object's content
      std::string _retrieveStreamContent(Object * object, unsigned int leftBound, unsigned int rightBound);

      //get stream from Arrey elemetns
      std::string _getStreamFromReferencies(Object * objectWithArray, unsigned int leftBound, unsigned int rightBound);

      //get stream from Object
      std::string _getStreamFromContent(Object * objectWithStream);

      //memebers
      std::string _concatenatedStream;
   };
}
#endif

