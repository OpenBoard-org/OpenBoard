/*
	File:		APELite.h

	Contains:	Application Enhancer Lite interfaces

	Copyright:	Copyright 2002-2003 Unsanity, LLC.
				All Rights Reserved.
 
*/
#ifndef _H_APELite
#define _H_APELite

#include <mach-o/loader.h>

#ifdef __cplusplus
extern "C" {
#endif

// Error checking function. object can be NULL in which case false is returned.
// This determines if the object (CFTypeRef) is an instance of the class named in type. The type is usually the var type minus "Ref".
// For instance to determine if an NSString* is really an NSString*, APEObjectIsType(string, CFString).
// This works for all toll-free bridged types such as using APEObjectIsType(timer, CFRunLoopTimer) where timer is an NSTimer* instance.
// Special notes: NSBundle and CFBundleRef are not toll-free bridged. And as of 10.4, there is no HIObjectGetTypeID() yet so you cannot
// check to see if a specific object is an HIObjectRef until 10.5. If object is a Cocoa object, it must be toll-free bridged with a CF version.
#define APEObjectIsType(object, type) ((NULL!=object) && type ## GetTypeID()==CFGetTypeID(object))

// Public and private mach-o symbol lookup.
extern void *APEFindSymbol(struct mach_header *image,const char *symbol);

// Mach-o function patching.
extern void *APEPatchCreate(const void *patchee,const void *address);
extern void *APEPatchGetAddress(void *patch);
extern void APEPatchSetAddress(void *patch,const void *address);

void apeprintf(const char *format,...);

#ifdef __cplusplus
}
#endif

#endif /* _H_APELite */
