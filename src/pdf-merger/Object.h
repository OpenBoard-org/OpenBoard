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




#if !defined Object_h
#define Object_h

#include "Utils.h"

#include <cmath>
#include <string>
#include <fstream>
#include <map>
#include <set>
#include <vector>
#include <utility>

namespace merge_lib
{
    //This class represents pdf objects, and defines methods for performing 
    //all necessary operations on pdf objects
    //Each object consists of two parts: content and object's number
    //<object number> <number> obj
    //<content>
    //endobj
    //Object can contain several links to other object. These objects has been named "children".
    //Each reference (child object) should be kept with it position(s) in object's content.
    //After each content modification, all references should be changed too.
    //This convention lighten the recalculation object numbers work.
    class Object
    {
    public:
       friend class PageElementHandler;       
       typedef std::vector<unsigned int> ReferencePositionsInContent;
       typedef std::pair<Object *, ReferencePositionsInContent > ChildAndItPositionInContent;
       typedef std::map <unsigned int, ChildAndItPositionInContent> Children;
       Object(unsigned int objectNumber, unsigned int generationNumber, const std::string & objectContent, 
           std::string fileName = "", std::pair<unsigned int, unsigned int> streamBounds = std::make_pair ((unsigned int)0,(unsigned int)0), bool hasStream = false
                  ):
       m_number(objectNumber), m_generationNumber(generationNumber), m_oldNumber(objectNumber), m_content(objectContent),m_parents(),m_children(),m_isPassed(false),
           m_streamBounds(streamBounds), m_fileName(fileName), m_hasStream(hasStream), m_hasStreamInContent(false)
       {
       }
       virtual ~Object();
       Object *                    getClone(std::vector<Object *> & clones);
       void                        addChild(Object * child, const std::vector<unsigned int> childPositionsInContent);
       void                        addChild(const Children & children);
       ReferencePositionsInContent removeChild(Object * child);
       void                        forgetAboutChildren(unsigned int leftBound, unsigned int rightBound);
       Object *                    getChild(unsigned int objectNumber);
       bool                        findObject(const std::string & token, Object* & foundObject, unsigned int & tokenPositionInContent);
       std::vector<Object *>       getChildrenByBounds(unsigned int leftBound, unsigned int rightBound);
       std::vector<Object *>       getSortedByPositionChildren(unsigned int leftBound, unsigned int rightBound);
       void                        removeChildrenByBounds(unsigned int leftBound, unsigned int rightBound);
       const                       Children & getChildren();

       void                        removeHimself();

       unsigned int                getObjectNumber() const;
       unsigned int                getgenerationNumber() const;


       std::string &               getObjectContent();

       void                        setObjectContent(const std::string & objectContent);
       void                        appendContent(const std::string & addToContent);
       void                        eraseContent(unsigned int from, unsigned int size);
       void                        insertToContent(unsigned int position, const char * insertedStr, unsigned int length);
       void                        insertToContent(unsigned int position, const std::string & insertedStr);   

       //vector <object number, its size>
       void serialize(std::ofstream & out, std::map< unsigned int, std::pair<unsigned long long, unsigned int > > & sizesAndGenerationNumbers);

       void recalculateObjectNumbers(unsigned int & newNumber);

       bool isPassed()
       {
          return m_isPassed;
       }
       void retrieveMaxObjectNumber(unsigned int & maxNumber);
       void resetIsPassed()
       {
          if(m_isPassed)
             m_isPassed = false;
          Children::iterator it;
          for ( it=m_children.begin() ; it != m_children.end(); it++ )
          {
             if((*it).second.first->isPassed())
                (*it).second.first->resetIsPassed();
          }

       }
       unsigned int getOldNumber()
       {
          return m_oldNumber;
       }
       void setObjectNumber(unsigned int objNumber)
       {
          m_number = objNumber;
          m_oldNumber = objNumber;

       }
       bool getStream(std::string &);
       bool hasStream();
       bool getHeader(std::string &content);
       void forgetStreamInFile()
       {
            m_hasStreamInContent = true;
            m_hasStream = true;
       }

       std::string getNameSimpleValue(const std::string &content, const std::string &patten, size_t pos = 0);
       
       unsigned int getChildPosition(const Object * child); //throw (Exception)
       const std::set<Object *> & getParents()
       {
           return m_parents;
       }
       
      Object* findPatternInObjOrParents(const std::string &pattern);

    private:
       //methods
       Object(const Object & copy);
       Object * getCloneImpl(std::map<unsigned int, Object *> & clones);
       void addChildImpl(Object * child, const ReferencePositionsInContent & childPositionsInContent);
       void setObjectNumberImpl(unsigned int objectNumber);
       void addParentImpl(Object * child);
       bool findObjectImpl(const std::string & token, Object* & foundObject, unsigned int & tokenPositionInContent);
       void serializeImpl(std::ofstream  & out, const std::string & stream);
       void recalculateObjectNumbersImpl(unsigned int & maxNumber);
       void recalculateReferencePositionsImpl(unsigned int changedReference, int displacement);
       void retrieveMaxObjectNumberImpl(unsigned int & maxNumber);
       void serializeImpl(std::ofstream & out, std::map<unsigned int, unsigned long long> & sizes);
       bool getStreamFromContentImpl(std::string & stream);

       //members
       unsigned int                          m_number;
       unsigned int                          m_generationNumber;
       unsigned int                          m_oldNumber;
       std::string                           m_content;
       std::set <Object *>                   m_parents;
       Children                              m_children;
       bool                                  m_isPassed;
       std::pair<unsigned int, unsigned int> m_streamBounds;
       std::string                           m_fileName;
       bool                                  m_hasStream;
       bool                                  m_hasStreamInContent;

    };
}
#endif

