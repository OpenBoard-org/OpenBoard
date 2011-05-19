#ifndef JBIG2Decode_H
#define JBIG2Decode_H 

#include <string>

namespace merge_lib
{
   // this class provides method for FlateDecode encoding and decoding
   class JBIG2Decode : public Decoder
   {
      public:
         JBIG2Decode(){};
         virtual ~JBIG2Decode(){};
         bool encode(std::string & decoded) {return true;};
         bool decode(std::string & encoded) {return true;};
         void initialize(Object * objectWithStram){};

   };
}

#endif // FLATEDECODE_H_INCLUDED

