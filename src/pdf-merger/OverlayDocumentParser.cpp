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
   m_fileName = fileName;
   return Parser::parseDocument(fileName);
}


void OverlayDocumentParser::readXRefAndCreateObjects()
{
   std::map<unsigned int, unsigned long> objectsAndPositions;
   readXref(objectsAndPositions);
   std::map<unsigned int, unsigned long> objectsAndSizes;
   std::map<unsigned int, unsigned long>::iterator objAndSIter;
   std::map<unsigned int, unsigned long>::iterator objAndPIter;
   unsigned long fileSize = Utils::getFileSize(m_fileName.c_str());

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
         getPartOfFileContent(partStart, partSize);
      else
      {
         getPartOfFileContent(partStart, fileSize - partStart);
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
            const std::string content = getObjectContent(objIter->second - partStart, objectNumber, generationNumber, streamBounds, hasObjectStream);
            streamBounds.first += partStart;
            streamBounds.second += partStart;
            Object * newObject = new Object(objectNumber, generationNumber, content, m_document->m_documentName ,streamBounds, hasObjectStream);
            m_objects[objectNumber] = newObject;
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

void OverlayDocumentParser::getFileContent(const char * fileName)
{
    Q_UNUSED(fileName);
}

void OverlayDocumentParser::getPartOfFileContent(long startOfPart, unsigned int length)
{
   ifstream pdfFile;
   pdfFile.open (m_fileName.c_str(), ios::binary );
   if (pdfFile.fail())
   {
      stringstream errorMessage("File ");
      errorMessage << m_fileName << " is absent" << "\0";
      throw Exception(errorMessage);
   }   
   ios_base::seekdir dir;
   if(startOfPart >= 0)
      dir = ios_base::beg;
   else 
      dir = ios_base::end;
   pdfFile.seekg (startOfPart, dir);
   m_fileContent.resize(length);
   pdfFile.read(&m_fileContent[0], length);
   pdfFile.close();
}

void OverlayDocumentParser::readXref(std::map<unsigned int, unsigned long> & objectsAndSizes)
{
   getPartOfFileContent(- DOC_PART_WITH_START_OF_XREF, DOC_PART_WITH_START_OF_XREF);
   unsigned int startOfStartxref = m_fileContent.find("startxref");
   unsigned int startOfNumber = m_fileContent.find_first_of(Parser::NUMBERS, startOfStartxref);
   unsigned int endOfNumber = m_fileContent.find_first_not_of(Parser::NUMBERS, startOfNumber + 1);
   std::string startXref = m_fileContent.substr(startOfNumber, endOfNumber - startOfNumber);
   unsigned int strtXref = Utils::stringToInt(startXref);

   unsigned int sizeOfXref = Utils::getFileSize(m_fileName.c_str()) - strtXref;
   getPartOfFileContent(strtXref, sizeOfXref);
   unsigned int leftBoundOfObjectNumber = m_fileContent.find("0 ") + strlen("0 ");
   unsigned int rightBoundOfObjectNumber = m_fileContent.find_first_not_of(Parser::NUMBERS, leftBoundOfObjectNumber);
   std::string objectNuberStr = m_fileContent.substr(leftBoundOfObjectNumber, rightBoundOfObjectNumber - leftBoundOfObjectNumber);
   unsigned long objectNumber = Utils::stringToInt(objectNuberStr);
   unsigned int startOfObjectPosition = m_fileContent.find("0000000000 65535 f ") + strlen("0000000000 65535 f ");
   for(unsigned long i = 1; i < objectNumber; ++i)
   {
      startOfObjectPosition = m_fileContent.find_first_of(Parser::NUMBERS, startOfObjectPosition);
      unsigned int endOfObjectPostion = m_fileContent.find(" 00000 n", startOfObjectPosition);
      std::string objectPostionStr = m_fileContent.substr(startOfObjectPosition, endOfObjectPostion - startOfObjectPosition);
      objectsAndSizes[i] = Utils::stringToInt(objectPostionStr);
      startOfObjectPosition = endOfObjectPostion + strlen(" 00000 n");
   }
}

unsigned int OverlayDocumentParser::readTrailerAndReturnRoot()
{
   getPartOfFileContent(- (3*DOC_PART_WITH_START_OF_XREF), (3*DOC_PART_WITH_START_OF_XREF));
   return Parser::readTrailerAndReturnRoot();
}

unsigned int OverlayDocumentParser::getStartOfXrefWithRoot()
{
   return 0;
}

