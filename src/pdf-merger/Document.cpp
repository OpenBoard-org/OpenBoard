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




#include "Config.h"
#include "Document.h"
#include "FlateDecode.h"
#include "Utils.h"
#include "Parser.h"
#include "Exception.h"
#include <fstream>
#include <iostream>
#include <iomanip>

#include "core/memcheck.h"

using namespace merge_lib;
const std::string firstObj("%PDF-1.4\n1 0 obj\n<<\n/Title ()/Creator ()/Producer (Qt 4.5.0 (C) 1992-2009 Nokia Corporation and/or its subsidiary(-ies))/CreationDate (D:20090424120829)\n>>\nendobj\n");
const std::string zeroStr("0000000000");
Document::Document(const char * fileName):
    _root(0), _pages(), _documentName(fileName), _maxObjectNumber(0)
{

}

Document::~Document()
{
   for(size_t i = 0; i < _allObjects.size(); ++i)
      delete _allObjects[i];         
   _allObjects.clear();

   std::map<unsigned int, Page *>::const_iterator it(_pages.begin());
   for(;it != _pages.end();it++)
   {
      delete (*it).second;
   }
   _pages.clear();
}


Page * Document::getPage(unsigned int pageNumber)
{
   if(!_pages.count(pageNumber))
   {
      return 0;
/*      std::stringstream error;
      error << "There is no page with " 
         << pageNumber << " number in " 
         << _documentName;
      throw Exception(error);*/
   }
   return  _pages[pageNumber];
}

void Document::saveAs(const char * newFileName)
{
   //first two objects will be created by hand
   unsigned int fromObjNumber = 2;
   _root->recalculateObjectNumbers(fromObjNumber);
   _root->retrieveMaxObjectNumber(_maxObjectNumber);
   
   //sizes of all objects
   //key - object number
   //value - size of object
   std :: map < unsigned int, std::pair<unsigned long long, unsigned int > > sizesAndGenerationNumbers;
   std::ofstream out;
   out.open(newFileName, std::ios::binary);
   if(!out.is_open())
   {      
      std::string error("Access denied on file ");
      error.append(newFileName);
      throw Exception(error);
   }

   out << firstObj.c_str();
   _root->serialize( out, sizesAndGenerationNumbers);
   
   std::map< unsigned int, std::pair<unsigned long long, unsigned int > >::iterator sizeIterator;

   unsigned int numberOfObjects = 2;
   for ( sizeIterator = sizesAndGenerationNumbers.begin() ; sizeIterator != sizesAndGenerationNumbers.end(); sizeIterator++ )
   {
      numberOfObjects++;
   }
   
   //create xref
   out << "xref\n"
      << "0 " << numberOfObjects  << "\n"
      << "0000000000 65535 f \n"
      << "0000000009 00000 n \n";


   unsigned long long sizeInXref = firstObj.size();

   for ( sizeIterator = sizesAndGenerationNumbers.begin() ; sizeIterator != sizesAndGenerationNumbers.end(); sizeIterator++ )
   {
      out << std::setfill('0')<<std::setw(10)<<sizeInXref << " " << std::setw(5) << (*sizeIterator).second.second << " n \n";
      sizeInXref = sizeInXref + (*sizeIterator).second.first;
   }
   out << "trailer\n<<\n/Size " << numberOfObjects  << "\n/Info 1 0 R\n"
      << "/Root " << _root->getObjectNumber() << " 0 R\n >>\nstartxref\n" << sizeInXref << "\n%%EOF";

}

Object * Document::getDocumentObject()
{
   return _root;
}


