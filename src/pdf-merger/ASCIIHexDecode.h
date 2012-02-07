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

#ifndef ASCIIHexDecode_h
#define ASCIIHexDecode_h

#include <string>
#include "Decoder.h"
namespace merge_lib
{
   // this class provides method for ASCIIHEX encoding and decoding
   class ASCIIHexDecode : public Decoder
   {
      public:
         ASCIIHexDecode(){};
         virtual ~ASCIIHexDecode(){};
         bool encode(std::string & decoded);
         bool decode(std::string & encoded);
         void initialize(Object * objectWithStram);

   };
}
#endif // FLATEDECODE_H_INCLUDED

