/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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

