///////////////////////////////////////////////////////////
//  Page.h
//  Implementation of the Class Page
//  Created on:      19-θών-2009 12:27:56
///////////////////////////////////////////////////////////

#if !defined(EA_FF53E894_11D7_4c64_A409_DBC97C9EF3CF__INCLUDED_)
#define EA_FF53E894_11D7_4c64_A409_DBC97C9EF3CF__INCLUDED_

#include "Object.h"

#include <string>

class PageParser
{
friend class Parser;

public:
   PageParser(const std::string & pageContent);
   ~PageParser();
   unsigned int getPageNumber()
   {
      return _pageNumber;
   }
   void merge(const Page & sourcePage);

   void recalculateObjectNumbers(unsigned int & newNumber);
   std::string & getPageContent();
   const std::map <unsigned int, Object *> &  getPageRefs();

private:
   //methods


   //members
   Object * _root;
   unsigned int _pageNumber;

};
#endif // !defined(EA_FF53E894_11D7_4c64_A409_DBC97C9EF3CF__INCLUDED_)

