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
