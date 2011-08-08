/*
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
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

#include "UBPlatformUtils.h"

#include "core/memcheck.h"

void UBPlatformUtils::destroy()
{
	destroyKeyboardLayouts();
}

UBPlatformUtils::UBPlatformUtils()
{
    // NOOP
}

UBPlatformUtils::~UBPlatformUtils()
{
    // NOOP
}

bool UBPlatformUtils::hasVirtualKeyboard()
{
	return keyboardLayouts!=NULL && nKeyboardLayouts!=0;
}


UBKeyboardLocale::~UBKeyboardLocale()
{
	if (varSymbols!=NULL)
	{
		for(int i=0; i<SYMBOL_KEYS_COUNT; i++)
			delete varSymbols[i];
		delete [] varSymbols;
	}
	delete icon;
}


int UBPlatformUtils::nKeyboardLayouts;
UBKeyboardLocale** UBPlatformUtils::keyboardLayouts;

UBKeyboardLocale** UBPlatformUtils::getKeyboardLayouts(int& nCount)
{
	nCount = nKeyboardLayouts;
	return keyboardLayouts;
}