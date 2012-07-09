/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QtGlobal>
#include "DCTDecode.h"

#include "core/memcheck.h"

using namespace merge_lib;

bool DCTDecode::encode(std::string & decoded)
{
    Q_UNUSED(decoded);
    return true;
}

bool DCTDecode::decode(std::string & encoded)
{
    Q_UNUSED(encoded);
    return true;
}

void DCTDecode::initialize(Object * objectWithStram)
{
    Q_UNUSED(objectWithStram);
}
