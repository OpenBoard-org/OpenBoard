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




#include <QtGlobal>
#include <fstream>
#include <iostream>
#include <vector>
#include <map>
#include <stack>
#include <string.h>
#include "Parser.h"
#include "Object.h"
#include "Exception.h"
#include "Utils.h"

#include "core/memcheck.h"

using namespace merge_lib;
using namespace std;

const std::string Parser::WHITESPACES(" \t\f\v\n\r");
const std::string Parser::DELIMETERS("()<>{}/%][");
const std::string Parser::NUMBERS("0123456789");
const std::string Parser::WHITESPACES_AND_DELIMETERS = Parser::WHITESPACES + Parser::DELIMETERS;

Document * Parser::parseDocument(const char * fileName)
{
   _document = new Document(fileName);
   try
   {
      _createObjectTree(fileName);
      _createDocument(fileName);
   }
   catch( std::exception &)
   {
      _clearParser();
      delete _document;
      _document = NULL;
      throw;
   }
   return _document;
}

void Parser::_retrieveAllPages(Object * objectWithKids)
{
   std::string & objectContent = objectWithKids->getObjectContent();
   unsigned int startOfKids = objectContent.find("/Kids");
   unsigned int endOfKids = objectContent.find("]", startOfKids);
   if(
      ((int)startOfKids == -1) &&
      ((int)objectContent.find("/Page") != -1)
      )
   {
      unsigned int numberOfPages = _document->_pages.size() + 1;
      Page * newPage = new Page(numberOfPages);     
      newPage->_root = objectWithKids;
      _document->_pages.insert(std::pair<unsigned int, Page *>(numberOfPages, newPage));
      return;
   }

   const std::vector<Object *> & kids = objectWithKids->getSortedByPositionChildren(startOfKids, endOfKids);
   for(size_t i(0); i < kids.size(); ++i)
   {
      _retrieveAllPages(kids[i]);
   }
}

void Parser::_createDocument(const char * docName)
{
    Q_UNUSED(docName);
   _document->_root = _root;
   std::string & rootContent = _root->getObjectContent();
   unsigned int startOfPages = rootContent.find("/Pages");
   if((int)startOfPages == -1)
      throw Exception("Some document is wrong");
   unsigned int endOfPages = rootContent.find("R", startOfPages);
   std::vector<Object *> objectWithKids = _root->getChildrenByBounds(startOfPages, endOfPages);
   if(objectWithKids.size() != 1)
      throw Exception("Some document is wrong");
   _retrieveAllPages(objectWithKids[0]);

   _root->retrieveMaxObjectNumber(_document->_maxObjectNumber);
   _clearParser();
}

void Parser::_clearParser()
{
   _root = 0;
   _fileContent.clear();
   _fileContent.reserve();
   _objects.clear();
}


void Parser::_getFileContent(const char * fileName)
{
   ifstream pdfFile;
   pdfFile.open (fileName, ios::binary );
   if (pdfFile.fail())
   {
      stringstream errorMessage("File ");
      errorMessage << fileName << " is absent" << "\0";
      throw Exception(errorMessage);
   }
   // get length of file:
   pdfFile.seekg (0, ios::end);
   int length = pdfFile.tellg();
   pdfFile.seekg (0, ios::beg);
   _fileContent.resize(length);
   pdfFile.read(&_fileContent[0], length);

   // check version
   const char *header = "%PDF-1.";
   size_t verPos = _fileContent.find(header);
   if( verPos == 0 )
   {
      verPos += strlen(header);
      char ver = _fileContent[verPos];
      if( ver < '0' || ver > '4' )
      {
         stringstream errorMsg;
         errorMsg<<" File with verion 1."<<ver<<" is not currently supported by merge library\n";
         throw Exception(errorMsg);
      }
   }
   else
   {
      throw Exception("Unrecognized header of PDF file");
   }
   pdfFile.close();
}


