/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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
std::map<std::string, Decoder *> Filter::_allDecoders;

Filter::~Filter()
{
   std::map<std::string, Decoder *>::iterator it = _allDecoders.begin();
   for(; it != _allDecoders.end(); ++it)
   {
      delete (*it).second;
   }
   _allDecoders.clear();
}

//replace coded stream with decoded
void Filter::getDecodedStream(std::string & stream)
{
   std::vector <Decoder * >  decoders = _getDecoders();
   _objectWithStream->getStream(stream);
   for(size_t i = 0; i < decoders.size(); ++i)
   {
      decoders[i]->decode(stream);
   }
}

//parse object's content and fill out vector with
//necessary decoders
std::vector <Decoder * > Filter::_getDecoders()
{
   std::string streamHeader;
   static std::string whitespacesAndDelimeters(" \t\f\v\n\r<<>>]/");
   _objectWithStream->getHeader(streamHeader);
   unsigned int filterPosition = streamHeader.find("/Filter"); 
   std::vector <Decoder * > result;   
   unsigned int startOfDecoder = filterPosition + 1;
   while(1)
   {      
      startOfDecoder = streamHeader.find("/", startOfDecoder);
      if(startOfDecoder == std::string::npos)
         break;
      else
         ++startOfDecoder;
      unsigned int endOfDecoder = streamHeader.find_first_of(whitespacesAndDelimeters, startOfDecoder);
      if(endOfDecoder == std::string::npos)
         break;
      std::map<std::string, Decoder *>::iterator foundDecoder = 
         _allDecoders.find(streamHeader.substr(startOfDecoder, endOfDecoder - startOfDecoder));
      if(foundDecoder == _allDecoders.end())
         break;
      Decoder * decoder = foundDecoder->second;
      decoder->initialize(_objectWithStream);
      result.push_back(decoder);
   }
   return result;


}

void Filter::_createAllDecodersSet()
{

   if(!_allDecoders.empty())
      return;
   _allDecoders.insert(std::pair<std::string, Decoder *> (std::string("ASCIIHexDecode"), new ASCIIHexDecode()));
   _allDecoders.insert(std::pair<std::string, Decoder *> (std::string("ASCII85Decode"), new ASCII85Decode()));   
   _allDecoders.insert(std::pair<std::string, Decoder *> (std::string("LZWDecode"), new LZWDecode()));   
   _allDecoders.insert(std::pair<std::string, Decoder *> (std::string("FlateDecode"), new FlateDecode()));   
   _allDecoders.insert(std::pair<std::string, Decoder *> (std::string("RunLengthDecode"), new RunLengthDecode()));   
   _allDecoders.insert(std::pair<std::string, Decoder *> (std::string("CCITTFaxDecode"), new CCITTFaxDecode()));   
   _allDecoders.insert(std::pair<std::string, Decoder *> (std::string("JBIG2Decode"), new JBIG2Decode())); 
   _allDecoders.insert(std::pair<std::string, Decoder *> (std::string("DCTDecode"), new DCTDecode())); 
}


