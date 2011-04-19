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
	  void         _getFileContent(const char * fileName){};
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