void Parser::_createObjectTree(const char * fileName)
{
   unsigned int rootObjectNumber = 0;
   try
   {
      _getFileContent(fileName);
      _readXRefAndCreateObjects();
      rootObjectNumber = _readTrailerAndReturnRoot();
   }
   catch (std::exception &)
   {
      std::map<unsigned int, Object *>::const_iterator it(_objects.begin());
      for(;it != _objects.end();it++)
      {
         delete (*it).second;
      }
      _objects.clear();
      throw;
   }

   std::map<unsigned int, Object *>::iterator objectsIterator;

   for ( objectsIterator = _objects.begin() ; objectsIterator != _objects.end(); objectsIterator++ )
   {
      Object * currentObject = (*objectsIterator).second;
      _document->_allObjects.push_back(currentObject);
      //key - object number :  value - positions in object content of this reference
      const std::map<unsigned int, Object::ReferencePositionsInContent> & refs = 
         _getReferences(currentObject->getObjectContent());      
      std::map<unsigned int, Object::ReferencePositionsInContent>::const_iterator refsIterator = refs.begin();
      for(; refsIterator !=  refs.end(); ++refsIterator)
      {        
         if(_objects.count((*refsIterator).first))
            currentObject->addChild(_objects[(*refsIterator).first], (*refsIterator).second);        
      }
   }   
   _root = _objects[rootObjectNumber];

}

const std::map<unsigned int, Object::ReferencePositionsInContent> & Parser::_getReferences(const std::string & objectContent)
{
   unsigned int currentPosition(0), startOfNextSearch(0);
   static std::map<unsigned int, std::vector<unsigned int> >  searchResult;
   searchResult.clear();
   unsigned int streamStart = objectContent.find("stream");
   if((int)streamStart == -1)
      streamStart = objectContent.size();
   while(startOfNextSearch < streamStart)
   {
      //try to find reference. reference example is 15 0 R
      startOfNextSearch = objectContent.find(" R", startOfNextSearch);
      currentPosition = startOfNextSearch;
      if((int)currentPosition != -1)
      {         
         //check that next character of " R" is WHITESPACE. 

         if(((int)WHITESPACES.find(objectContent[currentPosition + 2]) == -1) &&
            ((int)DELIMETERS.find(objectContent[currentPosition + 2]) == -1)
            )
         {
            //this is not reference. this is something looks like "0 0 0 RG"
            ++startOfNextSearch;
            continue;
         }
         //get previos symbol and check that it is a number
         unsigned int numberSearchCounter = _skipNumber(objectContent, --currentPosition);

         //previos symbol is not a number
         if(numberSearchCounter == currentPosition)
         {
            ++startOfNextSearch;
            continue;
         }
         else
         {
            currentPosition = numberSearchCounter;
         }

         bool isFound(false);
         //previos symbols should be whitespaces
         while((objectContent[currentPosition] == ' ') && --currentPosition) 
         {
            isFound = true;
         }

         //previos symbol is not a whitespace
         if(!isFound)
         {
            ++startOfNextSearch;
            continue;
         }
         //check that this and may be previos symbols are a numbers     
         numberSearchCounter = _skipNumber(objectContent, currentPosition);
         if(numberSearchCounter == currentPosition)
         {
            ++startOfNextSearch;
            continue;
         }
         unsigned int objectNumber = Utils::stringToInt(objectContent.substr(numberSearchCounter + 1, currentPosition - numberSearchCounter));

         searchResult[objectNumber].push_back(numberSearchCounter + 1);


         ++startOfNextSearch;

      }
      else
         break;      
   }
   return searchResult;
}

