/*
 * Copyright (C) 2015-2018 Département de l'Instruction Publique (DIP-SEM)
 *
 * Copyright (C) 2013 Open Education Foundation
 *
 * Copyright (C) 2010-2013 Groupement d'Intérêt Public pour
 * l'Education Numérique en Afrique (GIP ENA)
 *
 * This file is part of OpenBoard.
 *
 * OpenBoard is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * OpenBoard is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenBoard. If not, see <http://www.gnu.org/licenses/>.
 */




#ifndef DECODER_H
#define DECODER_H

#include <string>

#include "Object.h"

namespace merge_lib
{
   // base class 4 all decoders
   class Decoder
   {
   public:
      Decoder(){};
      virtual ~Decoder(){};
      virtual bool encode(std::string &decoded) = 0;
      virtual bool decode(std::string &encoded) = 0;
      //read fields of objectWithStream and initialize internal parameters
      //of decoder
      virtual void initialize(Object * objectWithStram) = 0;

   };
}
#endif // DECODER_H

