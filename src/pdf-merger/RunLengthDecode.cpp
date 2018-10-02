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




#include "RunLengthDecode.h"
#include <QtGlobal>
#include "core/memcheck.h"

using namespace merge_lib;

bool RunLengthDecode::encode(std::string & decoded)
{
    Q_UNUSED(decoded);
    return false;
}

/* The encoded data is a sequence of
runs, where each run consists of a length byte followed by 1 to 128 bytes of data. If
the length byte is in the range 0 to 127, the following length + 1 (1 to 128) bytes
are copied literally during decompression. If length is in the range 129 to 255, the
following single byte is to be copied 257 - length (2 to 128) times during decom-
pression. A length value of 128 denotes EOD.  */


// Function performs RunLength Decoder for PDF, very simple
bool RunLengthDecode::decode(std::string & encoded)
{
   std::string decoded;

   for(unsigned enci = 0;enci < encoded.size();)
   {
      unsigned char c = encoded[enci++];
      if( c == 128 )
      {
         break; //EOD
      }
      else if( c < 128 )
      {
         for(int j = 0; j < (c+1);j++)
         {
            decoded.append(1,encoded[enci]);
         }
         enci++;
      }
      else 
      {
         for(int j = 0; j < (257 - c);j++)
         {
            decoded.append(1,encoded[enci]);
         }
         enci++;
      }
   }
   return true;
}

void RunLengthDecode::initialize(Object * objectWithStream)
{
    Q_UNUSED(objectWithStream);
};