unsigned int Parser::_skipNumber(const std::string & str, unsigned int currentPosition)
{
   unsigned int numberSearchCounter = currentPosition;    
   while(((int)NUMBERS.find(str[numberSearchCounter]) != -1) && --numberSearchCounter)
   {}

   return numberSearchCounter;
}
void Parser::_readXRefAndCreateObjects()
{      
   unsigned int currentPostion = _getStartOfXrefWithRoot();
   do
   {
      const std::string & currentToken = _getNextToken(currentPostion);
      if(currentToken != "xref")
      {
         throw Exception("Wrong xref in some document");
      }
      unsigned int endOfLine = _getEndOfLineFromContent(currentPostion );
      if(_countTokens(currentPostion, endOfLine) != 2)
      {
         throw Exception("Wrong xref in some document");

      }
      //now we are reading the xref
      while(1)
      {
         Utils::stringToInt(_getNextToken(currentPostion));
         unsigned int objectCount = Utils::stringToInt(_getNextToken(currentPostion));
         for(unsigned int i(0); i < objectCount; i++)
         {
            unsigned long  first;

            if(_countTokens(currentPostion, _getEndOfLineFromContent(currentPostion)) == 3)
            {
               first  = Utils::stringToInt(_getNextToken(currentPostion));
               Utils::stringToInt(_getNextToken(currentPostion));
               const string & use         = _getNextToken(currentPostion);
               if(!use.compare("n"))
               {
                  unsigned int objectNumber;

                  try               
                  {
                     std::pair<unsigned int, unsigned int> streamBounds;
                     bool hasObjectStream;
                     unsigned int generationNumber;
                     const std::string content = _getObjectContent(first, objectNumber, generationNumber, streamBounds, hasObjectStream);
                     if(!_objects.count(objectNumber))
                     {
                        Object * newObject = new Object(objectNumber, generationNumber, content, _document->_documentName ,streamBounds, hasObjectStream);
                        _objects[objectNumber] = newObject;
                     }
                  }
                  catch(std::exception &)
                  {
                  }

               }
            }
            else
            {
               ;
            }
            ++currentPostion;


         }
         unsigned int previosPostion = currentPostion;
         const std::string & isTrailer = _getNextToken(currentPostion);

         std::string trailer("trailer");
         if(isTrailer == trailer)
         {
            currentPostion -= trailer.size();
            break;
         }
         else
            currentPostion = previosPostion;

      }
   }
   while(_readTrailerAndRterievePrev(currentPostion, currentPostion));


}

unsigned int Parser::_getStartOfXrefWithRoot()
{
   unsigned int leftBoundOfStartOfXref = _fileContent.rfind("startxref");
   leftBoundOfStartOfXref = _fileContent.find_first_of(NUMBERS, leftBoundOfStartOfXref);

   unsigned int rightBoundOfStartOfXref = _fileContent.find_first_not_of(NUMBERS, leftBoundOfStartOfXref + 1);

   std::string  startOfXref = _fileContent.substr(leftBoundOfStartOfXref, rightBoundOfStartOfXref - leftBoundOfStartOfXref);
   int integerStartOfXref = Utils::stringToInt(startOfXref);
   return integerStartOfXref;
}

unsigned int Parser::_getEndOfLineFromContent(unsigned int fromPosition)
{
   fromPosition = _skipWhiteSpacesFromContent(fromPosition);
   unsigned int endOfLine = _fileContent.find_first_of("\n\r", fromPosition);
   endOfLine = _fileContent.find_last_of("\n\r", endOfLine);
   return endOfLine;

}

const std::pair<unsigned int, unsigned int> & Parser::_getLineBounds(const std::string & str, unsigned int fromPosition)
{
   static std::pair<unsigned int, unsigned int> bounds;
   bounds.first = str.rfind('\n', fromPosition);
   if((int)bounds.first == -1)
      bounds.first = 0;
   bounds.second = str.find('\n', fromPosition);
   if((int)bounds.second == -1)
      bounds.second = str.size();    
   return bounds;
}

