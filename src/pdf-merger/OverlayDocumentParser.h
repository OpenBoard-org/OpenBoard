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

