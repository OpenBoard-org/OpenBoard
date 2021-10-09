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




#if !defined Merger_h
#define Merger_h

#include "Document.h"
#include "Parser.h"
#include <map>

// structure defines parameter of merge


namespace merge_lib
{
   class Document;
   class Merger
   {

   public:
      Merger();
      ~Merger();

      //this method should be called every time the "Add" button is clicked
      void addBaseDocument(const char *docName);

      void addOverlayDocument(const char *docName);

      void saveMergedDocumentsAs(const char *outDocumentName);

      void merge(const char *overlayDocName, const MergeDescription & pagesToMerge);

   private:
      std::map<std::string, Document * > m_baseDocuments;
      static Parser s_parser;
      Document * m_overlayDocument;
   };
}
#endif //
