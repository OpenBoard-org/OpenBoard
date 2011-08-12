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

void UBKeyboardButton::sendUnicodeSymbol(unsigned int nSymbol1, unsigned int nSymbol2, bool shift)
{
    unsigned int nSymbol = shift ? nSymbol2 : nSymbol1;

    // Obtain the X11 display.
    Display *display = XOpenDisplay(0);
    if(display == NULL)
       return;

    KeyCode keyCode = XKeysymToKeycode(display, nSymbol);

    if (keyCode != NoSymbol)
    {
        x11SendKey(display, keyCode, shift);
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
        ctrlButtons = new UBKeyboardButton*[8];

        ctrlButtons[0] = new UBCntrlButton(this, "<-", XK_BackSpace);
        ctrlButtons[1] = new UBCntrlButton(this, "<->", XK_Tab);
        ctrlButtons[2] = new UBCntrlButton(this, "Enter", XK_Return);
        ctrlButtons[3] = new UBCapsLockButton(this, "capslock");
        ctrlButtons[4] = new UBCapsLockButton(this, "capslock");
        ctrlButtons[5] = new UBLocaleButton(this);
        ctrlButtons[6] = new UBCntrlButton(this, "", XK_space);
        ctrlButtons[7] = new UBLocaleButton(this);
}


void UBKeyboardPalette::onActivated(bool activated)
{
    if (activated)
    {
        Display *display = XOpenDisplay(0);
        if(display == NULL)
           return;

        XDisplayKeycodes(display, &this->min_keycodes, &this->max_keycodes);
        KeySym* keySyms = XGetKeyboardMapping(display, min_keycodes,
                           max_keycodes - min_keycodes, &this->byte_per_code);

        storage = keySyms;

        XCloseDisplay(display);

        onLocaleChanged(locales[nCurrentLocale]);
    }
    else
    {
        Display *display = XOpenDisplay(0);
        if(display == NULL)
           return;

        KeySym* keySyms = (KeySym*)storage;
        if (keySyms!=NULL)
        {

            XChangeKeyboardMapping(display, min_keycodes, byte_per_code,
                                   keySyms, max_keycodes - min_keycodes);
            XFree(keySyms);
        }


        XCloseDisplay(display);
    }

}


void setSymbolsFromButton(Display *display,
                          const UBKeyboardLocale& locale,
                          int byte_per_code,
                          int nFromSymbol,
                          int nFromButton,
                          int count)
{
    KeySym* keySyms = new KeySym[count * byte_per_code];
    for(int i = 0; i < count; i++)
    {
        keySyms[byte_per_code * i + 0] =
                keySyms[byte_per_code * i + 2] = locale[nFromButton + i]->code1;

        keySyms[byte_per_code * i + 1] =
                keySyms[byte_per_code * i + 3] = locale[nFromButton + i]->code2;

        for(int j=4; j<byte_per_code; j++)
            keySyms[byte_per_code * i + j] = NoSymbol;
    }

    XChangeKeyboardMapping(display, nFromSymbol, byte_per_code, keySyms, count);
    delete [] keySyms;
}

void UBKeyboardPalette::onLocaleChanged(UBKeyboardLocale* locale)
{
    Display *display = XOpenDisplay(0);
    if(display == NULL)
       return;

    setSymbolsFromButton(display, *locale, byte_per_code, min_keycodes + 41, 0, 1);
    setSymbolsFromButton(display, *locale, byte_per_code, min_keycodes + 2,  1, 12);
    setSymbolsFromButton(display, *locale, byte_per_code, min_keycodes + 16,  13, 12);
    setSymbolsFromButton(display, *locale, byte_per_code, min_keycodes + 30,  25, 11);
    setSymbolsFromButton(display, *locale, byte_per_code, min_keycodes + 44,  37, 10);

    XCloseDisplay(display);
}
