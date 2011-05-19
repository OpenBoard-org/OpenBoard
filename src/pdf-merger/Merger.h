///////////////////////////////////////////////////////////
//  Merger.h
//  Implementation of the Class Merger
//  Created on:      19-θών-2009 12:27:54
///////////////////////////////////////////////////////////

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
