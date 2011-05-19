#if !defined PageElementHandler_h
#define PageElementHandler_h

#include <string>
#include <set>
#include "Object.h"
#include "Parser.h"


namespace merge_lib
{
   //base class for all elements handlers
   //Handlers are needed to process Page fields during the merge
   //this is the chain of responsibility pattern
   class PageElementHandler
   {
   public:

      PageElementHandler(Object * page): _page(page), _pageContent(page->_content), _nextHandler(0)
      {
         _createAllPageFieldsSet();
      }
      virtual ~PageElementHandler()
      {
         delete _nextHandler;
      }
      void addNextHandler(PageElementHandler * nextHandler)
      {
         _nextHandler = nextHandler;
      }

      void processObjectContent()
      {
         unsigned int startOfPageElement = _findStartOfPageElement();
         if(startOfPageElement != std::string::npos)
            _processObjectContent(startOfPageElement);
         if(_nextHandler)
            _nextHandler->processObjectContent();
      }

      void changeObjectContent()
      {
         unsigned int startOfPageElement = _findStartOfPageElement();
         if(startOfPageElement != std::string::npos)
            _changeObjectContent(startOfPageElement);
         else
            _pageElementNotFound();
         if(_nextHandler)
            _nextHandler->changeObjectContent();
      }

   protected:   
      //methods
      void _setHandlerName(const std::string & handlerName)
      {
         _handlerName = handlerName;
      }
      unsigned int _findEndOfElementContent(unsigned int startOfPageElement);
      void _createAllPageFieldsSet();

      //members   
      std::string & _pageContent;
      Object * _page;
      std::string _handlerName;  
      PageElementHandler * _nextHandler;

   private:
      //methods
      virtual void _processObjectContent(unsigned int startOfPageElement){};
      virtual void _changeObjectContent(unsigned int startOfPageElement) = 0;
      virtual void _pageElementNotFound() {};
      unsigned int _findStartOfPageElement()
      {
         return Parser::findToken(_pageContent,_handlerName);
      }
      //members


      static std::set<std::string> _allPageFields;

   };
}
#endif

