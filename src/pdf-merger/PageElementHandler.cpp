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




#include "PageElementHandler.h"
#include <QtGlobal>
#include "core/memcheck.h"

using namespace merge_lib;

std::set<std::string> PageElementHandler::_allPageFields;

void PageElementHandler::_createAllPageFieldsSet()
{
   if(!_allPageFields.empty())
      return;
   _allPageFields.insert(std::string("Type"));
   _allPageFields.insert(std::string("Parent"));
   _allPageFields.insert(std::string("LastModified"));
   _allPageFields.insert(std::string("Resources"));
   _allPageFields.insert(std::string("MediaBox"));
   _allPageFields.insert(std::string("CropBox"));
   _allPageFields.insert(std::string("BleedBox"));
   _allPageFields.insert(std::string("TrimBox"));
   _allPageFields.insert(std::string("ArtBox"));
   _allPageFields.insert(std::string("BoxColorInfo"));
   _allPageFields.insert(std::string("Contents"));
   _allPageFields.insert(std::string("Rotate"));
   _allPageFields.insert(std::string("Group"));
   _allPageFields.insert(std::string("Thumb"));
   _allPageFields.insert(std::string("B"));
   _allPageFields.insert(std::string("Dur"));
   _allPageFields.insert(std::string("Trans"));
   _allPageFields.insert(std::string("Annots"));
   _allPageFields.insert(std::string("AA"));
   _allPageFields.insert(std::string("Metadata"));
   _allPageFields.insert(std::string("PieceInfo"));
   _allPageFields.insert(std::string("StructParents"));   
   _allPageFields.insert(std::string("ID"));   
   _allPageFields.insert(std::string("PZ"));   
   _allPageFields.insert(std::string("SeparationInfo"));   
   _allPageFields.insert(std::string("Tabs"));   
   _allPageFields.insert(std::string("TemplateInstantiated"));   
   _allPageFields.insert(std::string("PresSteps"));   
   _allPageFields.insert(std::string("UserUnit"));   
   _allPageFields.insert(std::string("VP"));
   //for correct search all fields of XObject should be present to
   _allPageFields.insert(std::string("Subtype"));
   _allPageFields.insert(std::string("FormType"));
   _allPageFields.insert(std::string("BBox"));
   _allPageFields.insert(std::string("Matrix"));
   _allPageFields.insert(std::string("Ref"));
   _allPageFields.insert(std::string("StructParent"));
   _allPageFields.insert(std::string("OPI"));
   _allPageFields.insert(std::string("OC"));
   _allPageFields.insert(std::string("Name"));

}

unsigned int PageElementHandler::_findEndOfElementContent(unsigned int startOfPageElement)
{
   static std::string whitespacesAndDelimeters(" \t\f\v\n\r<<[/");
   unsigned int foundSlash = _pageContent.find("/", startOfPageElement + 1);
   std::string fieldType;
   while((int)foundSlash != -1)
   {
      unsigned int foundWhitespace = _pageContent.find_first_of(whitespacesAndDelimeters, foundSlash + 1);
      if((int)foundWhitespace != -1)
         fieldType = _pageContent.substr(foundSlash + 1, foundWhitespace - foundSlash - 1);
      else 
         break;
      //is this any page element between "/" and " "
      if(_allPageFields.count(fieldType))
      {         
         return foundSlash;
      }
      foundSlash = foundWhitespace;
   }
   return _pageContent.rfind(">>");
}

void PageElementHandler::_processObjectContent(unsigned int startOfPageElement)
{
    Q_UNUSED(startOfPageElement);
}
