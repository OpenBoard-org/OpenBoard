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

