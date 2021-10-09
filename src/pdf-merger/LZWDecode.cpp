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
      m_predict(NULL),
      m_dummy(""),
      m_encoded(m_dummy),
      m_curSymbolIndex(0),
      m_earlyChange(1),
      m_readBuf(0),
      m_readBits(0),
      m_nextCode(0),
      m_bitsToRead(0),
      m_first(true),
      m_curSequenceLength(0)

{
   clearTable();
}
LZWDecode::~LZWDecode()
{
   if( m_predict ) 
   {
      delete m_predict;
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
         m_predict = new FilterPredictor();
         m_predict->initialize(objectWithStream);
         m_earlyChange = m_predict->getEarlyChange();
      }
      m_readBits = 0;
      m_readBuf = 0;
      clearTable();
   }
}

void LZWDecode::clearTable() 
{
   m_nextCode = 258;
   m_bitsToRead = 9;
   m_curSequenceLength = 0;
   m_first = true;
}

int LZWDecode::getCode() 
{
   int c = 0;
   int code = 0;

   while (m_readBits < m_bitsToRead) 
   {
      if( m_curSymbolIndex < m_encoded.size() )
      {
         c = m_encoded[m_curSymbolIndex++];
      }
      else
      {
         return EOF;
      }
      m_readBuf = (m_readBuf << 8) | (c & 0xff);
      m_readBits += 8;
   }
   code = (m_readBuf >> (m_readBits - m_bitsToRead)) & ((1 << m_bitsToRead) - 1);
   m_readBits -= m_bitsToRead;
   return code;
}

// Method performs LZW decoding
bool LZWDecode::decode(std::string & encoded)
{
   m_curSymbolIndex = 0;
   m_encoded = encoded;

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
      if( m_nextCode >= 4997 )
      {
         std::cout<<"Bad LZW stream - unexpected clearTable\n";
         clearTable();
         continue;
      }
      nextLength = m_curSequenceLength + 1;
      if( code < 256 )
      {
         curSequence[ 0 ] = code;
         m_curSequenceLength = 1;
      }
      else if( code < m_nextCode )
      {
         //lets take sequence from table
         m_curSequenceLength = decTable[code].length;
         int j = code;
         for( int i = m_curSequenceLength - 1; i > 0; i--)
         {
            curSequence[ i ] = decTable[j].tail;
            j = decTable[ j ].head;
         }
         curSequence[0] = j;
      }
      else if( code == m_nextCode )
      {
        curSequence[ m_curSequenceLength ] = newChar;  
         ++m_curSequenceLength;
      }
      else
      {
         std::cout<<"Bad LZW stream - unexpected code "<<code<<"\n";
         break;
      }
      newChar = curSequence[0];      
      if( m_first ) 
      {
         m_first = false;
      }
      else
      {
         // lets build decoding table
         decTable[ m_nextCode ].length = nextLength;
         decTable[ m_nextCode ].head = prevCode;
         decTable[ m_nextCode ].tail = newChar;
         ++ m_nextCode;
         // processing of PDF LZW parameter
         if (m_nextCode + m_earlyChange == 512)
         {
            m_bitsToRead = 10;
         }
         else if (m_nextCode + m_earlyChange == 1024)
         {
            m_bitsToRead = 11;
         }
         else if (m_nextCode + m_earlyChange == 2048)
         {
            m_bitsToRead = 12;
         }
      }
      prevCode = code;
      // put current sequence to output stream
      for(int i = 0;i < m_curSequenceLength;i++)
      {
         decoded += (char)curSequence[ i ];
      }
   }
   encoded = decoded;

   // if predictor exists for that object, then lets decode it
   if( m_predict )
   {
      m_predict->decode(encoded);
   }
   return true;    
}



