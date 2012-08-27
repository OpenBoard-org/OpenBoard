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


#ifndef FILTER_PREDICTOR_H
#define FILTER_PREDICTOR_H

#include <string>

#include "Decoder.h"

namespace merge_lib
{
   // this method performs filter prediction processing.
   class FilterPredictor:public Decoder
   {
   public:
      FilterPredictor();
      virtual ~FilterPredictor();
      bool encode(std::string & decoded);
      bool decode(std::string & encoded);

      void initialize(Object * objectWithStream);
      static const std::string PREDICTOR_TOKEN;
      static const std::string DECODE_PARAM_TOKEN;

      int getEarlyChange() const { return _earlyChange;}


   private:
      bool decodeRow(const char *input, std::string &out,const std::string &prev,int curPrediction);
      void obtainDecodeParams(Object*objectWithStream,std::string &dictStr);
      std::string getDictionaryContentStr(std::string & in, size_t &pos );
      int _predictor;
      int _colors;
      int _bits;
      int _columns;
      int _earlyChange;
      int _rowLen;
      int _bytesPerPixel;

   };
}

#endif



