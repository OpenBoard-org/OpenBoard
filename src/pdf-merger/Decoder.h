#ifndef DECODER_H
#define DECODER_H

#include <string>

#include "Object.h"

namespace merge_lib
{
   // base class 4 all decoders
   class Decoder
   {
   public:
      Decoder(){};
      virtual ~Decoder(){};
      virtual bool encode(std::string &decoded) = 0;
      virtual bool decode(std::string &encoded) = 0;
      //read fields of objectWithStream and initialize internal parameters
      //of decoder
      virtual void initialize(Object * objectWithStram) = 0;

   };
}
#endif // DECODER_H

