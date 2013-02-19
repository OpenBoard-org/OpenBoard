/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


#if !defined OverlayDocumentParser_h
#define OverlayDocumentParser_h

#include "Object.h"
#include "Document.h"
#include "Page.h"
#include "Parser.h"
#include <map>
#include <iostream>

namespace merge_lib
{
   class Document;
   //This class parsed the pdf document and creates
   //an Document object
   class OverlayDocumentParser: private Parser
   {
   public:   
      OverlayDocumentParser(): Parser(), _fileName()  {};
      Document * parseDocument(const char * fileName);

   protected:
      unsigned int _readTrailerAndReturnRoot();
      
   private:
       //methods
      void         _getFileContent(const char * fileName);
      void         _readXRefAndCreateObjects();
      void         _readXref(std::map<unsigned int, unsigned long> & objectsAndSizes);
      void         _getPartOfFileContent(long startOfPart, unsigned int length);
      unsigned int _getStartOfXrefWithRoot();
      //constants
      static int DOC_PART_WITH_START_OF_XREF;

      //members
      std::string _fileName;
   };
}
#endif

