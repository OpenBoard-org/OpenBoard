#if !defined Rectangle_h
#define Rectangle_h

#include "Transformation.h"

#include <vector>
#include <map>


namespace merge_lib
{
   class Object;

   class Rectangle
   {
   public:
      Rectangle(const char * rectangleName);

      Rectangle(const char * rectangleName, const std::string content);
      void appendRectangleToString(std::string & content, const char * delimeter);
      void updateRectangle(Object * objectWithRectangle, const char * delimeter);
      void setNewRectangleName(const char * newName);

      void recalculateInternalRectangleCoordinates(const PageTransformations & transformations);
      double getWidth();
      double getHeight();

      //members
      double x1, x2, y1, y2;
   private:
      //methods
      const std::string _getRectangleAsString(const char * delimeter);
      //members   
      const char * _rectangleName;
      TransformationMatrix _tm;
   };
}
#endif

