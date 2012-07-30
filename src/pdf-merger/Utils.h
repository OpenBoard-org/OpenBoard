/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
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
#if !defined Utils_h
#define Utils_h

#include "Config.h"

#include <stdlib.h>
#include <string>
#include <time.h>
#include <stdio.h>

namespace merge_lib
{
   class Utils
   {
   public:
      static int stringToInt(const std::string & str); //throw ConvertException
      static std::string uIntToStr(unsigned int integer);
      static std::string doubleToStr(double doubleValue);
      static double stringToDouble(const std::string & s );
      static bool doubleEquals(const double left,const double right, const double epsilon = +1.e-10);
      static double normalizeValue(double &val,const double epsilon = +1.e-10);
      static unsigned long getFileSize(const char * fileName);

   private:
      static int _stringToInt(const std::string & str); //throw ConvertException
      static int _stringToInt(std::string & str); //throw ConvertException
   };



}
#endif
