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




#include <QtGlobal>
#include <QString>
#include "Config.h"
#include "Utils.h"
#include "Exception.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <fstream>
#include <string.h>

#include "core/memcheck.h"

using namespace merge_lib;

int Utils::stringToInt(const std::string & str) //throw ConvertException
{
   //skip zeros
   unsigned int lastZero = 0;str.find_last_of("0");
   while(str[lastZero++] == '0')
   {
      if(lastZero == str.size())
      {
         return 0;
      }
   }
   //if lastZero = 1, then 0 is not first symbol
   lastZero--;
   if((str.size() > 1) && (lastZero != 0))
   {
      //all number is zero, for ex. 00000000
      std::string copy = str;
      const std::string & cutedStr = copy.erase(0, lastZero);
      return _stringToInt(cutedStr) ;
   }
   else
   {
      return _stringToInt(str);
   }
}

double Utils::stringToDouble(const std::string & s )
{
   std::istringstream i(s);
   double x;
   if (!(i >> x))
      //TODO or throw exception? Think about!
      return 0;
   return x;
} 

std::string Utils::uIntToStr(unsigned int integer)
{
    return std::string(QString::number(integer).toLatin1());
}

std::string Utils::doubleToStr(double doubleValue)
{
    return std::string(QString::number(doubleValue).toLatin1());
}

int Utils::_stringToInt(const std::string & str) //throw ConvertException
{
   int intValue = atoi(str.c_str());

   if(((intValue == 0) && (str.size() > 1)) || // for ex string = xxx and integer = 0
      ((intValue == 0) && (str[0] != '0')))
   {
      throw Exception("Internal error");
   }


   if((intValue != 0) && (static_cast<unsigned int>(std::log10(static_cast<double>(intValue))) + 1) != str.size())         //for ex. string = 5x  and integer = 5)
   {
      throw Exception("Internal error");
   }
   return intValue;
}
bool Utils::doubleEquals(const double left, const double right, const double epsilon)
{
   return ( fabs (left - right) < epsilon);
}

double Utils::normalizeValue(double &val, const double epsilon )
{
    Q_UNUSED(epsilon);
   if( Utils::doubleEquals(val,0))
   {
      val = 0;
   }
   return val;
}

unsigned long Utils::getFileSize(const char * fileName)
{
   std::ifstream pdfFile;
   pdfFile.open (fileName, std::ios::binary );
   if (pdfFile.fail())
   {
      std::stringstream errorMessage("File ");
      errorMessage << fileName << " is absent" << "\0";
      throw Exception(errorMessage);
   }
   // get length of file:
   pdfFile.seekg (0, std::ios::end);
   unsigned long length = pdfFile.tellg();
   pdfFile.close();
   return length;
}

#ifdef DEBUG_VERBOSE

#define TO_HEX_CHAR(char_c) (char_c)>9?'A'+(char_c)-10:'0'+(char_c);

static void hex_dump_one_line(int         numberChars_n,
                              const char *input_p,
                              char       *output_p)
{
   int  i;
   char* to_hex_p;
   char* to_char_p;
   char char_c;

   /* Set pointer to the begining of hexadecimal area */
   to_hex_p=output_p;

   /* Set pointer to the begining of textual area */
   to_char_p=output_p+56;

   /* Write spaces between hexadecimal and textual areas */
   memset(output_p+50,' ',6);

   /* some kind of delimeter */
   *(output_p+53) = '#';

   /* Print out the hex area */
   for (i = 0 ; i < 16 ; i++)
   {
      /* Two spaces beetwen "four columns" */
      if (!(i&3))
      {
         *to_hex_p++=' ';
      }

      /* One space between columns */
      *to_hex_p++=' ';

      if (i < numberChars_n)
      {
         /* Print out byte in hexadecimal form */
         *to_hex_p++=TO_HEX_CHAR((input_p[i]>>4)&0xF);
         *to_hex_p++=TO_HEX_CHAR(input_p[i]&0xF);

         /* Output the char */
         char_c = input_p[i]&0xFF;

         if ( char_c<0x20 || char_c>0x7E )
         {
            char_c = '.';
         }
         *to_char_p=char_c;
         to_char_p++;
      }
      else
      {
         *to_hex_p++=' ';
         *to_hex_p++=' ';
         *to_char_p++=' ';
      }
   } /* for */
}

void trace_buffer(const void *buf, int len)
{
   char dump[160];
   int line_n = len/16;
   int rest_n = len- line_n*16;
   int i;
   memset(dump,0,160);
   printf("  length:%d\n",len);
   for (i = 0;i<line_n;i++)
   {
      hex_dump_one_line(16,
         (char*)buf+(i*16),
         dump);
      printf("%s\n",dump);;
   }
   if ( rest_n)
   {
      hex_dump_one_line(rest_n,
         (char*)buf+(line_n*16),
         dump);
      printf("%s\n",dump);
   }
}
#endif
