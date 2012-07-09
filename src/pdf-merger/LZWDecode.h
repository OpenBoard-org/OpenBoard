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
#ifndef LZWDecode_H
#define LZWDecode_H

#include <string>
#include "Decoder.h"
#include "FilterPredictor.h"

namespace merge_lib
{
   // this class provides method for FlateDecode encoding and decoding
   class LZWDecode : public Decoder
   {
   public:
      LZWDecode();
      virtual ~LZWDecode();
      bool encode(std::string & decoded);
      bool decode(std::string & encoded);
      void initialize(Object * objectWithStram);
   private:

      FilterPredictor *_predict;

      void clearTable();
      int getCode();

      std::string _dummy;
      std::string &_encoded;
      size_t _curSymbolIndex;

      int _earlyChange;	// early parameter
      int _readBuf;	   
      int _readBits;	   
      int _nextCode;     
      int _bitsToRead;     
      bool _first;	      
      int _curSequenceLength;    
   };
}

#endif // LZW_DECODE_H_INCLUDED

