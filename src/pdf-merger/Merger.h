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
      std::map<std::string, Document * > _baseDocuments;
      static Parser _parser;
      Document * _overlayDocument;
   };
}
#endif //
