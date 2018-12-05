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




#include "Merger.h"
#include "Parser.h"
#include "OverlayDocumentParser.h"
#include "Exception.h"

#include <map>
#include <iostream>

#include "core/memcheck.h"

using namespace merge_lib;

Parser Merger::_parser;

Merger::Merger():_baseDocuments(),_overlayDocument(0)
{

}



Merger::~Merger()
{
   std::map<std::string, Document *>::iterator docIterator = _baseDocuments.begin();
   for(; docIterator != _baseDocuments.end(); ++docIterator)
   {
      delete (*docIterator).second;
   }
   if( _overlayDocument )
   {
      delete _overlayDocument;
      _overlayDocument = 0;
   }
   _baseDocuments.clear();
}

void Merger::addBaseDocument(const char * docName)
{
   //if docName has been already opened then do nothing
   if(_baseDocuments.count(docName))
      return;
   Document * newBaseDoc = _parser.parseDocument(docName);
   _baseDocuments.insert(std::pair<std::string, Document *>(docName, newBaseDoc));
}

void Merger::addOverlayDocument(const char * docName)
{
   if( _overlayDocument )
   {
      delete _overlayDocument;
      _overlayDocument = 0;
   }
   if( !_overlayDocument )
   {
      OverlayDocumentParser overlayDocParser;
      _overlayDocument = overlayDocParser.parseDocument(docName);
      if( !_overlayDocument )
      {
         throw Exception("Error loading overlay document!");
      }
   }
}

// The main method which performs the merge
void Merger::merge(const char * overlayDocName, const MergeDescription & pagesToMerge)
{
   if( !_overlayDocument)
   {
      addOverlayDocument(overlayDocName);
      if( !_overlayDocument )
      {
         throw Exception("Error loading overlay document!");
      }
   }
   MergeDescription::const_iterator pageIterator = pagesToMerge.begin();
   for(; pageIterator != pagesToMerge.end(); ++pageIterator )
   {            
      Page * destinationPage = _overlayDocument->getPage( (*pageIterator).overlayPageNumber);
      if( destinationPage == 0 )
      {
         std::stringstream error;
         error << "There is no page with " << (*pageIterator).overlayPageNumber << 
               " number in " << overlayDocName;
         throw Exception(error);
      }
      Document * sourceDocument = _baseDocuments[(*pageIterator).baseDocumentName];
      Page * sourcePage = (sourceDocument == 0)? 0 : sourceDocument->getPage((*pageIterator).basePageNumber);
      bool isPageDuplicated = false;
      if( sourcePage )
      {
         unsigned int howManyTimesPageFound(0);
         for(size_t i = 0; i < pagesToMerge.size(); ++i)
         {
            if(pagesToMerge[i].basePageNumber == (*pageIterator).basePageNumber)
               ++howManyTimesPageFound;
            if(howManyTimesPageFound == 2)
               break;
         }
         isPageDuplicated = (2 == howManyTimesPageFound) ? true : false;
      }

      destinationPage->merge(sourcePage, _overlayDocument, const_cast<MergePageDescription&>((*pageIterator)), isPageDuplicated);
   }

}
// Method performs saving of merged documents into selected file
void Merger::saveMergedDocumentsAs(const char * outDocumentName)
{
   _overlayDocument->saveAs(outDocumentName);
}

