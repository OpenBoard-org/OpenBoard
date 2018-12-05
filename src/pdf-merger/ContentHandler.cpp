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




#include "ContentHandler.h"
#include "Filter.h"
#include "FlateDecode.h"

#include <iostream>

#include <string>
#include <string.h>

#include "core/memcheck.h"

using namespace merge_lib;
using namespace std;
//concatenate stream of all objects which contain Content of Page
void ContentHandler::_processObjectContent(unsigned int startOfPageElement)
{

   unsigned int endOfPage = _findEndOfElementContent(startOfPageElement);
   _concatenatedStream = _retrieveStreamContent(_page, startOfPageElement, endOfPage);

   FlateDecode flate;   
   flate.encode(_concatenatedStream);
}

//write concatenated stream to Page object
void ContentHandler::_changeObjectContent(unsigned int startOfPageElement)
{

   unsigned int endOfPage = _findEndOfElementContent(startOfPageElement);
   _page->forgetAboutChildren(startOfPageElement, endOfPage);
   _page->eraseContent(startOfPageElement, endOfPage - startOfPageElement);
   unsigned int endOfObjectDescription = _pageContent.rfind(">>");
   const char * length = "/Filter /FlateDecode\n/Length ";
   unsigned int sizeOfLength = strlen(length);
   _page->insertToContent(endOfObjectDescription, length, sizeOfLength);
   _page->insertToContent(endOfObjectDescription + sizeOfLength, Utils::uIntToStr(_concatenatedStream.size()).c_str());
   _page->appendContent("\nstream\n");
   _page->appendContent(_concatenatedStream);
   _page->appendContent("endstream\n");
   _page->forgetStreamInFile();
}

//get content of stream 
// object - object with stream
//leftBound - left bound of object's content
//rightBound - right bound of object's content
string ContentHandler::_retrieveStreamContent(merge_lib::Object * object, unsigned int leftBound, unsigned int rightBound)
{   
   return (object->hasStream()) ? 
      _getStreamFromContent(object) :
      _getStreamFromReferencies(object, leftBound, rightBound);

}
//get stream from Array elements
string ContentHandler::_getStreamFromReferencies(merge_lib::Object * objectWithArray, unsigned int leftBound, unsigned int rightBound)
{
   std::string result;
   std::vector<Object *> referencies = objectWithArray->getSortedByPositionChildren(leftBound, rightBound);
   for(size_t i = 0; i < referencies.size(); ++i)
   {
      result.append(_retrieveStreamContent(referencies[i], 0, referencies[i]->getObjectContent().size()));      
   }
   objectWithArray->forgetAboutChildren(leftBound,rightBound);
   return result;
}

//get stream from Object
string ContentHandler::_getStreamFromContent(merge_lib::Object * objectWithStream)
{
   Filter filter(objectWithStream);    
   string decodedStream;
   filter.getDecodedStream(decodedStream);

   return decodedStream;
}
