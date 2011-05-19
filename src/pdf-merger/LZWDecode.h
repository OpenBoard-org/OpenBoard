#ifndef LZWDecode_H
#define LZWDecode_H

#include <string>
#include "Decoder.h"
#include "FilterPredictor.h"

namespace merge_lib
{
   // this class provides method for FlateDecode encoding and decoding
   class LZWDecode : public Decoder
   {
   public:
      LZWDecode();
      virtual ~LZWDecode();
      bool encode(std::string & decoded) {return true;};
      bool decode(std::string & encoded);
      void initialize(Object * objectWithStram);
   private:

      FilterPredictor *_predict;

      void clearTable();
      int getCode();

      std::string &_encoded;
      std::string _dummy;
      size_t _curSymbolIndex;

      int _earlyChange;	// early parameter
      int _readBuf;	   
      int _readBits;	   
      int _nextCode;     
      int _bitsToRead;     
      bool _first;	      
      int _curSequenceLength;    
   };
}

#endif // LZW_DECODE_H_INCLUDED

