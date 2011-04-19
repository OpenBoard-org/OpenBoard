#include "UBKeyboardPalette.h"

#include <stdio.h>
#include <CoreServices/CoreServices.h>
#include <ApplicationServices/ApplicationServices.h>
#include <Carbon/Carbon.h>

#include <QMap>
#include <QString>

void UBKeyboardButton::sendUnicodeSymbol(unsigned int nSymbol1, unsigned int nSymbol2, bool shift)
{
	unsigned int nSymbol = (shift)? nSymbol2 : nSymbol1;

	if (shift)
		CGEventPost(kCGSessionEventTap, CGEventCreateKeyboardEvent(NULL, 56, true));
	CGEventPost(kCGSessionEventTap, CGEventCreateKeyboardEvent(NULL, nSymbol, true));
	CGEventPost(kCGSessionEventTap, CGEventCreateKeyboardEvent(NULL, nSymbol, false));
	if (shift)
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
        ctrlButtons = new UBKeyboardButton*[7];

        ctrlButtons[0] = new UBCntrlButton(this, "<-", 51);
        ctrlButtons[1] = new UBCntrlButton(this, "<->", 48);
        ctrlButtons[2] = new UBCntrlButton(this, "Enter", 76);
        ctrlButtons[3] = new UBCapsLockButton(this);
        ctrlButtons[4] = new UBCapsLockButton(this);
        ctrlButtons[5] = new UBLocaleButton(this);
        ctrlButtons[6] = new UBCntrlButton(this, "", 49);
        ctrlButtons[7] = new UBLocaleButton(this);
}

void SetMacLocaleByIdentifier(const QString& id)
{
	const char * strName = id.toAscii().data();
	CFStringRef iName = CFStringCreateWithCString(NULL, strName, kCFStringEncodingMacRoman );

	CFStringRef keys[] = { kTISPropertyInputSourceCategory, kTISPropertyInputSourceID };          
	CFStringRef values[] = { kTISCategoryKeyboardInputSource, iName };          
	CFDictionaryRef dict = CFDictionaryCreate(NULL, (const void **)keys, (const void **)values, 2, NULL, NULL);  
	CFArrayRef kbds = TISCreateInputSourceList(dict, true);          
	if (CFArrayGetCount(kbds)!=0)
	{
		TISInputSourceRef klRef =  (TISInputSourceRef)CFArrayGetValueAtIndex(kbds, 0);  
		if (klRef!=NULL)
			TISSelectInputSource(klRef);
	}
}

void UBKeyboardPalette::onActivated(bool activated)
{
	if (activated)
	{
		TISInputSourceRef selectedLocale = TISCopyCurrentKeyboardInputSource();

		CFStringRef sr = (CFStringRef) TISGetInputSourceProperty(selectedLocale,
															  kTISPropertyInputSourceID);  

		if (sr!=NULL)
		{
			char tmp[1024];
			CFStringGetCString(sr, tmp, 1024, 0);
			activeLocale = tmp;	
		}
		else
			activeLocale = "";
	

	    onLocaleChanged(locales[nCurrentLocale]);
	}
	else
	{
		if (activeLocale != "")
			SetMacLocaleByIdentifier(activeLocale);
	}
}

void UBKeyboardPalette::onLocaleChanged(UBKeyboardLocale* locale)
{
	SetMacLocaleByIdentifier(locale->id);
}

