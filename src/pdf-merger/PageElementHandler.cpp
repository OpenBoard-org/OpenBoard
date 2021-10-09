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

std::set<std::string> PageElementHandler::s_allPageFields;

void PageElementHandler::createAllPageFieldsSet()
{
   if(!s_allPageFields.empty())
      return;
   s_allPageFields.insert(std::string("Type"));
   s_allPageFields.insert(std::string("Parent"));
   s_allPageFields.insert(std::string("LastModified"));
   s_allPageFields.insert(std::string("Resources"));
   s_allPageFields.insert(std::string("MediaBox"));
   s_allPageFields.insert(std::string("CropBox"));
   s_allPageFields.insert(std::string("BleedBox"));
   s_allPageFields.insert(std::string("TrimBox"));
   s_allPageFields.insert(std::string("ArtBox"));
   s_allPageFields.insert(std::string("BoxColorInfo"));
   s_allPageFields.insert(std::string("Contents"));
   s_allPageFields.insert(std::string("Rotate"));
   s_allPageFields.insert(std::string("Group"));
   s_allPageFields.insert(std::string("Thumb"));
   s_allPageFields.insert(std::string("B"));
   s_allPageFields.insert(std::string("Dur"));
   s_allPageFields.insert(std::string("Trans"));
   s_allPageFields.insert(std::string("Annots"));
   s_allPageFields.insert(std::string("AA"));
   s_allPageFields.insert(std::string("Metadata"));
   s_allPageFields.insert(std::string("PieceInfo"));
   s_allPageFields.insert(std::string("StructParents"));
   s_allPageFields.insert(std::string("ID"));
   s_allPageFields.insert(std::string("PZ"));
   s_allPageFields.insert(std::string("SeparationInfo"));
   s_allPageFields.insert(std::string("Tabs"));
   s_allPageFields.insert(std::string("TemplateInstantiated"));
   s_allPageFields.insert(std::string("PresSteps"));
   s_allPageFields.insert(std::string("UserUnit"));
   s_allPageFields.insert(std::string("VP"));
   //for correct search all fields of XObject should be present to
   s_allPageFields.insert(std::string("Subtype"));
   s_allPageFields.insert(std::string("FormType"));
   s_allPageFields.insert(std::string("BBox"));
   s_allPageFields.insert(std::string("Matrix"));
   s_allPageFields.insert(std::string("Ref"));
   s_allPageFields.insert(std::string("StructParent"));
   s_allPageFields.insert(std::string("OPI"));
   s_allPageFields.insert(std::string("OC"));
   s_allPageFields.insert(std::string("Name"));

}

unsigned int PageElementHandler::findEndOfElementContent(unsigned int startOfPageElement)
{
   static std::string whitespacesAndDelimeters(" \t\f\v\n\r<<[/");
   unsigned int foundSlash = m_pageContent.find("/", startOfPageElement + 1);
   std::string fieldType;
   while((int)foundSlash != -1)
   {
      unsigned int foundWhitespace = m_pageContent.find_first_of(whitespacesAndDelimeters, foundSlash + 1);
      if((int)foundWhitespace != -1)
         fieldType = m_pageContent.substr(foundSlash + 1, foundWhitespace - foundSlash - 1);
      else 
         break;
      //is this any page element between "/" and " "
      if(s_allPageFields.count(fieldType))
      {         
         return foundSlash;
      }
      foundSlash = foundWhitespace;
   }
   return m_pageContent.rfind(">>");
}

void PageElementHandler::processObjectContentImpl(unsigned int startOfPageElement)
{
    Q_UNUSED(startOfPageElement);
}
