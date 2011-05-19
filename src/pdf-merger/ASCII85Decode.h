#ifndef ASCII85Decode_H
#define ASCII85Decode_H

#include <string>

#include "Decoder.h"
namespace merge_lib
{
   // this class provides method for FlateDecode encoding and decoding
   class ASCII85Decode : public Decoder
   {
      public:
         ASCII85Decode(){};
         virtual ~ASCII85Decode(){};
         bool encode(std::string & decoded) {return false;}
         bool decode(std::string & encoded);
         void initialize(Object * objectWithStram){};
      private:
         void _wput(std::string &cur,unsigned long tuple, int len);

   };
}
#endif // FLATEDECODE_H_INCLUDED

