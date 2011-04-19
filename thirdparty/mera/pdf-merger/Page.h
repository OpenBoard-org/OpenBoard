///////////////////////////////////////////////////////////
//  Page.h
//  Implementation of the Class Page
//  Created on:      19-θών-2009 12:27:56
///////////////////////////////////////////////////////////

#if !defined Page_h
#define Page_h

#include <string>
#include "Object.h"
#include "MergePageDescription.h"


namespace merge_lib
{
   class Document;


   class Page
   {
      friend class Parser;
      friend class PageElementsFactory;


   public:
      Page(unsigned int pageNumber);
      ~Page();
      unsigned int getPageNumber()
      {
         return _pageNumber;
      }
      void merge(Page * sourcePage, Document * parentDocument, MergePageDescription & description, bool isPageDuplicated);

      void recalculateObjectNumbers(unsigned int & newNumber);
      std::string & getPageContent();
      const Object::Children &   getPageRefs();
      Object * pageToXObject(std::vector<Object *> & allObjects, std::vector<Object *> & annots, bool isCloneNeeded);
      void setRotation(int rotation)
      {
         _rotation = rotation;
      }

   private:
      //methods
      Object * _pageToXObject(Object *& page, std::vector<Object *> & annots);
      std::string _getMergedPageContent(  unsigned int & contentPosition, 
         unsigned int & parentPosition, 
         unsigned int & originalPage1Position, 
         unsigned int & originalPage2Position,
         std::pair<unsigned int, unsigned int> originalPageNumbers, 
         const MergePageDescription & description,
         Object * basePage,
         const std::vector<Object *> & annots,
         std::vector <Object::ChildAndItPositionInContent> & annotsPositions
         );
      //members
      Object * _root;
      unsigned int _pageNumber;
      int _rotation;

   };
}
#endif 
