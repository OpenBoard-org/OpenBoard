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

      PageElementHandler(Object * page): m_page(page), m_pageContent(page->m_content), m_nextHandler(0)
      {
         createAllPageFieldsSet();
      }
      virtual ~PageElementHandler()
      {
         delete m_nextHandler;
      }
      void addNextHandler(PageElementHandler * nextHandler)
      {
         m_nextHandler = nextHandler;
      }

      void processObjectContent()
      {
         unsigned int startOfPageElement = findStartOfPageElement();
         if((int)startOfPageElement != -1)
            processObjectContentImpl(startOfPageElement);
         if(m_nextHandler)
            m_nextHandler->processObjectContent();
      }

      void changeObjectContent()
      {
         unsigned int startOfPageElement = findStartOfPageElement();
         if((int)startOfPageElement != -1)
            changeObjectContentImpl(startOfPageElement);
         else
            pageElementNotFound();
         if(m_nextHandler)
            m_nextHandler->changeObjectContent();
      }

   protected:   
      //methods
      void setHandlerName(const std::string & handlerName)
      {
         m_handlerName = handlerName;
      }
      unsigned int findEndOfElementContent(unsigned int startOfPageElement);
      void createAllPageFieldsSet();

      //members   
      Object * m_page;
      std::string & m_pageContent;
      std::string m_handlerName;  
      PageElementHandler * m_nextHandler;

   private:
      //methods
      virtual void processObjectContentImpl(unsigned int startOfPageElement);
      virtual void changeObjectContentImpl(unsigned int startOfPageElement) = 0;
      virtual void pageElementNotFound() {};
      unsigned int findStartOfPageElement()
      {
         return Parser::findToken(m_pageContent,m_handlerName);
      }
      //members


      static std::set<std::string> s_allPageFields;

   };
}
#endif

