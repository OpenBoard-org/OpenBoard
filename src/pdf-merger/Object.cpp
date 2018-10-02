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




#include "Object.h"
#include "Parser.h"
#include "Exception.h"
#include <string.h>
#include <algorithm>
#include <fstream>

#include "core/memcheck.h"

using namespace merge_lib;

std::string NUMBERANDWHITESPACE(" 0123456789");


Object::~Object()
{       
   _parents.clear();
   _children.clear();
   _content.clear();
}

Object * Object::getClone(std::vector<Object *> & clones)
{
   std::map<unsigned int, Object *> clonesMap;
   Object * clone = _getClone(clonesMap);
   std::map<unsigned int, Object *>::iterator conesIterator = clonesMap.begin();
   for(; conesIterator != clonesMap.end(); ++conesIterator)
      clones.push_back((*conesIterator).second);
   clonesMap.clear();
   resetIsPassed();
   return clone;
}

Object * Object::_getClone(std::map<unsigned int, Object *> & clones)
{
   _isPassed = true;
   unsigned int objectNumber = this->getObjectNumber();   
   Object * clone = new Object(objectNumber, this->_generationNumber, this->getObjectContent(), _fileName, _streamBounds, _hasStream);
   clone->_hasStreamInContent = _hasStreamInContent;
   clones.insert(std::pair<unsigned int, Object *>(objectNumber, clone));
   Children::iterator currentChild = _children.begin();

   for(; currentChild != _children.end(); ++currentChild)
   {
      Object * currentObject = (*currentChild).second.first;
      unsigned int childObjectNumber = currentObject->getObjectNumber();

      Object * cloneOfCurrentChild = 0;

      if(currentObject->isPassed())
      {
         cloneOfCurrentChild = clones[childObjectNumber];
      }
      else
      {
         cloneOfCurrentChild = currentObject->_getClone(clones);
      }
      ChildAndItPositionInContent newChild(
         cloneOfCurrentChild, 
         currentChild->second.second);

      clone->_children.insert(std::pair<unsigned int, ChildAndItPositionInContent>
         (newChild.first->getObjectNumber(), newChild)
         );
   }
   return clone;

}
void Object::addChild(Object * child, const std::vector<unsigned int> childPositionsInContent)
{
   child->_addParent(this);
   _addChild(child, childPositionsInContent);
}


Object::ReferencePositionsInContent Object::removeChild(Object * child)
{
   ReferencePositionsInContent positions = _children[child->getObjectNumber()].second;
   _children.erase(child->getObjectNumber());
   return positions;
}

void Object::forgetAboutChildren(unsigned int leftBound, unsigned int rightBound)
{
   std::vector<Object *> children = getChildrenByBounds(leftBound, rightBound);
   for(size_t i = 0; i < children.size(); ++i)
   {         
      _children.erase(_children.find(children[i]->getObjectNumber()));         
   }
}


Object * Object::getChild(unsigned int objectNumber)
{
   //TODO: check object before returning
   return _children[objectNumber].first;
}

std::vector<Object *> Object::getChildrenByBounds(unsigned int leftBound, unsigned int rightBound)
{
   std::vector<Object *> result;
   for(Children::iterator currentChild = _children.begin(); currentChild != _children.end(); ++currentChild)
   {
      ReferencePositionsInContent childPositions = (*currentChild).second.second;
      for(size_t i = 0; i < childPositions.size(); ++i)
      {
         unsigned int childPosition = childPositions[i];
         if((childPosition >= leftBound) &&  (childPosition <= rightBound))
         {
            result.push_back( (*currentChild).second.first);
            break;
         }
      }
   }
   return result;
}


std::vector<Object *> Object::getSortedByPositionChildren(unsigned int leftBound, unsigned int rightBound)
{
   std::vector<Object *> result;
   for(Children::iterator currentChild = _children.begin(); currentChild != _children.end(); ++currentChild)
   {
      ReferencePositionsInContent childPositions = (*currentChild).second.second;
      for(size_t i = 0; i < childPositions.size(); ++i)
      {
         unsigned int childPosition = childPositions[i];
         if((childPosition >= leftBound) &&  (childPosition <= rightBound))
         {
            unsigned int insertPosition = 0;
            for(unsigned int j(0); j < result.size(); ++j)
               if(childPosition > getChildPosition(result[j]))
                  insertPosition = j + 1;
            result.insert(result.begin() + insertPosition, (*currentChild).second.first);
            break;
         }
      }
   }
   return result;

}


