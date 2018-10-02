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
         if((int)startOfPageElement != -1)
            _processObjectContent(startOfPageElement);
         if(_nextHandler)
            _nextHandler->processObjectContent();
      }

      void changeObjectContent()
      {
         unsigned int startOfPageElement = _findStartOfPageElement();
         if((int)startOfPageElement != -1)
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
      Object * _page;
      std::string & _pageContent;
      std::string _handlerName;  
      PageElementHandler * _nextHandler;

   private:
      //methods
      virtual void _processObjectContent(unsigned int startOfPageElement);
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

