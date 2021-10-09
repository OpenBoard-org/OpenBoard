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
   m_parents.clear();
   m_children.clear();
   m_content.clear();
}

Object * Object::getClone(std::vector<Object *> & clones)
{
   std::map<unsigned int, Object *> clonesMap;
   Object * clone = getCloneImpl(clonesMap);
   std::map<unsigned int, Object *>::iterator conesIterator = clonesMap.begin();
   for(; conesIterator != clonesMap.end(); ++conesIterator)
      clones.push_back((*conesIterator).second);
   clonesMap.clear();
   resetIsPassed();
   return clone;
}

Object * Object::getCloneImpl(std::map<unsigned int, Object *> & clones)
{
   m_isPassed = true;
   unsigned int objectNumber = this->getObjectNumber();   
   Object * clone = new Object(objectNumber, this->m_generationNumber, this->getObjectContent(), m_fileName, m_streamBounds, m_hasStream);
   clone->m_hasStreamInContent = m_hasStreamInContent;
   clones.insert(std::pair<unsigned int, Object *>(objectNumber, clone));
   Children::iterator currentChild = m_children.begin();

   for(; currentChild != m_children.end(); ++currentChild)
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
         cloneOfCurrentChild = currentObject->getCloneImpl(clones);
      }
      ChildAndItPositionInContent newChild(
         cloneOfCurrentChild, 
         currentChild->second.second);

      clone->m_children.insert(std::pair<unsigned int, ChildAndItPositionInContent>
         (newChild.first->getObjectNumber(), newChild)
         );
   }
   return clone;

}
void Object::addChild(Object * child, const std::vector<unsigned int> childPositionsInContent)
{
   child->addParentImpl(this);
   addChildImpl(child, childPositionsInContent);
}


Object::ReferencePositionsInContent Object::removeChild(Object * child)
{
   ReferencePositionsInContent positions = m_children[child->getObjectNumber()].second;
   m_children.erase(child->getObjectNumber());
   return positions;
}

void Object::forgetAboutChildren(unsigned int leftBound, unsigned int rightBound)
{
   std::vector<Object *> children = getChildrenByBounds(leftBound, rightBound);
   for(size_t i = 0; i < children.size(); ++i)
   {         
      m_children.erase(m_children.find(children[i]->getObjectNumber()));
   }
}


Object * Object::getChild(unsigned int objectNumber)
{
   //TODO: check object before returning
   return m_children[objectNumber].first;
}

