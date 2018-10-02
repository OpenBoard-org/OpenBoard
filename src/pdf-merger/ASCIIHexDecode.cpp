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




#include "ASCIIHexDecode.h"
#include <QtGlobal>
#include <string>
#include "Utils.h"

#include "core/memcheck.h"

using namespace merge_lib;

const std::string WHITESPACES(" \t\f\v\n\r");

#define HEX_TO_VAL(char_c) (char_c)>9?'A'+(char_c)-10:'0'+(char_c);

static unsigned int convertHexVal(unsigned char c)
{
   if(c >= '0' && c <= '9')
   {
      return (c - '0');
   }
   if(c >= 'A' && c <= 'F')
   {
      return (c - 'A' + 10);
   }
   if(c >= 'a' && c <= 'f')
   {
      return (c - 'a' + 10);
   }
   return 0;
}

bool ASCIIHexDecode::encode(std::string & decoded)
{
    Q_UNUSED(decoded);
    return false;
}

bool ASCIIHexDecode::decode(std::string & encoded)
{
   bool isLow = true;
   unsigned char decodedChar = '\0';
   int len = encoded.size();
   std::string decoded ="";
   for(int i = 0;i<len;i++)
   {
      unsigned char ch = encoded[i];
      if((int) WHITESPACES.find(ch) != -1 )
      {
         continue;
      }
      if( ch == '>' )
      {
         continue; // EOD found
      }
      ch = convertHexVal(ch);
      if( isLow ) 
      {
         decodedChar = (ch & 0x0F);
         isLow         = false;
      }
      else
      {
         decodedChar = ((decodedChar << 4) | ch);
         isLow         = true;
         decoded += decodedChar;                        
      }
   }
   encoded = decoded;
   return true;
}

void ASCIIHexDecode::initialize(Object * objectWithStram)
{
    Q_UNUSED(objectWithStram);
}
