#ifndef RunLengthDecode_H
#define RunLengthDecode_H

#include <string>
#include "Decoder.h"

namespace merge_lib
{
   // this class provides method for RunLengthDecode aaaaaencoding and decoding
   class RunLengthDecode : public Decoder
   {
      public:
         RunLengthDecode(){};
         virtual ~RunLengthDecode(){};
         bool encode(std::string & decoded){return false;}
         bool decode(std::string & encoded);
         void initialize(Object * objectWithStream){};

   };
}
#endif // FLATEDECODE_H_INCLUDED

