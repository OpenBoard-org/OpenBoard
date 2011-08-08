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

#include "UBKeyboardPalette.h"

#include <windows.h>

#include "../core/UBApplication.h"
#include "../gui/UBMainWindow.h"

#include "core/memcheck.h"

void UBKeyboardButton::sendUnicodeSymbol(unsigned int nSymbol1, unsigned int nSymbol2, bool shift)
{
	unsigned int nSymbol = shift? nSymbol2 : nSymbol1;

	INPUT input[2];
	input[0].type = INPUT_KEYBOARD;
	input[0].ki.wVk = 0;
	input[0].ki.wScan = nSymbol;
	input[0].ki.dwFlags = KEYEVENTF_UNICODE;
	input[0].ki.time = 0;
	input[0].ki.dwExtraInfo = 0;

	input[1].type = INPUT_KEYBOARD;
	input[1].ki.wVk = 0;
	input[1].ki.wScan = nSymbol;
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
	ctrlButtons = new UBKeyboardButton*[7];

	ctrlButtons[0] = new UBCntrlButton(this, "<-", 0x08);
	ctrlButtons[1] = new UBCntrlButton(this, "<->", 0x09);
	ctrlButtons[2] = new UBCntrlButton(this, "Enter", 0x0d);
	ctrlButtons[3] = new UBCapsLockButton(this);
	ctrlButtons[4] = new UBCapsLockButton(this);
	ctrlButtons[5] = new UBLocaleButton(this);
	ctrlButtons[6] = new UBCntrlButton(this, "", 0x20);
	ctrlButtons[7] = new UBLocaleButton(this);
}

void UBKeyboardPalette::onActivated(bool)
{
}

void UBKeyboardPalette::onLocaleChanged(UBKeyboardLocale* )
{}

