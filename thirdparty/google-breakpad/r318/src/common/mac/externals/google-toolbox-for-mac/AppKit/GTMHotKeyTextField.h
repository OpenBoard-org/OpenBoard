//
//  GTMHotKeyTextField.h
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

// Text field for capturing hot key entry. This is intended to be similar to the
// Apple key editor in their Keyboard pref pane.

// NOTE: There are strings that need to be localized to use this field.  See the
// code in stringForKeycode the the keys.  The keys are all the English versions
// so you'll get reasonable things if you don't have a strings file.

#import <Cocoa/Cocoa.h>
#import "GTMDefines.h"

// Dictionary key for hot key configuration information modifier flags.
// NSNumber of a unsigned int. Modifier flags are stored using Cocoa constants
// (same as NSEvent) you will need to translate them to Carbon modifier flags
// for use with RegisterEventHotKey()
#define kGTMHotKeyModifierFlagsKey     @"Modifiers"

// Dictionary key for hot key configuration of virtual key code.  NSNumber of
// unsigned int. For double-modifier hotkeys (see below) this value is ignored.
#define kGTMHotKeyKeyCodeKey           @"KeyCode"

// Dictionary key for hot key configuration of double-modifier tap. NSNumber
// BOOL value. Double-tap modifier keys cannot be used with
// RegisterEventHotKey(), you must implement your own Carbon event handler.
#define kGTMHotKeyDoubledModifierKey   @"DoubleModifier"

// Custom text field class used for hot key entry. In order to use this class
// you will need to configure your window's delegate, to return the related
// field editor.
//
//  Sample window delegate method:
//
//    -(id)windowWillReturnFieldEditor:(NSWindow *)sender toObject:(id)anObject {
//      
//      if ([anObject isKindOfClass:[GTMHotKeyTextField class]]) {
//        return [GTMHotKeyFieldEditor sharedHotKeyFieldEditor];
//      } else {
//        return nil;  // Window will use the AppKit shared editor
//      }
//      
//    }
//
//
//  Other notes:
//  - Though you are free to implement control:textShouldEndEditing: in your
//    delegate its return is always ignored. The field always accepts only
//    one hotkey keystroke before editing ends.
//  - The "value" binding of this control is to the dictionary describing the
//    hotkey. At this time binding options are not supported.
//  - The field does not attempt to consume all hotkeys. Hotkeys which are
//    already bound in Apple prefs or other applications will have their
//    normal effect.
//

@interface GTMHotKeyTextField : NSTextField {
 @private
  NSDictionary    *hotKeyDict_;
  // Bindings
  NSObject        *boundObject_;
  NSString        *boundKeyPath_;
}

// Set/Get the hot key dictionary for the field. See above for key names.
- (void)setHotKeyValue:(NSDictionary *)hotKey;
- (NSDictionary *)hotKeyValue;

// Convert Cocoa modifier flags (-[NSEvent modifierFlags]) into a string for
// display. Modifiers are represented in the string in the same order they would
// appear in the Menu Manager.
//
//  Args: 
//    flags: -[NSEvent modifierFlags]
//
//  Returns:
//    Autoreleased NSString
//
+ (NSString *)stringForModifierFlags:(unsigned int)flags;

// Convert a keycode into a string that would result from typing the keycode in
// the current keyboard layout. This may be one or more characters.
//
// Args:
//   keycode: Virtual keycode such as one obtained from NSEvent
//   useGlyph: In many cases the glyphs are confusing, and a string is clearer.
//             However, if you want to display in a menu item, use must
//             have a glyph. Set useGlyph to FALSE to get localized strings
//             which are better for UI display in places other than menus.
//     bundle: Localization bundle to use for localizable key names
//
// Returns:
//   Autoreleased NSString
//
+ (NSString *)stringForKeycode:(UInt16)keycode 
                          useGlyph:(BOOL)useGlyph
                    resourceBundle:(NSBundle *)bundle;

@end

// Custom field editor for use with hotkey entry fields (GTMHotKeyTextField).
// See the GTMHotKeyTextField for instructions on using from the window
// delegate.
@interface GTMHotKeyFieldEditor : NSTextView {
 @private
  NSDictionary    *hotKeyDict_;  // strong
}

// Get the shared field editor for all hot key fields
+ (GTMHotKeyFieldEditor *)sharedHotKeyFieldEditor;

@end