const std::string & Parser::_getNextToken(unsigned int & fromPosition)
{
   fromPosition = _skipWhiteSpacesFromContent(fromPosition);
   unsigned int position = _fileContent.find_first_of(WHITESPACES, fromPosition);

   static std::string token;
   if(position > fromPosition)
   {        
      unsigned int tokenSize = position - fromPosition;
      token.resize(tokenSize);
      memcpy(&token[0], &_fileContent[fromPosition], tokenSize);
      fromPosition = position;
      return token;
   }
   else
   {
      //TODO throw exception
   }
   token = "";
   return token;
}

unsigned int Parser::_countTokens(unsigned int leftBound, unsigned int rightBount)
{
   unsigned int position = _skipWhiteSpacesFromContent(leftBound);
   unsigned int tokensCount = 0;

   while (position < rightBount)
   {
      position = _fileContent.find_first_of(WHITESPACES, position);
      if ((int)position != -1)
         ++tokensCount;
      //start search from next symbol
      ++position;
   }
   return tokensCount;
}

unsigned int Parser::_skipWhiteSpaces(const std::string & str, unsigned int fromPosition)
{
   unsigned int position = fromPosition;
   if((int)WHITESPACES.find(str[0]) != -1)
      position = str.find_first_not_of(WHITESPACES, position);
   return position;
}

unsigned int Parser::_skipWhiteSpacesFromContent(unsigned int fromPosition)
{
   unsigned int position = fromPosition;
   if((int)WHITESPACES.find(_fileContent[position]) != -1)
      position = _fileContent.find_first_not_of(WHITESPACES, position);// + 1;

   return position;
}

const std::string & Parser::_getObjectContent(unsigned int objectPosition, unsigned int & objectNumber, unsigned int & generationNumber, std::pair<unsigned int, unsigned int> & streamBounds, bool & hasObjectStream)
{
   hasObjectStream = false;
   unsigned int currentPosition = objectPosition;

   std::string token = _getNextToken(currentPosition);  // number of object
   objectNumber = Utils::stringToInt(token);

   token = _getNextToken(currentPosition);  // generation number - not interesting
   generationNumber = Utils::stringToInt(token);

   token = Parser::getNextToken(_fileContent,currentPosition);

   if( token != "obj" )
   {
      std::stringstream strOut;
      strOut<<"Wrong object in PDF, in position "<<currentPosition<<" cannot continue!\n";
      throw Exception(strOut.str());
   }

   static std::string objectContent;

   size_t contentStart = _fileContent.find_first_not_of(Parser::WHITESPACES,currentPosition);
   if((int) contentStart == -1 )
   {
      std::stringstream strOut;
      strOut<<"Wrong object "<< objectNumber<< "in PDF, cannot find content for it\n";
      throw Exception(strOut.str());
   }
   currentPosition = contentStart;
   unsigned int endOfContent = _fileContent.find("endobj", contentStart);
   if((int) endOfContent == -1 )
   {
      stringstream errorMessage("Corrupted PDF file, obj does not have matching endobj");
      throw Exception(errorMessage);
   }
   unsigned int endOfStream = _fileContent.find("endstream", currentPosition);
   if(((int)endOfStream != -1) && (endOfStream < endOfContent))
   {
      std::string stream("stream");
      unsigned int beginOfStream = _fileContent.find(stream, currentPosition) + stream.size();
      while(_fileContent[beginOfStream] == '\r')
      {
         ++beginOfStream;
      }
      if( _fileContent[beginOfStream] == '\n')
      {
         ++beginOfStream;
      }
      streamBounds.first = beginOfStream;

      // try to use Length field to determine end of stream.
      std::string lengthToken = "/Length";
      size_t lengthBegin = Parser::findTokenName(_fileContent,lengthToken,contentStart);
      if ((int) lengthBegin != -1 )
      {
         std::string lengthStr;
         size_t lenPos = lengthBegin + lengthToken.size();
         bool useContentLength = false;
         if( Parser::getNextWord(lengthStr,_fileContent,lenPos) )
         {
            useContentLength = true;
            std::string refStr;
            if( Parser::getNextWord(refStr,_fileContent,lenPos))
            {
               if( Parser::getNextWord(refStr,_fileContent,lenPos))
               {
                  if( refStr == "R" )
                  {
                     useContentLength = false;
                     //it is reference
                  }
               }
            }
         }
         if( useContentLength )
         {
            std::stringstream strin(lengthStr);
            unsigned int streamEnd = 0;
            strin>>streamEnd;
            streamEnd += beginOfStream;
            unsigned int streamEndBegin = _fileContent.find("endstream",streamEnd);
            if((int) streamEndBegin != -1 )
            {
               endOfStream = streamEndBegin;
            }
         }
      }
      streamBounds.second = endOfStream;
      endOfContent = beginOfStream;
      hasObjectStream = true;

   }
   unsigned int contentSize = endOfContent - currentPosition;

   objectContent.resize(contentSize);
   memcpy(&objectContent[0], &_fileContent[currentPosition], contentSize);
   return objectContent;

}

