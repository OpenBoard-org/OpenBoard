//
//  GTMUnitTestingUtilities.h
//
//  Copyright 2006-2008 Google Inc.
//
//  Licensed under the Apache License, Version 2.0 (the "License"); you may not
//  use this file except in compliance with the License.  You may obtain a copy
//  of the License at
// 
//  http://www.apache.org/licenses/LICENSE-2.0
// 
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
//  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  See the
//  License for the specific language governing permissions and limitations under
//  the License.
//

#import <Foundation/Foundation.h>
#import <objc/objc.h>

// Collection of utilities for unit testing
@interface GTMUnitTestingUtilities : NSObject

// Returns YES if we are currently being unittested.
+ (BOOL)areWeBeingUnitTested;

// Sets up the user interface so that we can run consistent UI unittests on
// it. This includes setting scroll bar types, setting selection colors
// setting color spaces etc so that everything is consistent across machines.
// This should be called in main, before NSApplicationMain is called.
+ (void)setUpForUIUnitTests;

// Syntactic sugar combining the above, and wrapping them in an 
// NSAutoreleasePool so that your main can look like this:
// int main(int argc, const char *argv[]) {
//   [UnitTestingUtilities setUpForUIUnitTestsIfBeingTested];
//   return NSApplicationMain(argc, argv);
// }
+ (void)setUpForUIUnitTestsIfBeingTested;

// Check if the screen saver is running. Some unit tests don't work when
// the screen saver is active.
+ (BOOL)isScreenSaverActive;

// Allows for posting either a keydown or a keyup with all the modifiers being 
// applied. Passing a 'g' with NSKeyDown and NSShiftKeyMask 
// generates two events (a shift key key down and a 'g' key keydown). Make sure
// to balance this with a keyup, or things could get confused. Events get posted 
// using the CGRemoteOperation events which means that it gets posted in the 
// system event queue. Thus you can affect other applications if your app isn't
// the active app (or in some cases, such as hotkeys, even if it is).
//  Arguments:
//    type - Event type. Currently accepts NSKeyDown and NSKeyUp
//    keyChar - character on the keyboard to type. Make sure it is lower case.
//              If you need upper case, pass in the NSShiftKeyMask in the
//              modifiers. i.e. to generate "G" pass in 'g' and NSShiftKeyMask.
//              to generate "+" pass in '=' and NSShiftKeyMask.
//    cocoaModifiers - an int made up of bit masks. Handles NSAlphaShiftKeyMask,
//                    NSShiftKeyMask, NSControlKeyMask, NSAlternateKeyMask, and
//                    NSCommandKeyMask
+ (void)postKeyEvent:(NSEventType)type 
           character:(CGCharCode)keyChar 
           modifiers:(UInt32)cocoaModifiers;

// Syntactic sugar for posting a keydown immediately followed by a key up event
// which is often what you really want. 
//  Arguments:
//    keyChar - character on the keyboard to type. Make sure it is lower case.
//              If you need upper case, pass in the NSShiftKeyMask in the
//              modifiers. i.e. to generate "G" pass in 'g' and NSShiftKeyMask.
//              to generate "+" pass in '=' and NSShiftKeyMask.
//    cocoaModifiers - an int made up of bit masks. Handles NSAlphaShiftKeyMask,
//                    NSShiftKeyMask, NSControlKeyMask, NSAlternateKeyMask, and
//                    NSCommandKeyMask
+ (void)postTypeCharacterEvent:(CGCharCode)keyChar 
                     modifiers:(UInt32)cocoaModifiers;

// Runs the event loop in NSDefaultRunLoopMode until date. Can be useful for
// testing user interface responses in a controlled timed event loop. For most
// uses using:
// [[NSRunLoop currentRunLoop] runUntilDate:date]
// will do. The only reason you would want to use this is if you were 
// using the postKeyEvent:character:modifiers to send events and wanted to
// receive user input.
//  Arguments:
//    date - end of execution time
+ (void)runUntilDate:(NSDate*)date;

@end

