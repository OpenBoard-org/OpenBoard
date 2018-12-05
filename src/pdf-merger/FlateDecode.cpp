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

#include "FlateDecode.h"
#include "zlib.h"
#include "Utils.h"
#include <string.h>

#include "core/memcheck.h"

using namespace merge_lib;
#define ZLIB_MEM_DELTA 65535
#define ZLIB_CHECK_ERR(err,msg) \
   if( err != Z_OK) {\
   std::cout<<msg<<" ZLIB error:"<<err<<std::endl; \
   }\

FlateDecode::FlateDecode():_predict(NULL)
{
}

FlateDecode::~FlateDecode()
{
   if( _predict )
   {
      delete _predict;
   }
}

void FlateDecode::initialize(Object * objectWithStream)
{
   if( objectWithStream )
   {
      std::string head;
      objectWithStream->getHeader(head);

      if((int) head.find(FilterPredictor::DECODE_PARAM_TOKEN)  != -1 )
      {
         _predict = new FilterPredictor();
         _predict->initialize(objectWithStream);
      }

   }
}

/** @brief encode
*
* @todo:
document this function
*/
bool FlateDecode::encode(std::string &decoded)
{   
   z_stream stream;

   stream.zalloc = (alloc_func)0;
   stream.zfree = (free_func)0;
   stream.opaque = (voidpf)0;

   size_t out_len = 0;
   unsigned char *out_p = NULL;

   stream.next_out = out_p;
   stream.avail_out = (uInt)out_len;

   stream.next_in = (unsigned char*)decoded.c_str();
   stream.avail_in = (uInt)decoded.size();

   int err = deflateInit(&stream, Z_DEFAULT_COMPRESSION);
   ZLIB_CHECK_ERR(err, "deflateInit");
   if ( err != Z_OK )
   {
      return false;
   }
   bool toContinue = false;
   int flush = Z_NO_FLUSH;
   do
   {
      toContinue = false;
      flush = (stream.avail_in == 0)?Z_FINISH:Z_NO_FLUSH;

      if ( !stream.avail_out )
      {
         // increase the space
         out_p = (unsigned char*)realloc(out_p,out_len + ZLIB_MEM_DELTA);

         // init new memory
         unsigned char *new_out_start = out_p + out_len;
         memset(new_out_start,0,ZLIB_MEM_DELTA);

         // Point next_out to the next unused byte
         stream.next_out = new_out_start;

         // Update the size of the buffer
         stream.avail_out = (uInt)ZLIB_MEM_DELTA;
         out_len += ZLIB_MEM_DELTA;
      }

      err = deflate(&stream,flush);
      if ( err == Z_OK && stream.avail_out == 0 )
      {
         toContinue = true;
      }

   }
   while ( toContinue || flush == Z_NO_FLUSH );

   err = deflateEnd(&stream);
   ZLIB_CHECK_ERR(err, "deflateEnd");
   if( err != Z_OK )
   {
      free(out_p);
      return false;
   }

   decoded = std::string((char*)out_p,stream.total_out);
   free(out_p);
   return true;
}

/** @brief decode
*
* @todo: document this function
*/
bool FlateDecode::decode(std::string & encoded)
{
   z_stream stream;

   //some initialization of ZLIB stuff
   stream.zalloc = (alloc_func)0;
   stream.zfree = (free_func)0;
   stream.opaque = (voidpf)0;

   //trace_hex((char*)encoded.c_str(),encoded.size());

   stream.next_in  = (unsigned char*)encoded.c_str();
   stream.avail_in = (uInt)encoded.size();

   int err = inflateInit(&stream);
   ZLIB_CHECK_ERR(err,"InflateInit");
   if ( err != Z_OK )
   {
      return false;
   }
   unsigned char *out_p = NULL;
   int out_len = 0;

   stream.next_out = out_p;
   stream.avail_out = out_len;

   for (;;)
   {
      if ( !stream.avail_out)
      {
         // there is no more space for deallocation - increase the space
         out_p = (unsigned char*)realloc(out_p,out_len + ZLIB_MEM_DELTA);

         // init new memory
         unsigned char *new_out_start = out_p + out_len;
         memset(new_out_start,0,ZLIB_MEM_DELTA);

         // Point next_out to the next unused byte
         stream.next_out = new_out_start;

         // Update the size of the uncompressed buffer
         stream.avail_out = (uInt)ZLIB_MEM_DELTA;
         out_len += ZLIB_MEM_DELTA;
      }
      err = inflate(&stream,Z_NO_FLUSH);

      if ( err == Z_STREAM_END)
      {
         break;
      }
      ZLIB_CHECK_ERR(err,"Deflate");
      if ( err != Z_OK )
      {         
         if( out_p )
         {
            free(out_p);
         }
         return false;
      }
   }
   err = inflateEnd(&stream);
   ZLIB_CHECK_ERR(err,"InflateEnd");
   if( err != Z_OK )
   {
      if( out_p )
      {
         free(out_p);
      }
      return false;
   }
   encoded = std::string((char*)out_p,stream.total_out);
   free(out_p);
   //    trace_hex((char*)encoded.c_str(),encoded.size());
   // if predictor exists for that object, then lets decode it
   if( _predict )
   {
      _predict->decode(encoded);
   }

   return true;
}