unsigned int Object::getChildPosition(const Object * child)//throw (Exception)
{
   const ReferencePositionsInContent & childrenPostion = _children[child->getObjectNumber()].second;
   if(
      (childrenPostion.size() != 1) ||
      (_children[child->getObjectNumber()].first != child)
      )
      throw Exception("Internal error or wrong document (some reference is found twise)");
   return childrenPostion[0];
}

void Object::removeChildrenByBounds(unsigned int leftBound, unsigned int rightBound)
{
   std::vector<Object *> children = getChildrenByBounds(leftBound, rightBound);
   for(size_t i = 0; i < children.size(); ++i)
   {
      children[i]->removeHimself();
   }
}


const Object::Children & Object::getChildren()
{
   return _children;
}

void Object::removeHimself()
{
   if(!_parents.empty())
   {
      std::set<Object *>::iterator currentParent = _parents.begin();
      for(; currentParent != _parents.end(); ++currentParent)
      {
         (*currentParent)->removeChild(this);
      }
   }
}
unsigned int Object::getObjectNumber() const
{
   return _number;
}

unsigned int Object::getgenerationNumber() const
{
   return _generationNumber;
}

std::string & Object::getObjectContent()
{
   return _content;
}

void Object::_setObjectNumber(unsigned int objectNumber)
{
   if(!isPassed())
   {
      _isPassed = true;
      _oldNumber = _number;
      _number = objectNumber;
   }
}

void Object::setObjectContent(const std::string & objectContent)
{
   _content = objectContent;
}

void Object::appendContent(const std::string & addToContent)
{
   _content.append(addToContent);
}

void Object::eraseContent(unsigned int from, unsigned int size)
{
   int iSize = size;
   _recalculateReferencePositions(from + size, -iSize);
   _content.erase(from, size);
}

void Object::insertToContent(unsigned int position, const std::string & insertedStr)
{
   _recalculateReferencePositions(position, insertedStr.size());
   _content.insert(position, insertedStr);
}

void Object::insertToContent(unsigned int position, const char * insertedStr, unsigned int length)
{    
   _recalculateReferencePositions(position, length);
   _content.insert(position, insertedStr, length);    
}

//vector <object number, its size>
void Object::serialize(std::ofstream & out, std::map< unsigned int, std::pair<unsigned long long, unsigned int > > & sizesAndGenerationNumbers)
{
   //is this element already printed
   if(sizesAndGenerationNumbers.find(_number) != sizesAndGenerationNumbers.end()) return;

   std::string stream;
   if(_hasStream && !_hasStreamInContent)
   {       
      getStream(stream);
      stream.append("endstream\n");       
   }
   // xxxx + " " + "0" + " " + "obj" + "\n" + _content.size() + "endobj\n", where x - is a digit
   unsigned long long objectSizeForXref = (static_cast<unsigned int>(std::log10(static_cast<double>(_number))) + 1) + 14 + _content.size() + stream.size();    

   sizesAndGenerationNumbers.insert(std::pair<unsigned int, std::pair<unsigned long long, unsigned int > >(_number, std::make_pair(objectSizeForXref, _generationNumber)));

   _serialize(out, stream);
   stream.clear();
   stream.reserve();

   //call serialize of each child
   Children::iterator it;
   for ( it=_children.begin() ; it != _children.end(); it++ )
   {
      Object * currentChild = (*it).second.first;
      currentChild->serialize(out, sizesAndGenerationNumbers);
   }
}
void Object::recalculateObjectNumbers(unsigned int & newNumber)
{    
   _recalculateObjectNumbers(newNumber);
   resetIsPassed();
}

