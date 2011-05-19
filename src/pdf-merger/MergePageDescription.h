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
