/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
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

