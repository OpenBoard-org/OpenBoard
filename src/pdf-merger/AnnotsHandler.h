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
#if !defined AnnotsHandler_h
#define AnnotsHandler_h

#include "RemoveHimSelfHandler.h"
//this class is needed to process "Annots" field
//AnnotsHandler parses Annots field of Page object and fills
//annots container with Annots objects
namespace merge_lib
{
   class AnnotsHandler: public RemoveHimselfHandler
   {
   public:
      AnnotsHandler(Object * page, const std::string & handlerName, std::vector<Object *> & annots): 
                        RemoveHimselfHandler(page, handlerName),
                        _annotations(annots)

      {
        _setHandlerName(handlerName);
      }

      
   private:
      //methods
      void _processObjectContent(unsigned int startOfPageElement);
       

      //memebers
      std::vector<Object *> & _annotations;
      
   };
}
#endif

