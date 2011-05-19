#ifndef CCITTFaxDecode_H
#define CCITTFaxDecode_H

#include <string>
namespace merge_lib
{
   // this class provides method for FlateDecode encoding and decoding
   class CCITTFaxDecode : public Decoder
   {
      public:
         CCITTFaxDecode(){};
         virtual ~CCITTFaxDecode(){};
         bool encode(std::string & decoded) {return true;};
         bool decode(std::string & encoded) {return true;};
         void initialize(Object * objectWithStram){};

   };
}

#endif // FLATEDECODE_H_INCLUDED

