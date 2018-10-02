/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#if !defined MergePageDescription_h
#define MergePageDescription_h

#include "Transformation.h"
#include <map>
#include <string>

namespace merge_lib
{
   struct MergePageDescription
   {
      //members:
      double outPageWidth;    // output page width
      double outPageHeight;   // output page height    
      unsigned int basePageNumber;  
      std::string baseDocumentName;
      TransformationDescription basePageTransformation;

      unsigned int overlayPageNumber;
      TransformationDescription overlayPageTransformation;

      bool skipOverlayPage;
      bool skipBasePage;

      //methods:
      //constructor
      MergePageDescription(double outputPageWidth, 
        double outputPageHeight, 
        unsigned int basePageNum,
        const char * baseDocName,
        const TransformationDescription & baseTrans,
        unsigned int overlayPageNum,
        const TransformationDescription & overlayTrans,
        bool omitOverlayPage = false,
        bool omitBasePage    = false
        ):
      outPageWidth(outputPageWidth),
        outPageHeight(outputPageHeight),
        basePageNumber(basePageNum),
        baseDocumentName(baseDocName), 
        basePageTransformation(baseTrans), 
        overlayPageNumber(overlayPageNum),
        overlayPageTransformation(overlayTrans),
        skipOverlayPage(omitOverlayPage),
        skipBasePage(omitBasePage)
      {
      }

      MergePageDescription(double outputPageWidth, 
        double outputPageHeight, 
        unsigned int basePageNum,
        const char * baseDocName,
        const TransformationDescription & baseTrans
        ):
      outPageWidth(outputPageWidth),
        outPageHeight(outputPageHeight),
        basePageNumber(basePageNum),
        baseDocumentName(baseDocName), 
        basePageTransformation(baseTrans), 
        overlayPageNumber(0),
        overlayPageTransformation(),
        skipOverlayPage(true),
        skipBasePage(false)
      {
      }

      MergePageDescription(const MergePageDescription & copy)
      {
        *this = copy;
      }
      MergePageDescription& operator = (const MergePageDescription &copy)
      {
        if( this != &copy )
        {
          baseDocumentName = copy.baseDocumentName;
          basePageNumber = copy.basePageNumber;   
          skipBasePage = copy.skipBasePage;
          skipOverlayPage = copy.skipOverlayPage;
          outPageHeight = copy.outPageHeight;
          outPageWidth = copy.outPageWidth;
          basePageTransformation = copy.basePageTransformation;
          overlayPageNumber = copy.overlayPageNumber;
          overlayPageTransformation = copy.overlayPageTransformation;
        }
        return *this;
      }

      ~MergePageDescription()
      {
      }
   };

   // array of merge descriptions - allows to merge selected pages
   typedef std::vector<MergePageDescription> MergeDescription;
}

#endif
