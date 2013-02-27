/*
 * Copyright (C) 2012 Webdoc SA
 *
 * This file is part of Open-Sankoré.
 *
 * Open-Sankoré is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3 of the License,
 * with a specific linking exception for the OpenSSL project's
 * "OpenSSL" library (or with modified versions of it that use the
 * same license as the "OpenSSL" library).
 *
 * Open-Sankoré is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Open-Sankoré.  If not, see <http://www.gnu.org/licenses/>.
 */



#include "UBKeyboardPalette.h"

#include <stdio.h>
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>

#include <QMap>
#include <QString>

void UBKeyboardButton::sendUnicodeSymbol(KEYCODE keycode)
{
    if (keycode.modifier)
        CGEventPost(kCGSessionEventTap, CGEventCreateKeyboardEvent(NULL, 56, true));
    CGEventPost(kCGSessionEventTap, CGEventCreateKeyboardEvent(NULL, keycode.code, true));
    CGEventPost(kCGSessionEventTap, CGEventCreateKeyboardEvent(NULL, keycode.code, false));
    if (keycode.modifier)
        CGEventPost(kCGSessionEventTap, CGEventCreateKeyboardEvent(NULL, 56, false));
    
}

void UBKeyboardButton::sendControlSymbol(int nSymbol)
{
    CGEventRef event1 = CGEventCreateKeyboardEvent(NULL, nSymbol, true);
    CGEventRef event2 = CGEventCreateKeyboardEvent(NULL, nSymbol, false);

    CGEventPost(kCGHIDEventTap, event1);
    CGEventPost(kCGHIDEventTap, event2);
}

void UBKeyboardPalette::createCtrlButtons()
{
        ctrlButtons = new UBKeyboardButton*[9];

        ctrlButtons[0] = new UBCntrlButton(this, 51, "backspace");
        ctrlButtons[1] = new UBCntrlButton(this, 48, "tab");
        ctrlButtons[2] = new UBCapsLockButton(this, "capslock");
        ctrlButtons[3] = new UBCntrlButton(this, tr("Enter"), 76);
        ctrlButtons[4] = new UBShiftButton(this, "shift");
        ctrlButtons[5] = new UBShiftButton(this, "shift");
        ctrlButtons[6] = new UBLocaleButton(this);
        ctrlButtons[7] = new UBCntrlButton(this, "", 49);
        ctrlButtons[8] = new UBLocaleButton(this);
}



void UBKeyboardPalette::checkLayout()
{
    TISInputSourceRef selectedLocale = TISCopyCurrentKeyboardInputSource();

    CFStringRef sr = (CFStringRef) TISGetInputSourceProperty(selectedLocale,
                                                          kTISPropertyInputSourceID);

    if (sr!=NULL)
    {
        char clId[1024];
        CFStringGetCString(sr, clId, 1024, 0);

        for(int i=0; i<nLocalesCount;i++)
        {
            if (locales[i]->id == clId)
            {
                if (nCurrentLocale!=i)
                {
                    setLocale(i);
                }
                break;
            }
        }
    }
}

void UBKeyboardPalette::onActivated(bool)
{
}

void UBKeyboardPalette::onLocaleChanged(UBKeyboardLocale* locale)
{
    UBPlatformUtils::SetMacLocaleByIdentifier(locale->id);
}

