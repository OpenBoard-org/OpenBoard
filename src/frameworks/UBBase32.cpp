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




#include "UBBase32.h"

#include "core/memcheck.h"

QString UBBase32::sBase32Chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";

int UBBase32::sBase32Lookup[] =
    { 0xFF,0xFF,0x1A,0x1B,0x1C,0x1D,0x1E,0x1F, // '0', '1', '2', '3', '4', '5', '6', '7'
      0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF, // '8', '9', ':', ';', '<', '=', '>', '?'
      0xFF,0x00,0x01,0x02,0x03,0x04,0x05,0x06, // '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G'
      0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E, // 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O'
      0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16, // 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W'
      0x17,0x18,0x19,0xFF,0xFF,0xFF,0xFF,0xFF, // 'X', 'Y', 'Z', '[', '\', ']', '^', '_'
      0xFF,0x00,0x01,0x02,0x03,0x04,0x05,0x06, // '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g'
      0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E, // 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o'
      0x0F,0x10,0x11,0x12,0x13,0x14,0x15,0x16, // 'p', 'q', 'r', 's', 't', 'u', 'v', 'w'
      0x17,0x18,0x19,0xFF,0xFF,0xFF,0xFF,0xFF  // 'x', 'y', 'z', '{', '|', '}', '~', 'DEL'
    };


QByteArray UBBase32::decode(const QString& base32String)
{
    int i, index, offset;
    QByteArray bytes(base32String.length() * 5/8, '\0');

    for (i = 0, index = 0, offset = 0; i < base32String.length(); i++)
    {
        QChar ch = base32String.at(i);
        char lookup = ch.toLatin1() - '0';

        /* Skip chars outside the lookup table */
        if (lookup < 0 || lookup >= 80)
            continue;

        int digit = sBase32Lookup[(int)lookup];

        /* If this digit is not in the table, ignore it */
        if (digit == 0xFF)
            continue;

        if (index <= 3)
        {
            index = (index + 5) % 8;
            if (index == 0)
            {
               bytes[offset] = bytes[offset] | digit;
               offset++;

               if(offset >= bytes.length())
                   break;
            }
            else
               bytes[offset] = bytes[offset] | (digit << (8 - index));
        }
        else
        {
            index = (index + 5) % 8;
            bytes[offset] = bytes[offset] | ((unsigned int)digit >> index);
            offset++;

            if(offset >= bytes.length())
                break;

            bytes[offset] = bytes[offset] | (digit << (8 - index));
        }
    }
    return bytes;

}
