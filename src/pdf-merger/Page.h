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




#if !defined Page_h
#define Page_h

#include <string>
#include "Object.h"
#include "MergePageDescription.h"


namespace merge_lib
{
   class Document;


   class Page
   {
      friend class Parser;
      friend class PageElementsFactory;


   public:
      Page(unsigned int pageNumber);
      ~Page();
      unsigned int getPageNumber()
      {
         return _pageNumber;
      }
      void merge(Page * sourcePage, Document * parentDocument, MergePageDescription & description, bool isPageDuplicated);

      void recalculateObjectNumbers(unsigned int & newNumber);
      std::string & getPageContent();
      const Object::Children &   getPageRefs();
      Object * pageToXObject(std::vector<Object *> & allObjects, std::vector<Object *> & annots, bool isCloneNeeded);
      void setRotation(int rotation)
      {
         _rotation = rotation;
      }

   private:
      //methods
      Object * _pageToXObject(Object *& page, std::vector<Object *> & annots);
      std::string _getMergedPageContent(  unsigned int & contentPosition, 
         unsigned int & parentPosition, 
         unsigned int & originalPage1Position, 
         unsigned int & originalPage2Position,
         std::pair<unsigned int, unsigned int> originalPageNumbers, 
         const MergePageDescription & description,
         Object * basePage,
         const std::vector<Object *> & annots,
         std::vector <Object::ChildAndItPositionInContent> & annotsPositions
         );
      //members
      Object * _root;
      unsigned int _pageNumber;
      int _rotation;

   };
}
#endif 
