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
