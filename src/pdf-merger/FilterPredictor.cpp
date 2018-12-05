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




#include "Config.h"
#include <iostream>
#include <map>
#include <QtGlobal>

#include "FilterPredictor.h"
#include "Utils.h"
#include "Object.h"
#include "Parser.h"

#include "core/memcheck.h"

using namespace merge_lib;

const std::string FilterPredictor::PREDICTOR_TOKEN = "/Predictor";

const std::string FilterPredictor::DECODE_PARAM_TOKEN = "/DecodeParms";
const std::string COLUMNS_TOKEN = "/Columns";
const std::string COLORS_TOKEN = "/Colors";
const std::string BITS_TOKEN = "/BitsPerComponent";
const std::string EARLY_TOKEN = "/EarlyChange";
const std::string DICT_START_TOKEN = "<<";
const std::string DICT_END_TOKEN = ">>";


FilterPredictor::FilterPredictor():
_predictor(1),
_colors(1),
_bits(8),
_columns(1),
_earlyChange(1)
{
}

FilterPredictor::~FilterPredictor()
{
}

 bool FilterPredictor::encode(std::string & decoded)
 {
     Q_UNUSED(decoded);
     return false;
 }

std::string FilterPredictor::getDictionaryContentStr(std::string & in, size_t &pos )
{
   size_t beg = in.find(DICT_START_TOKEN,pos);
   if((int) beg == -1 )
   {
      return "";
   }

   beg += DICT_START_TOKEN.size();
   size_t end = in.find(DICT_END_TOKEN,beg);
   if((int) end == -1 )
   {
      return "";
   }
   return in.substr(beg,end-beg);
}


void FilterPredictor::obtainDecodeParams(Object *objectWithStream, std::string &dictStr)
{
   typedef std::map< std::string, int> DecodeParams;

   DecodeParams params;
   // set some initiaial values
   params[PREDICTOR_TOKEN] = _predictor;
   params[COLUMNS_TOKEN] = _columns;
   params[COLORS_TOKEN] = _colors;
   params[BITS_TOKEN] = _bits;
   params[EARLY_TOKEN] = _earlyChange;

   // lets parse the content of dictionary and set actual values into the map

   DecodeParams::iterator it = params.begin();
   for(; it != params.end();it++)
   {
      size_t pos = dictStr.find((*it).first);
      if((int) pos != -1 )
      {
         pos += (*it).first.size();

         // we assume the following pattern "/Colors 8"
         std::string numstr = objectWithStream->getNameSimpleValue(dictStr,(*it).first);
         if( numstr.empty() )
         {
            std::cerr<<"Wrong value of "<<(*it).first<<"defined as "<<dictStr<<"\n";
         }
         int number = Utils::stringToInt(numstr);
         //trace("Object number with length = %d",number);
         params[(*it).first] = number;
      }
   }
   // refresh the values after reading
   _predictor = params[PREDICTOR_TOKEN];
   _columns = params[COLUMNS_TOKEN];
   _colors = params[COLORS_TOKEN];
   _bits = params[BITS_TOKEN];
   _earlyChange = params[EARLY_TOKEN];
}

