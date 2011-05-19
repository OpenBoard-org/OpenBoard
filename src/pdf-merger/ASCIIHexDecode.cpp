#include "ASCIIHexDecode.h"

#include <string>
#include "Utils.h"

using namespace merge_lib;

const std::string WHITESPACES(" \t\f\v\n\r");

#define HEX_TO_VAL(char_c) (char_c)>9?'A'+(char_c)-10:'0'+(char_c);

static unsigned int convertHexVal(unsigned char c)
{
   if(c >= '0' && c <= '9')
   {
      return (c - '0');
   }
   if(c >= 'A' && c <= 'F')
   {
      return (c - 'A' + 10);
   }
   if(c >= 'a' && c <= 'f')
   {
      return (c - 'a' + 10);
   }
   return 0;
}

bool ASCIIHexDecode::decode(std::string & encoded)
{
   bool isLow = true;
   unsigned char decodedChar = '\0';
   int len = encoded.size();
   std::string decoded ="";
   for(int i = 0;i<len;i++)
   {
      unsigned char ch = encoded[i];
      if( WHITESPACES.find(ch) != std::string::npos )
      {
         continue;
      }
      if( ch == '>' )
      {
         continue; // EOD found
      }
      ch = convertHexVal(ch);
      if( isLow ) 
      {
         decodedChar = (ch & 0x0F);
         isLow         = false;
      }
      else
      {
         decodedChar = ((decodedChar << 4) | ch);
         isLow         = true;
         decoded += decodedChar;                        
      }
   }
   encoded = decoded;
   return true;
}
