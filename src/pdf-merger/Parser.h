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




#if !defined Parser_h
#define Parser_h

#include "Object.h"
#include "Document.h"
#include "Page.h"

#include <string>
#include <vector>


namespace merge_lib
{
   class Document;

   //This class parsed the pdf document and creates
   //an Document object
   class Parser
   {
   public:   
      Parser(): _root(0), _fileContent(), _objects(), _document(0)  {};
      Document * parseDocument(const char * fileName);

      static const std::string WHITESPACES;
      static const std::string DELIMETERS;
      static const std::string NUMBERS;
      static const std::string WHITESPACES_AND_DELIMETERS;

      static bool getNextWord(std::string & out, const std::string &in, size_t &nextPosition,size_t *found = NULL);
      static std::string getNextToken( const std::string &in, unsigned &position);
      static void trim(std::string &str);
      static std::string findTokenStr(const std::string &content, const std::string &pattern, size_t start,size_t &foundStart, size_t &foundEnd); 

      static size_t findToken(const std::string &content, const std::string &keyword,size_t start = 0);
      static size_t findTokenName(const std::string &content, const std::string &keyword,size_t start = 0);
      static unsigned int findEndOfElementContent(const std::string &content, unsigned int startOfPageElement);
      static bool tokenIsAName(const std::string &content, size_t start );
   protected:
      const std::string &                           _getObjectContent(unsigned int objectPosition, unsigned int & objectNumber, unsigned int & generationNumber, std::pair<unsigned int, unsigned int> &, bool &);
      virtual unsigned int                          _readTrailerAndReturnRoot();
   private:
      //methods
      virtual void                                  _getFileContent(const char * fileName);
      bool                                          _getNextObject(Object * object);
      void                                          _callObserver(std::string objectContent);
      void                                          _createObjectTree(const char * fileName);
      void                                          _retrieveAllPages(Object * objectWithKids);
      void                                          _fillOutObjects();
      virtual void                                  _readXRefAndCreateObjects();
      unsigned int                                  _getEndOfLineFromContent(unsigned int fromPosition);
      const std::pair<unsigned int, unsigned int> & _getLineBounds(const std::string & str, unsigned int fromPosition);
      const std::string &                           _getNextToken(unsigned int & fromPosition);
      unsigned int                                  _countTokens(unsigned int leftBound, unsigned int rightBount);
      unsigned int                                  _skipWhiteSpaces(const std::string & str);
      unsigned int                                  _skipWhiteSpacesFromContent(unsigned int fromPosition);
      const std::map<unsigned int, Object::ReferencePositionsInContent> & _getReferences(const std::string & objectContent);
      unsigned int                                  _skipNumber(const std::string & str, unsigned int currentPosition);      
      unsigned int                                  _skipWhiteSpaces(const std::string & str, unsigned int fromPosition);
      void                                          _createDocument(const char * docName);      
      virtual unsigned int                          _getStartOfXrefWithRoot();
      unsigned int                                  _readTrailerAndRterievePrev(const unsigned int startPositionForSearch, unsigned int & previosXref);
      void                                          _clearParser();      
      

   protected:  

      //members
      Object *                         _root;
      std::string                      _fileContent;
      std::map<unsigned int, Object *> _objects;
      Document *                       _document;
      
   };
}
#endif

