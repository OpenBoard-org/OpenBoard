#ifndef EXCEPTION_H
#define EXCEPTION_H


#include <exception>
#include <string>
#include <sstream>
#include <iostream>


namespace merge_lib
{
   class Exception : public std::exception
   {
   public:
      Exception() {}

      Exception(const char * message) : _message(message) {}
      
      Exception(std::string & message) : _message(message) {}

      Exception(std::stringstream & message) : _message(message.str()) {}

      Exception(const std::string & message) : _message(message) {}
      
      virtual ~Exception() throw () {}
      
      virtual const char * what() const throw() { return _message.c_str(); }

      void show() const {}

   protected:
      std::string _message;
   };
}
#endif // EXCEPTION_HH
