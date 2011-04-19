
#include "UBPlatformUtils.h"


void UBPlatformUtils::destroy()
{
	initializeKeyboardLayouts();
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