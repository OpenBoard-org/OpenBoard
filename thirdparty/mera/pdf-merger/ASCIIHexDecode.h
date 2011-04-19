
#ifndef ASCIIHexDecode_h
#define ASCIIHexDecode_h

#include <string>
#include "Decoder.h"
namespace merge_lib
{
   // this class provides method for ASCIIHEX encoding and decoding
   class ASCIIHexDecode : public Decoder
   {
      public:
         ASCIIHexDecode(){};
         virtual ~ASCIIHexDecode(){};
         bool encode(std::string & decoded){return false;}
         bool decode(std::string & encoded);
         void initialize(Object * objectWithStram){};

   };
}
#endif // FLATEDECODE_H_INCLUDED