unsigned int Parser::_readTrailerAndReturnRoot()
{

   unsigned int startOfTrailer = Parser::findToken(_fileContent,"trailer", _getStartOfXrefWithRoot());
   std::string rootStr("/Root");
   unsigned int startOfRoot = Parser::findToken(_fileContent,rootStr.data(), startOfTrailer);
   if((int) startOfRoot == -1)
   {
      throw Exception("Cannot find Root object !");
   }
   std::string encryptStr("/Encrypt");
   if((int) Parser::findToken(_fileContent,encryptStr,startOfTrailer) != -1 )
   {
      throw Exception("Encrypted PDF is not supported!");
   }
   startOfRoot += rootStr.size()+1; //"/Root + ' ' 
   unsigned int endOfRoot = startOfRoot;
   while((int)NUMBERS.find(_fileContent[endOfRoot++]) != -1)
   {}
   --endOfRoot;
   return Utils::stringToInt(_fileContent.substr(startOfRoot, endOfRoot - startOfRoot));   
}

unsigned int Parser::_readTrailerAndRterievePrev(const unsigned int startPositionForSearch, unsigned int & previosXref)
{
   unsigned int startOfTrailer = Parser::findToken(_fileContent,"trailer", startPositionForSearch);
   if((int) startOfTrailer == -1 )
   {
      throw Exception("Cannot find trailer!");
   }

   unsigned int startOfPrev = _fileContent.find("Prev ", startOfTrailer);
   unsigned int startxref = _fileContent.find("startxref", startOfTrailer);
   if((int)startOfPrev == -1 || (startOfPrev > startxref))
      return false;
   //"Prev "s length = 5
   else
      startOfPrev += 5;

   unsigned int endOfPrev = startOfPrev;
   while((int)NUMBERS.find(_fileContent[endOfPrev++]) != -1)
   {}
   --endOfPrev;
   previosXref = Utils::stringToInt(_fileContent.substr(startOfPrev, endOfPrev - startOfPrev));   
   return true;
}