void Object::_recalculateObjectNumbers(unsigned int & newNumber)
{    
   _setObjectNumber(newNumber);

   Children::iterator childIterator;
   for ( childIterator = _children.begin() ; childIterator != _children.end(); ++childIterator )
   {
      Object * currentChild = (*childIterator).second.first;
      if(currentChild->isPassed()) continue;                
      currentChild->_recalculateObjectNumbers(++newNumber);        
   }

   //recalculate referencies in content
   for ( childIterator = _children.begin() ; childIterator != _children.end(); ++childIterator)
   {    
      Object * currentChild = (*childIterator).second.first;
      //if(currentChild->getOldNumber() == currentChild->getObjectNumber()) continue;
      const ReferencePositionsInContent & refPositionForcurrentChild = (*childIterator).second.second;
      const std::string & oldNumberStr = Utils::uIntToStr(currentChild->getOldNumber());
      const std::string & newNumber = Utils::uIntToStr(currentChild->getObjectNumber());
      const unsigned int newNumberStringSize = newNumber.size();
      const unsigned int oldNumberStringSize = oldNumberStr.size();
      unsigned int diff = newNumberStringSize;
      if (newNumberStringSize > oldNumberStringSize)
      {

         for(size_t referencePositionIter(0); referencePositionIter < refPositionForcurrentChild.size(); ++referencePositionIter)               
         {
            _recalculateReferencePositions(refPositionForcurrentChild[referencePositionIter], newNumberStringSize - oldNumberStringSize);
            for(size_t referenceStringInter(oldNumberStringSize); referenceStringInter < newNumberStringSize; ++referenceStringInter )  
               _content.insert(
               refPositionForcurrentChild[referencePositionIter] + referenceStringInter, 
               1,
               newNumber[referenceStringInter]);                
         }
         diff = oldNumberStringSize;
      }
      if (newNumberStringSize < oldNumberStringSize)
      {
         for(size_t referencePositionIter(0); referencePositionIter < refPositionForcurrentChild.size(); ++referencePositionIter)
         {
            _recalculateReferencePositions(refPositionForcurrentChild[referencePositionIter], newNumberStringSize - oldNumberStringSize);
            _content.erase(refPositionForcurrentChild[referencePositionIter] + newNumberStringSize, 
               oldNumberStringSize - newNumberStringSize
               );
         }
      }    

      for(unsigned int i = 0; i < diff; i++)
         for(size_t referencePositionIter(0); referencePositionIter < refPositionForcurrentChild.size(); ++referencePositionIter)            
            _content[i + refPositionForcurrentChild[referencePositionIter]] = newNumber[i];


   }    
}

//this method should be called in case changing object's content
void Object::_recalculateReferencePositions(unsigned int changedReference, int displacement)
{
   Children::iterator childIterator;
   for ( childIterator = _children.begin() ; childIterator != _children.end(); ++childIterator )
   {
      ReferencePositionsInContent & refPositionForcurrentChild = (*childIterator).second.second;
      for(size_t i = 0; i < refPositionForcurrentChild.size(); ++i)
         if(refPositionForcurrentChild[i] > changedReference)
            refPositionForcurrentChild[i] += displacement;

   }
}


void Object::_retrieveMaxObjectNumber(unsigned int & maxNumber)
{

   if(isPassed())  return;
   _isPassed = true;
   if(maxNumber < _number)
      maxNumber = _number;
   Children::iterator it;
   for ( it=_children.begin() ; it != _children.end(); ++it )
      (*it).second.first->_retrieveMaxObjectNumber(maxNumber);       
}

//TODO add check for absent token
bool Object::_findObject(const std::string & token, Object* & foundObject, unsigned int & tokenPositionInContent)
{
   _isPassed = true;
   tokenPositionInContent = Parser::findToken(_content,token);
   if((int)tokenPositionInContent != -1)
   {
      foundObject = this;
      return true;
   }      
   for (Children::iterator  it=_children.begin() ; it != _children.end(); ++it )
      if((!(*it).second.first->_isPassed) &&
         ((*it).second.first->_findObject(token, foundObject, tokenPositionInContent)))
         return true;

   return false;
}

bool Object::findObject(const std::string & token, Object* & foundObject, unsigned int & tokenPositionInContent)
{
   bool result = _findObject(token, foundObject, tokenPositionInContent);
   resetIsPassed();
   if(result)
   {    
      return true;
   }
   return false;
}

void Object::retrieveMaxObjectNumber(unsigned int & maxNumber)
{
   _retrieveMaxObjectNumber(maxNumber);
   resetIsPassed();   
}

//methods 
void Object::_addChild(Object * child, const ReferencePositionsInContent & childPositionsInContent)
{
   ChildAndItPositionInContent childAndItPositions(child, childPositionsInContent);
   unsigned int childObjectNumber = child->getObjectNumber();
   while(_children.count(childObjectNumber))
      ++childObjectNumber;
   _children.insert(std::pair<unsigned int, ChildAndItPositionInContent > (childObjectNumber, childAndItPositions));
}


