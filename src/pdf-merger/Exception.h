/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation, version 2,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with Open-Sankoré; if not, see
 * <http://www.gnu.org/licenses/>.
 */


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
