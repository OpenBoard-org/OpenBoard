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

#include <X11/Xlib.h>
#include <X11/keysym.h>


// The key code to be sent.
// A full list of available codes can be found in /usr/include/X11/keysymdef.h
//#define KEYCODE XK_Down

// Function to create a keyboard event
XKeyEvent createKeyEvent(Display *display, Window &win,
                           Window &winRoot, bool press,
                           int keycode, int modifiers)
{
   XKeyEvent event;

   event.display     = display;
   event.window      = win;
   event.root        = winRoot;
   event.subwindow   = None;
   event.time        = CurrentTime;
   event.x           = 1;
   event.y           = 1;
   event.x_root      = 1;
   event.y_root      = 1;
   event.same_screen = True;
   event.keycode     = keycode;
   event.state       = modifiers;

   if(press)
      event.type = KeyPress;
   else
      event.type = KeyRelease;

   return event;
}

void x11SendKey(Display *display, int keyCode, int modifiers)
{

// Get the root window for the current display.
   Window winRoot = XDefaultRootWindow(display);
   if (winRoot==0)
       return;

// Find the window which has the current keyboard focus.
   Window winFocus;
   int    revert;
   XGetInputFocus(display, &winFocus, &revert);

// Send a fake key press event to the window.
   XKeyEvent event = createKeyEvent(display, winFocus, winRoot, true, keyCode, modifiers);
   XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

// Send a fake key release event to the window.
   event = createKeyEvent(display, winFocus, winRoot, false, keyCode, modifiers);
   XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);

}

/*
void traceKeyCodes()
{
    Display *display = XOpenDisplay(0);
    if(display == NULL)
       return;

    int min_keycodes, max_keycodes, byte_per_code;

    XDisplayKeycodes(display, &min_keycodes, &max_keycodes);
    KeySym* keySyms = XGetKeyboardMapping(display, min_keycodes,
                       max_keycodes - min_keycodes, &byte_per_code);

    qDebug() << "Codes table, min_keycodes: " << min_keycodes << ", max_keycodes: "
             << max_keycodes << ", bytes_per_code:" << byte_per_code;
    for(int i=0; i<max_keycodes - min_keycodes; i++)
    {
        QString str;
        for(int j=0; j<byte_per_code;j++)
        {
            str += QString::number(keySyms[i*byte_per_code + j], 16);
            str += ":";
        }
        qDebug() << i << ":" << str;
    }

    XFree(keySyms);

    XCloseDisplay(display);
}
*/


void UBKeyboardButton::sendUnicodeSymbol(KEYCODE keycode)
{
    // Obtain the X11 display.
    Display *display = XOpenDisplay(0);
    if(display == NULL)
       return;

    if (!keycode.empty())
    {
        int modifier = keycode.modifier;
        if (keycode.modifier==2) modifier = 0x2000;
        if (keycode.modifier==3) modifier = 0x2001;
        if (keycode.modifier==4) modifier = 0x4000;
        if (keycode.modifier==5) modifier = 0x4001;
        //modifiers 6 and 7 seems are not available.... They are reassigned in layout creationtime
        //if (keycode.modifier==6) modifier = 0x6000;
        //if (keycode.modifier==7) modifier = 0x6001;

        x11SendKey(display, keycode.code + keyboard->min_keycodes , modifier);
    }

    XCloseDisplay(display);
}

void UBKeyboardButton::sendControlSymbol(int nSymbol)
{
    // Obtain the X11 display.
    Display *display = XOpenDisplay(0);
    if(display == NULL)
       return;


    KeyCode keyCode = XKeysymToKeycode(display, nSymbol);

    if (keyCode != NoSymbol)
    {
        x11SendKey(display, keyCode, 0);
    }

    XCloseDisplay(display);
}

void UBKeyboardPalette::createCtrlButtons()
{
        ctrlButtons = new UBKeyboardButton*[9];

        ctrlButtons[0] = new UBCntrlButton(this, XK_BackSpace, "backspace");
        ctrlButtons[1] = new UBCntrlButton(this, XK_Tab, "tab");
        ctrlButtons[2] = new UBCapsLockButton(this, "capslock");
        ctrlButtons[3] = new UBCntrlButton(this, tr("Enter"), XK_Return);
        ctrlButtons[4] = new UBShiftButton(this, "shift");
        ctrlButtons[5] = new UBShiftButton(this, "shift");
        ctrlButtons[6] = new UBLocaleButton(this);
        ctrlButtons[7] = new UBCntrlButton(this, "", XK_space);
        ctrlButtons[8] = new UBLocaleButton(this);
}


void UBKeyboardPalette::checkLayout()
{}

void UBKeyboardPalette::onActivated(bool activated)
{
    if (activated)
    {
        if (storage)
        {
            qDebug() << "Keybard already activated....";
            return;
        }

        Display *display = XOpenDisplay(0);
        if(display == NULL)
           return;

        XDisplayKeycodes(display, &this->min_keycodes, &this->max_keycodes);
        KeySym* keySyms = XGetKeyboardMapping(display, min_keycodes,
                           max_keycodes - min_keycodes, &byte_per_code);

        storage = keySyms;

        XCloseDisplay(display);

        onLocaleChanged(locales[nCurrentLocale]);

    }
    else
    {
        Display *display = XOpenDisplay(0);
        if(display == NULL)
        {
            qDebug() << "Keybard not activated....";
            return;
        }

        KeySym* keySyms = (KeySym*)storage;
        if (keySyms!=NULL)
        {
            qDebug() << "Default key table restored.....";

            XChangeKeyboardMapping(display, min_keycodes, byte_per_code,
                                   keySyms, max_keycodes - min_keycodes);

            XFree(keySyms);
            storage = NULL;
        }

        XCloseDisplay(display);
    }
}

void UBKeyboardPalette::onLocaleChanged(UBKeyboardLocale* locale)
{
    const int maxMapOffset = 3; //Suppose to have at least 2 keysym groups due to X11 xlib specification

    Display *display = XOpenDisplay(0);
    if(display == NULL)
       return;

    int byte_per_code;
    KeySym* keySyms = XGetKeyboardMapping(display, min_keycodes,
                       max_keycodes - min_keycodes, &byte_per_code);

    for(int i=0; i<SYMBOL_KEYS_COUNT; i++)
    {
        // loop by keybt
        for(int j=0; j<8; j++)
        {
            KEYCODE& kc = (*locale)[i]->codes[j];
            if (!kc.empty())
            {
                if (kc.modifier <= maxMapOffset)
                    keySyms[kc.code * byte_per_code + kc.modifier] = kc.symbol;

            }
        }
    }

    //Now look for modifiers > 5 and reassign them to free places
    for(int i=0; i<SYMBOL_KEYS_COUNT; i++)
    {
        // loop by keybt
        for(int j=0; j<8; j++)
        {
            KEYCODE& kc = (*locale)[i]->codes[j];
            if (!kc.empty())
            {
                if (kc.modifier > maxMapOffset)
                {
                    for(int i1=0; i1<SYMBOL_KEYS_COUNT; i1++)
                        for(int j1=0; j1<=maxMapOffset; j1++)
                            if (keySyms[i1 * byte_per_code + j1]==NoSymbol)
                            {
                                kc.code =i1;
                                kc.modifier =j1;
                                break;
                            }

                }
                keySyms[kc.code * byte_per_code + kc.modifier] = kc.symbol;
            }
        }
    }


    XChangeKeyboardMapping(display, min_keycodes, byte_per_code, keySyms, max_keycodes - min_keycodes);
    XFree(keySyms);

    XCloseDisplay(display);
}
