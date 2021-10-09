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




#include "Filter.h"
#include "Object.h"
#include "Decoder.h"
#include "ASCIIHexDecode.h"
#include "ASCII85Decode.h"
#include "LZWDecode.h"
#include "FlateDecode.h"
#include "RunLengthDecode.h"
#include "CCITTFaxDecode.h"
#include "JBIG2Decode.h"
#include "DCTDecode.h"

#include "core/memcheck.h"

using namespace merge_lib;
std::map<std::string, Decoder *> Filter::s_allDecoders;

Filter::~Filter()
{
   std::map<std::string, Decoder *>::iterator it = s_allDecoders.begin();
   for(; it != s_allDecoders.end(); ++it)
   {
      delete (*it).second;
   }
   s_allDecoders.clear();
}

//replace coded stream with decoded
void Filter::getDecodedStream(std::string & stream)
{
   std::vector <Decoder * >  decoders = getDecoders();
   m_objectWithStream->getStream(stream);
   for(size_t i = 0; i < decoders.size(); ++i)
   {
      decoders[i]->decode(stream);
   }
}

//parse object's content and fill out vector with
//necessary decoders
std::vector <Decoder * > Filter::getDecoders()
{
   std::string streamHeader;
   static std::string whitespacesAndDelimeters(" \t\f\v\n\r<<>>]/");
   m_objectWithStream->getHeader(streamHeader);
   unsigned int filterPosition = streamHeader.find("/Filter"); 
   std::vector <Decoder * > result;   
   unsigned int startOfDecoder = filterPosition + 1;
   while(1)
   {      
      startOfDecoder = streamHeader.find("/", startOfDecoder);
      if((int)startOfDecoder == -1)
         break;
      else
         ++startOfDecoder;
      unsigned int endOfDecoder = streamHeader.find_first_of(whitespacesAndDelimeters, startOfDecoder);
      if((int)endOfDecoder == -1)
         break;
      std::map<std::string, Decoder *>::iterator foundDecoder = 
         s_allDecoders.find(streamHeader.substr(startOfDecoder, endOfDecoder - startOfDecoder));
      if(foundDecoder == s_allDecoders.end())
         break;
      Decoder * decoder = foundDecoder->second;
      decoder->initialize(m_objectWithStream);
      result.push_back(decoder);
   }
   return result;


}

void Filter::createAllDecodersSet()
{

   if(!s_allDecoders.empty())
      return;
   s_allDecoders.insert(std::pair<std::string, Decoder *> (std::string("ASCIIHexDecode"), new ASCIIHexDecode()));
   s_allDecoders.insert(std::pair<std::string, Decoder *> (std::string("ASCII85Decode"), new ASCII85Decode()));   
   s_allDecoders.insert(std::pair<std::string, Decoder *> (std::string("LZWDecode"), new LZWDecode()));   
   s_allDecoders.insert(std::pair<std::string, Decoder *> (std::string("FlateDecode"), new FlateDecode()));   
   s_allDecoders.insert(std::pair<std::string, Decoder *> (std::string("RunLengthDecode"), new RunLengthDecode()));   
   s_allDecoders.insert(std::pair<std::string, Decoder *> (std::string("CCITTFaxDecode"), new CCITTFaxDecode()));   
   s_allDecoders.insert(std::pair<std::string, Decoder *> (std::string("JBIG2Decode"), new JBIG2Decode())); 
   s_allDecoders.insert(std::pair<std::string, Decoder *> (std::string("DCTDecode"), new DCTDecode())); 
}


