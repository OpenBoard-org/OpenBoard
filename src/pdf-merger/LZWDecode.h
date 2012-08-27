/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
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

