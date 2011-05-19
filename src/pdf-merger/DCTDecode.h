#ifndef DCTDecode_H 
#define DCTDecode_H

#include <string>

namespace merge_lib
{
   // this class provides method for FlateDecode encoding and decoding
   class DCTDecode : public Decoder
   {
      public:
         DCTDecode(){};
         virtual ~DCTDecode(){};
         bool encode(std::string & decoded) {return true;};
         bool decode(std::string & encoded) {return true;};
         void initialize(Object * objectWithStram){};

   };
}

#endif // FLATEDECODE_H_INCLUDED