void FilterPredictor::initialize(Object *objectWithStream)
{
   if( objectWithStream )
   {
      std::string content;
      objectWithStream->getHeader(content);
      // we need to parse the header of file to obtain the decoder parameter      
      size_t position = content.find(DECODE_PARAM_TOKEN);
      if((int) position != -1)
      {
         position += DECODE_PARAM_TOKEN.size();
         std::string dictStr = getDictionaryContentStr(content,position);
         //         trace_hex(dictStr.data(),dictStr.size());
         obtainDecodeParams(objectWithStream,dictStr);
      }
   }
}
//-----------------------------
// Function perorms decoding of one row of data.
//-----------------------------
bool FilterPredictor::decodeRow(const char *in,std::string & out,const std::string &prev,int curPrediction)
{
   std::string dec(_bytesPerPixel,'\0');
   dec.append(in,_rowLen); // the buffer to decode
   int start = _bytesPerPixel;
   int end = _bytesPerPixel + _rowLen;
   switch(curPrediction)
   {
   case 2: // TIFF predictor
      // to do, implement TIFF predictor
      std::cerr<<"TIFF predictor not yet implemented!\n";
      return false;

      break;

   case 1: 
   case 10: // PNG NONE prediction
      // nothing to do, take as is
      break;

   case 11: // PNG SUB on all raws
      for(int i = start;i<end;i++)
      {
         dec[i] += dec[ i - _bytesPerPixel ];
      }
      break;
   case 12: // PNG UP on all raws
      for(int i = start;i<end;i++)
      {
         dec[i] += prev[i];
      }
      break;
   case 13: // PNG average on all raws
      //Average(x) + floor((Raw(x-bpp)+Prior(x))/2)
      for(int i = start;i<end;i++)
      {
         int leftV  = int(dec[i - _bytesPerPixel])&0xFF;
         int   aboveV = int(prev[i - _bytesPerPixel])&0xFF;
         unsigned char average = (unsigned char)( (((leftV+aboveV)>>1)&0xFF));
         dec[i] += average;
      }
      break;
   case 14: //PNG PAETH on all rows
      /*function PaethPredictor (a, b, c)
      ; a = left, b = above, c = upper left
      p := a + b - c        ; initial estimate
      pa := abs(p - a)      ; distances to a, b, c
      pb := abs(p - b)
      pc := abs(p - c)
      ; return nearest of a,b,c,
      ; breaking ties in order a,b,c.
      if pa <= pb AND pa <= pc then return a
      else if pb <= pc then return b
      else return c
      Paeth(x) + PaethPredictor(Raw(x-bpp), Prior(x), Prior(x-bpp))
      */
      for(int i = start;i<end;i++)
      {
         int left = int( dec[i - _bytesPerPixel]) & 0xFF;
         int upperLeft = int( prev[i - _bytesPerPixel]) & 0xFF;

         int above = int( prev[i]) & 0xFF;
         int p = left + above - upperLeft;
         int pLeft = abs(p - left);
         int pAbove = abs(p - above);
         int pUpperLeft = abs(p - upperLeft);
         int paeth = 0;
         if( pLeft <= pAbove && pLeft <=pUpperLeft )
         {
            paeth = left;
         }
         else if( pAbove <= pUpperLeft )
         {
            paeth = above;
         }
         else 
         {
            paeth = upperLeft;
         }
         dec[i] += char (paeth & 0xFF);
      }
      break;
   default:
      break;

   }
   out = dec;
   return true;
}

// method performs prediction decoding

bool FilterPredictor::decode(std::string &content)
{
   bool isPNG = _predictor >= 10?true:false;
   int rowBits = _columns*_colors*_bits;
   _rowLen = (rowBits>>3) + (rowBits&7);
   _bytesPerPixel =  (_colors * _bits + 7) >> 3;
   int rows = 0;

   if( isPNG )
   {
      rows = content.size()/(_rowLen+1) + (content.size()% (_rowLen+1));
   }
   else
   {
      rows = content.size()/(_rowLen) + (content.size()% (_rowLen) );
   }

   int inSize = content.size();
   std::string  out = "";

   if( inSize%(isPNG?_rowLen+1:_rowLen) != 0 )
   {
      std::cerr<<"Warning : wrong PNG identation inSize "<<inSize<<" rowLen = "<<_rowLen<<" isPNG = "<<isPNG<<"\n";
      content = out;
      return false;
   }

   const char *curRow  = NULL;
   std::string prev(_bytesPerPixel+_rowLen,'\0');  //"previous" line
   int curPredictor  = 1;

   for(int i = 0;i<rows;i++)
   {
      curRow = content.data() + (i* (_rowLen + (isPNG?1:0)) );            
      if( isPNG )
      {
         // this is PNG predictor!
         curPredictor = *curRow++;
         curPredictor +=10;         
      }
      else
      {
         curPredictor = _predictor; // default NONE predictor
      }
      std::string dec;
      if( !decodeRow(curRow,dec,prev,curPredictor) )
      {
         std::cerr<<"Unable to process prediction"<<curPredictor<<"!\n";
         content = out;
         return false;
      }
      //trace_hex(dec.data()+_bytesPerPixel,_rowLen);
      prev = dec;
      out += std::string(dec.data()+_bytesPerPixel,_rowLen);
   }
   content = out;
   return true;
}
