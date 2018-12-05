/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#include <iostream>
#include <QtGlobal>
#include "LZWDecode.h"
#include "FilterPredictor.h"

#include "core/memcheck.h"

// method performs decoding
using namespace merge_lib;

LZWDecode::LZWDecode():
      _predict(NULL),
      _dummy(""),
      _encoded(_dummy),
      _curSymbolIndex(0),
      _earlyChange(1),
      _readBuf(0),
      _readBits(0),
      _nextCode(0),
      _bitsToRead(0),
      _first(true),
      _curSequenceLength(0)

{
   clearTable();
}
LZWDecode::~LZWDecode()
{
   if( _predict ) 
   {
      delete _predict;
   }
}

bool LZWDecode::encode(std::string & decoded)
{
    Q_UNUSED(decoded);
    return true;
}

void LZWDecode::initialize(Object * objectWithStream)
{
   if( objectWithStream )
   {
      std::string head;
      objectWithStream->getHeader(head);

      if((int) head.find(FilterPredictor::DECODE_PARAM_TOKEN)  != -1 )
      {
         _predict = new FilterPredictor();
         _predict->initialize(objectWithStream);
         _earlyChange = _predict->getEarlyChange();
      }
      _readBits = 0;
      _readBuf = 0;
      clearTable();
   }
}

void LZWDecode::clearTable() 
{
   _nextCode = 258;
   _bitsToRead = 9;
   _curSequenceLength = 0;
   _first = true;
}

int LZWDecode::getCode() 
{
   int c = 0;
   int code = 0;

   while (_readBits < _bitsToRead) 
   {
      if( _curSymbolIndex < _encoded.size() )
      {
         c = _encoded[_curSymbolIndex++];
      }
      else
      {
         return EOF;
      }
      _readBuf = (_readBuf << 8) | (c & 0xff);
      _readBits += 8;
   }
   code = (_readBuf >> (_readBits - _bitsToRead)) & ((1 << _bitsToRead) - 1);
   _readBits -= _bitsToRead;
   return code;
}

// Method performs LZW decoding
bool LZWDecode::decode(std::string & encoded)
{
   _curSymbolIndex = 0;
   _encoded = encoded;

   // LZW decoding
   std::string decoded;
   struct DecodingTable
   {    
      int length;
      int head;
      unsigned tail;
   } decTable[4097];

   int prevCode = 0;     
   int newChar = 0;      
   unsigned curSequence[4097];    
   int nextLength = 0;

   clearTable();
   while(1)
   {
      int code = getCode();
      if( code == EOF || code == 257 )
      {
         // finish
         break;
      }
      if( code == 256 )
      {
         clearTable();
         continue;
      }
      if( _nextCode >= 4997 )
      {
         std::cout<<"Bad LZW stream - unexpected clearTable\n";
         clearTable();
         continue;
      }
      nextLength = _curSequenceLength + 1;
      if( code < 256 )
      {
         curSequence[ 0 ] = code;
         _curSequenceLength = 1;
      }
      else if( code < _nextCode )
      {
         //lets take sequence from table
         _curSequenceLength = decTable[code].length;
         int j = code;
         for( int i = _curSequenceLength - 1; i > 0; i--)
         {
            curSequence[ i ] = decTable[j].tail;
            j = decTable[ j ].head;
         }
         curSequence[0] = j;
      }
      else if( code == _nextCode )
      {
        curSequence[ _curSequenceLength ] = newChar;  
         ++_curSequenceLength;
      }
      else
      {
         std::cout<<"Bad LZW stream - unexpected code "<<code<<"\n";
         break;
      }
      newChar = curSequence[0];      
      if( _first ) 
      {
         _first = false;
      }
      else
      {
         // lets build decoding table
         decTable[ _nextCode ].length = nextLength;
         decTable[ _nextCode ].head = prevCode;
         decTable[ _nextCode ].tail = newChar;
         ++ _nextCode;
         // processing of PDF LZW parameter
         if (_nextCode + _earlyChange == 512)
         {
            _bitsToRead = 10;
         }
         else if (_nextCode + _earlyChange == 1024)
         {
            _bitsToRead = 11;
         }
         else if (_nextCode + _earlyChange == 2048)
         {
            _bitsToRead = 12;
         }
      }
      prevCode = code;
      // put current sequence to output stream
      for(int i = 0;i < _curSequenceLength;i++)
      {
         decoded += (char)curSequence[ i ];
      }
   }
   encoded = decoded;

   // if predictor exists for that object, then lets decode it
   if( _predict )
   {
      _predict->decode(encoded);
   }
   return true;    
}



