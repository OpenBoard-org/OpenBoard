#ifndef FLATEDECODE_H_INCLUDED
#define FLATEDECODE_H_INCLUDED

#include "Decoder.h"
#include <string>

#include "Decoder.h"
#include "FilterPredictor.h"

namespace merge_lib
{
   // this class provides method for FlateDecode encoding and decoding
   class FlateDecode : public Decoder
   {
      public:
         FlateDecode();
         virtual ~FlateDecode();
         bool encode(std::string & decoded);
         bool decode(std::string & encoded);
         void initialize(Object * objectWithStream);
      private:
         FilterPredictor *_predict;
   };
}
#endif // FLATEDECODE_H_INCLUDED