std::vector<Object *> Object::getChildrenByBounds(unsigned int leftBound, unsigned int rightBound)
{
   std::vector<Object *> result;
   for(Children::iterator currentChild = m_children.begin(); currentChild != m_children.end(); ++currentChild)
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
   for(Children::iterator currentChild = m_children.begin(); currentChild != m_children.end(); ++currentChild)
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
   const ReferencePositionsInContent & childrenPostion = m_children[child->getObjectNumber()].second;
   if(
      (childrenPostion.size() != 1) ||
      (m_children[child->getObjectNumber()].first != child)
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
   return m_children;
}

void Object::removeHimself()
{
   if(!m_parents.empty())
   {
      std::set<Object *>::iterator currentParent = m_parents.begin();
      for(; currentParent != m_parents.end(); ++currentParent)
      {
         (*currentParent)->removeChild(this);
      }
   }
}
unsigned int Object::getObjectNumber() const
{
   return m_number;
}

unsigned int Object::getgenerationNumber() const
{
   return m_generationNumber;
}

std::string & Object::getObjectContent()
{
   return m_content;
}

void Object::setObjectNumberImpl(unsigned int objectNumber)
{
   if(!isPassed())
   {
      m_isPassed = true;
      m_oldNumber = m_number;
      m_number = objectNumber;
   }
}

void Object::setObjectContent(const std::string & objectContent)
{
   m_content = objectContent;
}

void Object::appendContent(const std::string & addToContent)
{
   m_content.append(addToContent);
}

void Object::eraseContent(unsigned int from, unsigned int size)
{
   int iSize = size;
   recalculateReferencePositionsImpl(from + size, -iSize);
   m_content.erase(from, size);
}

void Object::insertToContent(unsigned int position, const std::string & insertedStr)
{
   recalculateReferencePositionsImpl(position, insertedStr.size());
   m_content.insert(position, insertedStr);
}

void Object::insertToContent(unsigned int position, const char * insertedStr, unsigned int length)
{    
   recalculateReferencePositionsImpl(position, length);
   m_content.insert(position, insertedStr, length);
}

//vector <object number, its size>
void Object::serialize(std::ofstream & out, std::map< unsigned int, std::pair<unsigned long long, unsigned int > > & sizesAndGenerationNumbers)
{
   //is this element already printed
   if(sizesAndGenerationNumbers.find(m_number) != sizesAndGenerationNumbers.end()) return;

   std::string stream;
   if(m_hasStream && !m_hasStreamInContent)
   {       
      getStream(stream);
      stream.append("endstream\n");       
   }
   // xxxx + " " + "0" + " " + "obj" + "\n" + m_content.size() + "endobj\n", where x - is a digit
   unsigned long long objectSizeForXref = (static_cast<unsigned int>(std::log10(static_cast<double>(m_number))) + 1) + 14 + m_content.size() + stream.size();

   sizesAndGenerationNumbers.insert(std::pair<unsigned int, std::pair<unsigned long long, unsigned int > >(m_number, std::make_pair(objectSizeForXref, m_generationNumber)));

   serializeImpl(out, stream);
   stream.clear();
   stream.reserve();

   //call serialize of each child
   Children::iterator it;
   for ( it=m_children.begin() ; it != m_children.end(); it++ )
   {
      Object * currentChild = (*it).second.first;
      currentChild->serialize(out, sizesAndGenerationNumbers);
   }
}
void Object::recalculateObjectNumbers(unsigned int & newNumber)
{    
   recalculateObjectNumbersImpl(newNumber);
   resetIsPassed();
}

void Object::recalculateObjectNumbersImpl(unsigned int & newNumber)
{    
   setObjectNumberImpl(newNumber);

   Children::iterator childIterator;
   for ( childIterator = m_children.begin() ; childIterator != m_children.end(); ++childIterator )
   {
      Object * currentChild = (*childIterator).second.first;
      if(currentChild->isPassed()) continue;                
      currentChild->recalculateObjectNumbersImpl(++newNumber);
   }

   //recalculate referencies in content
   for ( childIterator = m_children.begin() ; childIterator != m_children.end(); ++childIterator)
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
            recalculateReferencePositionsImpl(refPositionForcurrentChild[referencePositionIter], newNumberStringSize - oldNumberStringSize);
            for(size_t referenceStringInter(oldNumberStringSize); referenceStringInter < newNumberStringSize; ++referenceStringInter )  
               m_content.insert(
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
            recalculateReferencePositionsImpl(refPositionForcurrentChild[referencePositionIter], newNumberStringSize - oldNumberStringSize);
            m_content.erase(refPositionForcurrentChild[referencePositionIter] + newNumberStringSize,
               oldNumberStringSize - newNumberStringSize
               );
         }
      }    

      for(unsigned int i = 0; i < diff; i++)
         for(size_t referencePositionIter(0); referencePositionIter < refPositionForcurrentChild.size(); ++referencePositionIter)            
            m_content[i + refPositionForcurrentChild[referencePositionIter]] = newNumber[i];


   }    
}

//this method should be called in case changing object's content
void Object::recalculateReferencePositionsImpl(unsigned int changedReference, int displacement)
{
   Children::iterator childIterator;
   for ( childIterator = m_children.begin() ; childIterator != m_children.end(); ++childIterator )
   {
      ReferencePositionsInContent & refPositionForcurrentChild = (*childIterator).second.second;
      for(size_t i = 0; i < refPositionForcurrentChild.size(); ++i)
         if(refPositionForcurrentChild[i] > changedReference)
            refPositionForcurrentChild[i] += displacement;

   }
}