//Method finds the token from current position from string
// It uses PDF whitespaces and delimeters to recognize
// Returned string without begin/end spaces
std::string Parser::getNextToken(const std::string &str, unsigned int  &position)
{
   if( position >= str.size() )
   {
      return "";
   }
   //skip first spaces
   size_t beg_pos = str.find_first_not_of(Parser::WHITESPACES,position);
   if ((int) beg_pos == -1 )
   {   
      // it is empty string!
      return "";
   }
   size_t end_pos = str.find_first_of(Parser::WHITESPACES_AND_DELIMETERS,beg_pos);
   if ((int) end_pos == -1 )
   {
      end_pos = str.size();
   }
   position = end_pos;

   std::string out = str.substr(beg_pos,end_pos - beg_pos);
   Parser::trim(out);
   return out;
}
/** @brief getNextWord
*
* method finds and returns next word from the string
* For example: " 1 0 R \n" will return "1" , then "0" then "R"
*/
bool Parser::getNextWord(std::string &out, const std::string &str, size_t &nextPosition, size_t  *found)
{
   if( found )
   {
      *found = -1;
   }
   //trace("position = %d",position);
   if( nextPosition >= str.size() )
   {
      return false;
   }
   //skip first spaces
   size_t beg_pos = str.find_first_not_of(Parser::WHITESPACES,nextPosition);
   if ((int) beg_pos == -1 )
   {   
      // it is empty string!
      return false;
   }
   if( found )
   {
      *found = beg_pos;
   }
   size_t end_pos = str.find_first_of(Parser::WHITESPACES,beg_pos);

   if ((int) end_pos == -1 )
   {
      end_pos = str.size();
   }
   nextPosition = end_pos;
   out = str.substr(beg_pos,end_pos - beg_pos);
   Parser::trim(out);
   if( out.empty() )
   {
      return false;
   }
   return true;
}

/** @brief trim
*
* @todo: document this function
*/
void Parser::trim(std::string &str)
{
   std::string::size_type pos1 = str.find_first_not_of(WHITESPACES);
   std::string::size_type pos2 = str.find_last_not_of(WHITESPACES);
   str = str.substr((int)pos1 == -1 ? 0 : pos1,
      (int)pos2 == -1 ? str.length() - 1 : pos2 - pos1 + 1);
}

// Method tries to find the PDF token from the content 
// The token is "/L 12 0R" or /Length 123
std::string Parser::findTokenStr(const std::string &content, const std::string &pattern, size_t start, size_t &foundStart, size_t &foundEnd)
{
   size_t cur_pos  = Parser::findToken(content,pattern,start);
   if((int) cur_pos == -1 )
   {
      return "";
   }
   foundStart = cur_pos;
   cur_pos += pattern.size();
   // then lets parse the content of remaining part
   size_t end_pos = content.find_first_of(Parser::DELIMETERS,cur_pos);
   if((int) end_pos == -1 )
   {
      end_pos = content.size();
   }
   std::string token = content.substr(cur_pos,end_pos-cur_pos);
   foundEnd = end_pos -1;
   return token;
}

// Method tries to find token in the string from specified position,
// returns position of first occurent or npos if not found
// It properly handles cases when content contains strings which 
// contains token but not euqal to it
// Example: content "/Transparency/ ..." pattern "/Trans
//          will return npos.
size_t Parser::findToken(const std::string &content, const std::string &keyword,size_t start)
{
   size_t cur_pos  = start;
   // lets find pattern first
   size_t foundStart = -1;
   size_t savedPos = 0;
   while( 1 )
   {
      cur_pos = content.find(keyword,cur_pos);
      if((int) cur_pos == -1 )
      {
         break;
      }
      savedPos = cur_pos;
      cur_pos += keyword.size();
      if( cur_pos < content.size() )
      {
          if((int) Parser::WHITESPACES.find(content[cur_pos]) != -1 ||
            (int)Parser::DELIMETERS.find(content[cur_pos]) != -1 )
         {
            foundStart = savedPos;
            break;
         }
      }
      else
      {
         foundStart = savedPos;
         // end of line is reached
         break;
      }
   }
   return foundStart;
}

// Method checks if token at current position can be a Name or it is not name but value
// Example
// /H /P /P 12 0 R
// the tag /P can be a name (and a value also), while 12 cannot
// start defines the position of token content
bool Parser::tokenIsAName(const std::string &content, size_t start )
{
   std::string openBraces = "<[({";
   bool found = false;
   while(1)
   {
      size_t foundNonWhite = content.find_first_not_of(Parser::WHITESPACES,start);
      size_t foundDelim = content.find_first_of(Parser::DELIMETERS,start);

      if( (int)foundNonWhite != -1 &&
          (int)foundDelim != -1 )
      {
         if( (foundNonWhite < foundDelim )  || ( (int)openBraces.find(content[foundDelim]) != -1) )
         {
            if( found )
            {
               return false;
            }
            else
            {
               return true;
            }
         }
         else
         {
            if( found )
            {
               return true;
            }
            else
            {
               found = true;
               start = content.find_first_of(Parser::WHITESPACES_AND_DELIMETERS,foundDelim+1);
            }
         }
      }
      else
      {
         return true;
      }
   }
}

