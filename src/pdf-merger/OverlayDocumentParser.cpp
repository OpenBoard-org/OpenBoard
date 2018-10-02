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




#include "OverlayDocumentParser.h"
#include <fstream>
#include <string.h>
#include <QtGlobal>
#include "Exception.h"
#include "Object.h"

#include "core/memcheck.h"

using namespace merge_lib;
using namespace std;

int OverlayDocumentParser::DOC_PART_WITH_START_OF_XREF = 30;
unsigned int partSize = 10485760; // = 10 Mb

Document * OverlayDocumentParser::parseDocument(const char * fileName)
{
   _fileName = fileName;
   return Parser::parseDocument(fileName);
}


void OverlayDocumentParser::_readXRefAndCreateObjects()
{
   std::map<unsigned int, unsigned long> objectsAndPositions;
   _readXref(objectsAndPositions);
   std::map<unsigned int, unsigned long> objectsAndSizes;
   std::map<unsigned int, unsigned long>::iterator objAndSIter;
   std::map<unsigned int, unsigned long>::iterator objAndPIter;
   unsigned long fileSize = Utils::getFileSize(_fileName.c_str());

   for(objAndSIter = objectsAndPositions.begin(); objAndSIter != objectsAndPositions.end(); ++objAndSIter)
   {
      unsigned int nextPosition = fileSize;
      for(objAndPIter = objectsAndPositions.begin(); objAndPIter != objectsAndPositions.end(); ++objAndPIter)
      {
         if((objAndPIter->second > objAndSIter->second) && (objAndPIter->second < nextPosition))
            nextPosition = objAndPIter->second;            
      }
      objectsAndSizes[objAndSIter->first] = nextPosition - objAndSIter->second;
   }

   bool notEndOfFile = true;
   do
   {
      unsigned long partStart = fileSize;
      std::map<unsigned int, unsigned long>::iterator objIter;
      for(objIter = objectsAndPositions.begin(); objIter != objectsAndPositions.end(); ++objIter)
      {
         if(objIter->second < partStart)
            partStart = objIter->second;
      }
      unsigned long nextPartStart = partStart + partSize;                 

      if((nextPartStart) < fileSize)
         _getPartOfFileContent(partStart, partSize);
      else
      {
         _getPartOfFileContent(partStart, fileSize - partStart);
         nextPartStart = fileSize;
         notEndOfFile = false;
      }

      for(objIter = objectsAndPositions.begin(); objIter != objectsAndPositions.end(); )
      {
         if((objectsAndSizes[objIter->first] +  objIter->second <= nextPartStart) && (objIter->second >= partStart) && ((objIter->second < nextPartStart)))
         {
            std::pair<unsigned int, unsigned int> streamBounds;
            unsigned int objectNumber;
            unsigned int generationNumber;
            bool hasObjectStream;
            const std::string content = _getObjectContent(objIter->second - partStart, objectNumber, generationNumber, streamBounds, hasObjectStream);
            streamBounds.first += partStart;
            streamBounds.second += partStart;
            Object * newObject = new Object(objectNumber, generationNumber, content, _document->_documentName ,streamBounds, hasObjectStream);
            _objects[objectNumber] = newObject;
            std::map<unsigned int, unsigned long>::iterator temp = objIter;                   
            ++objIter;
            objectsAndPositions.erase(temp);
            continue;
         }                      
         ++objIter;           
      }
      partStart = nextPartStart;
   }
   while(notEndOfFile);   
}

void OverlayDocumentParser::_getFileContent(const char * fileName)
{
    Q_UNUSED(fileName);
}

void OverlayDocumentParser::_getPartOfFileContent(long startOfPart, unsigned int length)
{
   ifstream pdfFile;
   pdfFile.open (_fileName.c_str(), ios::binary );
   if (pdfFile.fail())
   {
      stringstream errorMessage("File ");
      errorMessage << _fileName << " is absent" << "\0";
      throw Exception(errorMessage);
   }   
   ios_base::seekdir dir;
   if(startOfPart >= 0)
      dir = ios_base::beg;
   else 
      dir = ios_base::end;
   pdfFile.seekg (startOfPart, dir);
   _fileContent.resize(length);
   pdfFile.read(&_fileContent[0], length);
   pdfFile.close();
}

void OverlayDocumentParser::_readXref(std::map<unsigned int, unsigned long> & objectsAndSizes)
{
   _getPartOfFileContent(- DOC_PART_WITH_START_OF_XREF, DOC_PART_WITH_START_OF_XREF);
   unsigned int startOfStartxref = _fileContent.find("startxref");
   unsigned int startOfNumber = _fileContent.find_first_of(Parser::NUMBERS, startOfStartxref);
   unsigned int endOfNumber = _fileContent.find_first_not_of(Parser::NUMBERS, startOfNumber + 1);
   std::string startXref = _fileContent.substr(startOfNumber, endOfNumber - startOfNumber);
   unsigned int strtXref = Utils::stringToInt(startXref);

   unsigned int sizeOfXref = Utils::getFileSize(_fileName.c_str()) - strtXref;
   _getPartOfFileContent(strtXref, sizeOfXref);
   unsigned int leftBoundOfObjectNumber = _fileContent.find("0 ") + strlen("0 ");
   unsigned int rightBoundOfObjectNumber = _fileContent.find_first_not_of(Parser::NUMBERS, leftBoundOfObjectNumber);
   std::string objectNuberStr = _fileContent.substr(leftBoundOfObjectNumber, rightBoundOfObjectNumber - leftBoundOfObjectNumber);
   unsigned long objectNumber = Utils::stringToInt(objectNuberStr);
   unsigned int startOfObjectPosition = _fileContent.find("0000000000 65535 f ") + strlen("0000000000 65535 f ");
   for(unsigned long i = 1; i < objectNumber; ++i)
   {
      startOfObjectPosition = _fileContent.find_first_of(Parser::NUMBERS, startOfObjectPosition);
      unsigned int endOfObjectPostion = _fileContent.find(" 00000 n", startOfObjectPosition);
      std::string objectPostionStr = _fileContent.substr(startOfObjectPosition, endOfObjectPostion - startOfObjectPosition);
      objectsAndSizes[i] = Utils::stringToInt(objectPostionStr);
      startOfObjectPosition = endOfObjectPostion + strlen(" 00000 n");
   }
}

unsigned int OverlayDocumentParser::_readTrailerAndReturnRoot()
{
   _getPartOfFileContent(- (3*DOC_PART_WITH_START_OF_XREF), (3*DOC_PART_WITH_START_OF_XREF));
   return Parser::_readTrailerAndReturnRoot();
}

unsigned int OverlayDocumentParser::_getStartOfXrefWithRoot()
{
   return 0;
}

