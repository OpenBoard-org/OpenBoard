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
void ContentHandler::processObjectContentImpl(unsigned int startOfPageElement)
{

   unsigned int endOfPage = findEndOfElementContent(startOfPageElement);
   m_concatenatedStream = retrieveStreamContent(m_page, startOfPageElement, endOfPage);

   FlateDecode flate;   
   flate.encode(m_concatenatedStream);
}

//write concatenated stream to Page object
void ContentHandler::changeObjectContentImpl(unsigned int startOfPageElement)
{

   unsigned int endOfPage = findEndOfElementContent(startOfPageElement);
   m_page->forgetAboutChildren(startOfPageElement, endOfPage);
   m_page->eraseContent(startOfPageElement, endOfPage - startOfPageElement);
   unsigned int endOfObjectDescription = m_pageContent.rfind(">>");
   const char * length = "/Filter /FlateDecode\n/Length ";
   unsigned int sizeOfLength = strlen(length);
   m_page->insertToContent(endOfObjectDescription, length, sizeOfLength);
   m_page->insertToContent(endOfObjectDescription + sizeOfLength, Utils::uIntToStr(m_concatenatedStream.size()).c_str());
   m_page->appendContent("\nstream\n");
   m_page->appendContent(m_concatenatedStream);
   m_page->appendContent("endstream\n");
   m_page->forgetStreamInFile();
}

//get content of stream 
// object - object with stream
//leftBound - left bound of object's content
//rightBound - right bound of object's content
string ContentHandler::retrieveStreamContent(merge_lib::Object * object, unsigned int leftBound, unsigned int rightBound)
{   
   return (object->hasStream()) ? 
      getStreamFromContent(object) :
      getStreamFromReferencies(object, leftBound, rightBound);

}
//get stream from Array elements
string ContentHandler::getStreamFromReferencies(merge_lib::Object * objectWithArray, unsigned int leftBound, unsigned int rightBound)
{
   std::string result;
   std::vector<Object *> referencies = objectWithArray->getSortedByPositionChildren(leftBound, rightBound);
   for(size_t i = 0; i < referencies.size(); ++i)
   {
      result.append(retrieveStreamContent(referencies[i], 0, referencies[i]->getObjectContent().size()));
   }
   objectWithArray->forgetAboutChildren(leftBound,rightBound);
   return result;
}

//get stream from Object
string ContentHandler::getStreamFromContent(merge_lib::Object * objectWithStream)
{
   Filter filter(objectWithStream);    
   string decodedStream;
   filter.getDecodedStream(decodedStream);

   return decodedStream;
}
