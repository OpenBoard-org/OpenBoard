#include "RemoveHimSelfHandler.h"

using namespace merge_lib;

void RemoveHimselfHandler::_changeObjectContent(unsigned int startOfPageElement)
{
   unsigned int endOfElement = _findEndOfElementContent(startOfPageElement);
   _page->forgetAboutChildren(startOfPageElement, endOfElement);
   _page->eraseContent(startOfPageElement, endOfElement - startOfPageElement);
}




