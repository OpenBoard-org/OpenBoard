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
#include "Rectangle.h"
#include "Utils.h"
#include "Object.h"
#include "Parser.h"

#include <iostream>

using namespace merge_lib;

Rectangle::Rectangle(const char * rectangleName): 
_rectangleName(rectangleName), 
x1(0),
y1(0),
x2(0),
y2(0),
_tm()

{}

Rectangle::Rectangle(const char * rectangleName, const std::string content): 
_rectangleName(rectangleName), 
x1(0),
y1(0),
x2(0),
y2(0)
{
   unsigned int rectanglePosition = Parser::findToken(content,rectangleName);
   
   if( rectanglePosition == std::string::npos )
   {
      std::cerr<<"Unable to find rectangle name  "<<rectangleName<<" in content\n";
   }
   size_t beg = content.find("[",rectanglePosition);
   size_t end = content.find("]",rectanglePosition);

   if( beg != std::string::npos && end != std::string::npos )
   {
      std::string arr = content.substr(beg+1,end-beg-1);
      std::stringstream in;
      in<<arr;
      in>>x1>>y1>>x2>>y2;
   }
}
void Rectangle::appendRectangleToString(std::string & content, const char * delimeter)
{   
   content.append(_getRectangleAsString(delimeter));      
}

const std::string Rectangle::_getRectangleAsString(const char * delimeter)
{
   std::string result(_rectangleName);    
   result.append(" [");
   result.append(Utils::doubleToStr(x1));
   result.append(delimeter);
   result.append(Utils::doubleToStr(y1));
   result.append(delimeter);
   result.append(Utils::doubleToStr(x2));
   result.append(delimeter);
   result.append(Utils::doubleToStr(y2));
   result.append(" ]\n");
   return result;
}

void Rectangle::setNewRectangleName(const char * newName)
{
   _rectangleName = newName;
}

void Rectangle::recalculateInternalRectangleCoordinates(const PageTransformations & transformations)
{   
   TransformationMatrix tempTm;
   for(size_t i = 0; i < transformations.size(); ++i)
   {
      tempTm = transformations[i]->getMatrix();
      tempTm.add(_tm);
      _tm = tempTm;
   }
   _tm.recalculateCoordinates(x1, y1);
   _tm.recalculateCoordinates(x2, y2);
}

void Rectangle::updateRectangle(Object * objectWithRectangle, const char * delimeter)
{
   Object * foundObjectWithRectangle;
   unsigned int fake;
   objectWithRectangle->findObject(std::string(_rectangleName), foundObjectWithRectangle, fake);
   std::string objectContent = foundObjectWithRectangle->getObjectContent();
   unsigned int rectanglePosition = objectContent.find(_rectangleName);
   unsigned int endOfRectangle = objectContent.find("]", rectanglePosition) + 1;
   foundObjectWithRectangle->eraseContent(rectanglePosition, endOfRectangle - rectanglePosition);
   foundObjectWithRectangle->insertToContent(rectanglePosition, _getRectangleAsString(delimeter));

   // reread the objectContent, since it was changed just above;
   objectContent = foundObjectWithRectangle->getObjectContent();

   //update matrix
   unsigned int startOfAP = Parser::findToken(objectContent,"/AP");
   unsigned int endOfAP = objectContent.find(">>", startOfAP);
   std::vector<Object *>  aps = foundObjectWithRectangle->getChildrenByBounds(startOfAP, endOfAP);
   for(size_t i = 0; i < aps.size(); ++i)
   {
      Object * objectWithMatrix = aps[i];

      std::string objectContent = objectWithMatrix->getObjectContent();      
      unsigned int matrixPosition = Parser::findToken(objectContent,"/Matrix");
      if(matrixPosition == std::string::npos)
         continue;
      unsigned int matrixValueLeftBound = objectContent.find("[", matrixPosition);
      unsigned int matrixValueRightBound = objectContent.find("]", matrixValueLeftBound) + 1;
      objectWithMatrix->eraseContent(matrixValueLeftBound, matrixValueRightBound - matrixValueLeftBound);
      objectWithMatrix->insertToContent(matrixValueLeftBound, _tm.getValue());

   }
}

double Rectangle::getWidth()
{
   return x2 - x1;
}

double Rectangle::getHeight()
{
   return y2 - y1;
}
