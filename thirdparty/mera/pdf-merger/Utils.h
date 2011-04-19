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