// Method tries to find token name in the string from specified position,
// For example, the string contains /H /P /P 12 0 R.
// If search for /P then it will return position of /P 12 0 R, not value of 
// /H /P
size_t Parser::findTokenName(const std::string &content, const std::string &keyword,size_t start)
{
   size_t cur_pos  = start;
   // lets find pattern first
   size_t foundStart = -1;
   size_t savedPos = 0;
   std::string braces = "<[({";
   while( 1 )
   {
      cur_pos = content.find(keyword,cur_pos);
      if((int) cur_pos == -1 )
      {
         break;
      }
      savedPos = cur_pos;
      cur_pos += keyword.size();
      if( cur_pos < content.size() )
      {
          if((int) Parser::WHITESPACES_AND_DELIMETERS.find(content[cur_pos]) != -1 )
         {
            if( tokenIsAName(content,cur_pos ) )
            {
               foundStart = savedPos;
               break;
            }
         }
      }
      else
      {
         foundStart = savedPos;
         // end of line is reached
         break;
      }
   }
   return foundStart;
}

unsigned int Parser::findEndOfElementContent(const std::string &content,unsigned int startOfPageElement)
{
   unsigned int foundEnd = -1;
   std::stack<std::string> delimStack;
   std::string endDelim = "/]>)}";
   unsigned int curPos = startOfPageElement;
   std::string openDict("<");
   std::string openArray("[");
   std::string delimeter = endDelim;

   delimStack.push(delimeter); //initial delimeter

   bool compensation = true;
   while(1)
   {
      unsigned int nonWhiteSpace = content.find_first_not_of(Parser::WHITESPACES,curPos);

      unsigned int foundDelimeter = content.find_first_of(delimeter,curPos);
      unsigned int foundOpenBrace = content.find("[",curPos);
      unsigned int foundOpenDict = content.find("<",curPos);

      if((int) foundDelimeter == -1 && (int)foundOpenBrace == -1 && (int)foundOpenDict == -1 )
      {
         if( !delimStack.empty() )
         {
            delimStack.pop();
         }
      }
      else if( (foundDelimeter <= foundOpenBrace && foundDelimeter <= foundOpenDict ) )
      {
         if( !delimStack.empty() )
         {
            delimStack.pop();
         }
         if( nonWhiteSpace == foundDelimeter  && delimeter == endDelim )
         {
            curPos = foundDelimeter;
            if(content[foundDelimeter] == '/' && compensation )
            {
               curPos ++;
               compensation = false;
            }
         }
         else
         {
            compensation = false;
            if( delimeter == endDelim )
            {
               curPos = foundDelimeter;
            }
            else
            {
               curPos = foundDelimeter + delimeter.size();
            }
         }
      }
      else if( foundOpenBrace <= foundDelimeter && foundOpenBrace <= foundOpenDict )
      {
         compensation = false;
         delimStack.push("]");
         curPos = foundOpenBrace + openArray.size();
      }
      else if( foundOpenDict <= foundDelimeter && foundOpenDict <= foundOpenBrace )
      {
         compensation = false;
         delimStack.push(">");
         curPos = foundOpenDict + openDict.size();
      }
      if( delimStack.empty() )
      {
         foundEnd = content.find_first_of(delimeter,curPos);
         if((int) foundEnd == -1 )
         {
            foundEnd = curPos;
         }
         break;
      }
      delimeter = delimStack.top();

   }
   return foundEnd;
}

