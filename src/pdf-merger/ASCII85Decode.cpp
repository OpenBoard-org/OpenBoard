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
#include <iostream>

#include "ASCII85Decode.h"

#include "core/memcheck.h"

using namespace merge_lib;
static const unsigned long pow85[] = {
   85*85*85*85, 85*85*85, 85*85, 85, 1
};

void ASCII85Decode::_wput(std::string &cur,unsigned long tuple, int len)
{
   switch (len) 
   {
   case 4:
      cur+=static_cast<char>(tuple >> 24);
      cur+=static_cast<char>(tuple >> 16);
      cur+=static_cast<char>(tuple >>  8);
      cur+=static_cast<char>(tuple);
      break;
   case 3:
      cur+=static_cast<char>(tuple >> 24);
      cur+=static_cast<char>(tuple >> 16);
      cur+=static_cast<char>(tuple >>  8);
      break;
   case 2:
      cur+=static_cast<char>(tuple >> 24);
      cur+=static_cast<char>(tuple >> 16);
      break;
   case 1:
      cur+=static_cast<char>(tuple >> 24);
      break;
   default:
      std::cerr<<"Asci85Filter - unexpected len = "<<len<<"\n";
      break;
   }
}

bool ASCII85Decode::decode(std::string &encoded)
{
   unsigned long tuple = 0;
   std::string decoded = "";
   int count = 0;
   int size = encoded.size();
   int i = 0;
   bool found = false;
   for(;size;)
   {
      char ch = encoded[i++];
      // sometimes <~ can present.
      switch(ch)
      {
      default:
         if( ch < '!' || ch > 'u' )
         {
            std::cerr<<"bag character in ascii85 block["<<ch<<"]\n";
            return false;
         }
         tuple += (unsigned long)(ch - '!') * pow85[count++];
         if( count == 5)
         {
            _wput(decoded,tuple,4);
            count = 0;
            tuple = 0;
         }
         break;
      case 'z':
         if( count != 0 )
         {
            std::cerr<<"Z inside of acii85 5-tuple!\n";
            return false;
         }
         decoded += "\0\0\0\0";
         break;
      case '~':
         if( --size )
         {
            ch = encoded[i++];
            if( ch == '>') 
            {
               if( count > 0 )
               {
                  count --;
                  tuple += pow85[count];
                  _wput(decoded,tuple,count);
               }
            }
            encoded = decoded;
            return true;
         }
         std::cerr<<"~ without > in ascii85 stream!\n = ["<<ch<<"]\n";
         encoded = decoded;
         return false;
         break;
      case '\n':
      case '\r':
      case '\t':
      case ' ':
      case '\0':
      case '\f':
      case '\b':
      case 0177:
         break;
      }
      --size;
   }
   return true;
}
