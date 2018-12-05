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




#include "UBKeyboardPalette.h"

#include <windows.h>

#include "../core/UBApplication.h"
#include "../gui/UBMainWindow.h"

#include "core/memcheck.h"

void UBKeyboardButton::sendUnicodeSymbol(KEYCODE keycode)
{
    INPUT input[2];
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = 0;
    input[0].ki.wScan = keycode.symbol;
    input[0].ki.dwFlags = KEYEVENTF_UNICODE;
    input[0].ki.time = 0;
    input[0].ki.dwExtraInfo = 0;

    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = 0;
    input[1].ki.wScan = keycode.symbol;
    input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    input[1].ki.time = 0;
    input[1].ki.dwExtraInfo = 0;

    ::SendInput(2, input, sizeof(input[0]));
}

void UBKeyboardButton::sendControlSymbol(int nSymbol)
{
    INPUT input[2];
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = nSymbol;
    input[0].ki.wScan = 0;
    input[0].ki.dwFlags = 0;
    input[0].ki.time = 0;
    input[0].ki.dwExtraInfo = 0;

    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = nSymbol;
    input[1].ki.wScan = 0;
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;
    input[1].ki.time = 0;
    input[1].ki.dwExtraInfo = 0;

    ::SendInput(2, input, sizeof(input[0]));
}

void UBKeyboardPalette::createCtrlButtons()
{
    int ctrlID = 0;
    ctrlButtons = new UBKeyboardButton*[9];

    ctrlButtons[ctrlID++] = new UBCntrlButton(this, 0x08, "backspace");// Backspace
    ctrlButtons[ctrlID++] = new UBCntrlButton(this, 0x09, "tab");      // Tab
    ctrlButtons[ctrlID++] = new UBCapsLockButton(this, "capslock");    // Shift
    ctrlButtons[ctrlID++] = new UBCntrlButton(this, tr("Enter"), 0x0d);    // Enter
    ctrlButtons[ctrlID++] = new UBShiftButton(this, "shift");    // Shift
    ctrlButtons[ctrlID++] = new UBShiftButton(this, "shift");    // Shift
    ctrlButtons[ctrlID++] = new UBLocaleButton(this);                  // Language Switch 
    ctrlButtons[ctrlID++] = new UBCntrlButton(this, "", 0x20);         // Space
    ctrlButtons[ctrlID++] = new UBLocaleButton(this);                  // Language Switch 
}

void UBKeyboardPalette::checkLayout()
{}

void UBKeyboardPalette::onActivated(bool)
{}

void UBKeyboardPalette::onLocaleChanged(UBKeyboardLocale* )
{}

