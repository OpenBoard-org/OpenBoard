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
      bool encode(std::string & decoded){return false;}
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



