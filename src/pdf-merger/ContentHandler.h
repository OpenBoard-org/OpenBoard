/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
#if !defined ContentHandler_h
#define ContentHandler_h

#include "PageElementHandler.h"

namespace merge_lib
{
   //this class is needed to process "Content" field of Page object
   class ContentHandler: public PageElementHandler
   {
   public:
      ContentHandler(Object * page, const std::string & handlerName): 
                PageElementHandler(page)
                     
      {
         _setHandlerName(handlerName);
      }
      virtual ~ContentHandler(){};

      
   private:
      //methods

      //concatedate stream of all objects which contains Content of Page
      void _processObjectContent(unsigned int startOfPageElement);

      //write concatenated stream to Page object
      void _changeObjectContent(unsigned int startOfPageElement);
      //get content of stream 
      // object - object with stream
      //leftBound - left bound of object's content
      //rightBound - right bound of object's content
      std::string _retrieveStreamContent(Object * object, unsigned int leftBound, unsigned int rightBound);

      //get stream from Arrey elemetns
      std::string _getStreamFromReferencies(Object * objectWithArray, unsigned int leftBound, unsigned int rightBound);

      //get stream from Object
      std::string _getStreamFromContent(Object * objectWithStream);

      //memebers
      std::string _concatenatedStream;
   };
}
#endif