void Object::retrieveMaxObjectNumberImpl(unsigned int & maxNumber)
{

   if(isPassed())  return;
   m_isPassed = true;
   if(maxNumber < m_number)
      maxNumber = m_number;
   Children::iterator it;
   for ( it=m_children.begin() ; it != m_children.end(); ++it )
      (*it).second.first->retrieveMaxObjectNumber(maxNumber);
}

//TODO add check for absent token
bool Object::findObjectImpl(const std::string & token, Object* & foundObject, unsigned int & tokenPositionInContent)
{
   m_isPassed = true;
   tokenPositionInContent = Parser::findToken(m_content,token);
   if((int)tokenPositionInContent != -1)
   {
      foundObject = this;
      return true;
   }      
   for (Children::iterator  it=m_children.begin() ; it != m_children.end(); ++it )
      if((!(*it).second.first->m_isPassed) &&
         ((*it).second.first->findObject(token, foundObject, tokenPositionInContent)))
         return true;

   return false;
}

bool Object::findObject(const std::string & token, Object* & foundObject, unsigned int & tokenPositionInContent)
{
   bool result = findObjectImpl(token, foundObject, tokenPositionInContent);
   resetIsPassed();
   if(result)
   {    
      return true;
   }
   return false;
}

void Object::retrieveMaxObjectNumber(unsigned int & maxNumber)
{
   retrieveMaxObjectNumberImpl(maxNumber);
   resetIsPassed();   
}

//methods 
void Object::addChildImpl(Object * child, const ReferencePositionsInContent & childPositionsInContent)
{
   ChildAndItPositionInContent childAndItPositions(child, childPositionsInContent);
   unsigned int childObjectNumber = child->getObjectNumber();
   while(m_children.count(childObjectNumber))
      ++childObjectNumber;
   m_children.insert(std::pair<unsigned int, ChildAndItPositionInContent > (childObjectNumber, childAndItPositions));
}


void Object::addParentImpl(Object * child)
{
   m_parents.insert(child);
}
void Object::serializeImpl(std::ofstream  & out, const std::string & stream)
{
    out << m_number << " " << m_generationNumber << " obj\n" << m_content << stream << "endobj\n";
   out.flush();
}

/** @brief getStream
*
* @todo: document this function
*/
bool Object::getStream(std::string & stream)
{
   if(!m_hasStream && !m_hasStreamInContent)
      return false;
   if( m_hasStream && m_hasStreamInContent)
   {
      if(getStreamFromContentImpl(stream))
         return true;
      else
         return false;
   }

   std::ifstream pdfFile;
   pdfFile.open (m_fileName.c_str(), std::ios::binary );
   if (pdfFile.fail())
   {
      std::stringstream errorMessage("File ");
      errorMessage << m_fileName << " is absent" << "\0";
      throw Exception(errorMessage);
   }
   // get length of file:
   int length = m_streamBounds.second - m_streamBounds.first;
   pdfFile.seekg (m_streamBounds.first, std::ios_base::beg);
   stream.resize(length);
   pdfFile.read(&stream[0], length);   
   pdfFile.close();
   return true;
}

bool Object::getStreamFromContentImpl(std::string & stream)
{
   size_t stream_begin = m_content.find("stream");
   if((int) stream_begin == -1 )
   {
      return false;
   }
   size_t stream_end = m_content.find("endstream",stream_begin);
   if((int) stream_end == -1 )
   {
      return false;
   }
   stream_begin += strlen("stream");
   // need to skip trailing \r
   while(m_content[stream_begin] == '\r')
   {
      stream_begin ++;
   }
   if( m_content[stream_begin] == '\n')
   {
      stream_begin ++;
   }

   stream = m_content.substr(stream_begin, stream_end - stream_begin);
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
      content = m_content;
      return true;
   }
   size_t stream_begin = m_content.find("stream");
   content = m_content.substr(0,stream_begin);
   return true;
}


/** @brief hasStream
*
* @todo: document this function
*/
bool Object::hasStream()
{
   return m_hasStream;
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

