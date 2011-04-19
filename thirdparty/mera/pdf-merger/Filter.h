#if !defined Filter_h
#define Filter_h

#include <vector>
#include <map>
#include <string>

namespace merge_lib
{
   class Object;
    class Decoder;
   //this class is needed to parse object in order to create
   //all decoders to decode object's stream
   class Filter
   {
   public:
      Filter(Object * objectWithStream): _objectWithStream(objectWithStream)
      {
         _createAllDecodersSet();
      }
      virtual ~Filter();
      //replace coded stream with decoded
      void getDecodedStream(std::string & stream);
   private:
      //methods

      //parse object's content and fill out vector with
      //necessary decoders
      std::vector <Decoder * > _getDecoders();
      void _createAllDecodersSet();

      //members
      Object * _objectWithStream;
      static std::map<std::string, Decoder *> _allDecoders;
   };
}
#endif

