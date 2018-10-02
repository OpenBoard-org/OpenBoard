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




#if !defined Document_h
#define Document_h

#include "Page.h"
#include <vector>

namespace merge_lib
{
   //this class contains all info about pdf document
   class Document
   {
   //only this class can fill out Document
   friend class Parser;
   friend class OverlayDocumentParser;

   public:
      ~Document();

      //get page by number
      //this method throws exception if page is absent
      Page *   getPage(unsigned int pageNumber);
      
      //save document with newFileName file name
      void     saveAs(const char * newFileName);   

      //get root of all document objects
      Object * getDocumentObject();

      //add objects to allObjects repository
      void addToAllObjects(const std::vector<Object *> & allObjs)
      {
         _allObjects.insert(_allObjects.end(), allObjs.begin(), allObjs.end());
      }

   private:
      //methods   
      Document(const char * docName);
      //members

      //root of all document's objects
      Object * _root;

      //document's pages
      //key - is a page number
      //values is a page
      std::map <unsigned int, Page *> _pages;

      //repository for all objects. 
      //this repository is using to destroy all document's objects in destructor
      std::vector<Object *> _allObjects;

      std::string _documentName;

      //max number of all document's objects
      unsigned int _maxObjectNumber;

   };
}
#endif
