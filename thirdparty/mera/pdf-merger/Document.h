///////////////////////////////////////////////////////////
//  Document.h
//  Implementation of the Class Document
//  Created on:      19-???-2009 12:27:55
///////////////////////////////////////////////////////////

#if !defined Document_h
#define Document_h

#include "Page.h"
#include <vector>

namespace merge_lib
{
   //this class contains all info about pdf document
   class Document
   {
   //only this class can fill out Document
   friend class Parser;
   friend class OverlayDocumentParser;

   public:
      ~Document();

      //get page by number
      //this method throws exception if page is absent
      Page *   getPage(unsigned int pageNumber);
      
      //save document with newFileName file name
      void     saveAs(const char * newFileName);   

      //get root of all document objects
      Object * getDocumentObject();

      //add objects to allObjects repository
      void addToAllObjects(const std::vector<Object *> & allObjs)
      {
         _allObjects.insert(_allObjects.end(), allObjs.begin(), allObjs.end());
      }

   private:
      //methods   
      Document(const char * docName);
      //members

      //root of all document's objects
      Object * _root;

      //document's pages
      //key - is a page number
      //values is a page
      std::map <unsigned int, Page *> _pages;

      //repository for all objects. 
      //this repository is using to destroy all document's objects in destructor
      std::vector<Object *> _allObjects;

      std::string _documentName;

      //max number of all document's objects
      unsigned int _maxObjectNumber;

   };
}
#endif
