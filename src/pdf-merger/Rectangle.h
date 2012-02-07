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
      double x1, y1, x2, y2;
   private:
      //methods
      const std::string _getRectangleAsString(const char * delimeter);
      //members   
      const char * _rectangleName;
      TransformationMatrix _tm;
   };
}
#endif