void Object::_addParent(Object * child)
{
   _parents.insert(child);
}
void Object::_serialize(std::ofstream  & out, const std::string & stream)
{
    out << _number << " " << _generationNumber << " obj\n" << _content << stream << "endobj\n";
   out.flush();
}

/** @brief getStream
*
* @todo: document this function
*/
bool Object::getStream(std::string & stream)
{
   if(!_hasStream && !_hasStreamInContent)
      return false;
   if( _hasStream && _hasStreamInContent)
   {
      if(_getStreamFromContent(stream))
         return true;
      else
         return false;
   }

   std::ifstream pdfFile;
   pdfFile.open (_fileName.c_str(), std::ios::binary );
   if (pdfFile.fail())
   {
      std::stringstream errorMessage("File ");
      errorMessage << _fileName << " is absent" << "\0";
      throw Exception(errorMessage);
   }
   // get length of file:
   int length = _streamBounds.second - _streamBounds.first;
   pdfFile.seekg (_streamBounds.first, std::ios_base::beg);
   stream.resize(length);
   pdfFile.read(&stream[0], length);   
   pdfFile.close();
   return true;
}

bool Object::_getStreamFromContent(std::string & stream)
{
   size_t stream_begin = _content.find("stream");
   if((int) stream_begin == -1 )
   {
      return false;
   }
   size_t stream_end = _content.find("endstream",stream_begin);
   if((int) stream_end == -1 )
   {
      return false;
   }
   stream_begin += strlen("stream");
   // need to skip trailing \r
   while(_content[stream_begin] == '\r')
   {
      stream_begin ++;
   }
   if( _content[stream_begin] == '\n')
   {
      stream_begin ++;
   }

   stream = _content.substr(stream_begin, stream_end - stream_begin);
   return true;
}


/** @brief getHeader
*
* @todo: document this function
*/
bool Object::getHeader(std::string &content)
{
   if( !hasStream() )
   {
      content = _content;
      return true;
   }
   size_t stream_begin = _content.find("stream");
   content = _content.substr(0,stream_begin);
   return true;
}


/** @brief hasStream
*
* @todo: document this function
*/
bool Object::hasStream()
{
   return _hasStream;
}

// the method returns the value of some object.
// For example, .../Length 123 /Filter will return  123
// For /Length 12 0 R will return the content of 12 0 obj
std::string Object::getNameSimpleValue(const std::string &content, const std::string &pattern, size_t start)
{
   size_t foundStart, foundEnd;
   std::string token = Parser::findTokenStr(content,pattern,start,foundStart,foundEnd);

   std::string value;
   size_t beg = 0;

   // Now token could be /Length 127 or /Length 12 0 R
   if( Parser::getNextWord(value,token,beg) )
   {
      // 127 or 12 
      std::string interm;
      if( Parser::getNextWord(interm,token,beg) )  // 0?
      {
         if( Parser::getNextWord(interm,token,beg) ) // R
         {
            if( interm == "R" )  // we found reference to object!
            {
               int number = Utils::stringToInt(value);
               Object *child = getChild(number);
               if( child )
               {
                  value = child->getObjectContent();
                  Parser::trim(value);                  
               }
               else
               {
                  std::cerr<<"Error::child object with number "<<number<<"is absent\n";
               }
            }
            else
            {
               std::cerr<<"Error:undefined format of token "<<token<<"\n";
            }
         }
         else
         {
            std::cerr<<"Error:undefined word"<<interm<<"\n";
         }
      }
   }
   return value;
}

Object* Object::findPatternInObjOrParents(const std::string &pattern)
{
   std::string content=getObjectContent();
   if((int) Parser::findToken(content,pattern,0) != -1 )
   {
      return this;
   }

   Object * parent = this;
   Object *foundObj = NULL;
   while(1)
   {
      unsigned int startOfParent = content.find("/Parent");
      unsigned int endOfParent = content.find(" R", startOfParent);
      if((int)startOfParent == -1)
      {
         break;
      }
      std::vector <Object *> parents = parent->getChildrenByBounds(startOfParent, endOfParent);
      if( parents.size() != 1 )
      {
         break;
      }
      parent = parents[0];
      std::string parentContent = parent->getObjectContent();
      unsigned int startOfPattern = parentContent.find(pattern);
      if((int)startOfPattern == -1)
      {
         content = parentContent;
         continue;
      }
      foundObj = parent;
      break;
   }
   return foundObj;
}

