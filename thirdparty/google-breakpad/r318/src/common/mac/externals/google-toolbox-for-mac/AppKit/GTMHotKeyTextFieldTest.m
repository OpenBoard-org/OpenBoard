//  GTMHotKeyTextFieldTest.m
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

#import "GTMHotKeyTextField.h"

#import "GTMSenTestCase.h"

@interface GTMHotKeyTextField (PrivateMethods)
// Private methods which we want to access to test
+ (BOOL)isValidHotKey:(NSDictionary *)hotKey;
+ (NSString *)displayStringForHotKey:(NSDictionary *)hotKey;
@end

@interface GTMHotKeyTextFieldTest : GTMTestCase
@end

@implementation GTMHotKeyTextFieldTest

- (void)testStringForModifierFlags {
  
  // Make sure only the flags we expect generate things in their strings
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:NSAlphaShiftKeyMask] length],
                 (NSUInteger)0, nil);
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:NSShiftKeyMask] length],
                 (NSUInteger)1, nil);
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:NSControlKeyMask] length],
                 (NSUInteger)1, nil);
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:NSAlternateKeyMask] length],
                 (NSUInteger)1, nil);
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:NSCommandKeyMask] length],
                 (NSUInteger)1, nil);
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:NSNumericPadKeyMask] length],
                 (NSUInteger)0, nil);
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:NSHelpKeyMask] length],
                 (NSUInteger)0, nil);
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:NSFunctionKeyMask] length],
                 (NSUInteger)0, nil);
  
  // And some quick checks combining flags to make sure the string gets longer
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:(NSShiftKeyMask |
                                                        NSAlternateKeyMask)] length],
                 (NSUInteger)2, nil);
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:(NSShiftKeyMask |
                                                        NSAlternateKeyMask |
                                                        NSCommandKeyMask)] length],
                 (NSUInteger)3, nil);
  STAssertEquals([[GTMHotKeyTextField stringForModifierFlags:(NSShiftKeyMask |
                                                        NSAlternateKeyMask |
                                                        NSCommandKeyMask |
                                                        NSControlKeyMask)] length],
                 (NSUInteger)4, nil);
  
}

- (void)testStringForKeycode_useGlyph_resourceBundle {
  NSBundle *bundle = [NSBundle bundleForClass:[self class]];
  STAssertNotNil(bundle, @"failed to get our bundle?");
  NSString *str;
  
  // We need a better test, but for now, we'll just loop through things we know
  // we handle.
  
  // TODO: we need to force the pre leopard code path during tests.

  UInt16 testData[] = {
    123, 124, 125, 126, 122, 120, 99, 118, 96, 97, 98, 100, 101, 109, 103, 111,
    105, 107, 113, 106, 117, 36, 76, 48, 49, 51, 71, 53, 115, 116, 119, 121,
    114, 65, 67, 69, 75, 78, 81, 82, 83, 84, 85, 86, 87, 88, 89, 91, 92,
  };
  for (int useGlyph = 0 ; useGlyph < 2 ; ++useGlyph) {
    for (size_t i = 0; i < (sizeof(testData) / sizeof(UInt16)); ++i) {
      UInt16 keycode = testData[i];
      
      str = [GTMHotKeyTextField stringForKeycode:keycode
                                        useGlyph:useGlyph
                                  resourceBundle:bundle];
      STAssertNotNil(str,
                     @"failed to get a string for keycode %u (useGlyph:%@)",
                     keycode, (useGlyph ? @"YES" : @"NO"));
      STAssertGreaterThan([str length], (NSUInteger)0,
                          @"got an empty string for keycode %u (useGlyph:%@)",
                          keycode, (useGlyph ? @"YES" : @"NO"));
    }
  }
}

- (void)testGTMHotKeyPrettyString {
  NSDictionary *hkDict;
  
  hkDict = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithBool:NO],
            kGTMHotKeyDoubledModifierKey,
            [NSNumber numberWithUnsignedInt:114],
            kGTMHotKeyKeyCodeKey,
            [NSNumber numberWithUnsignedInt:NSCommandKeyMask],
            kGTMHotKeyModifierFlagsKey,
            nil];
  STAssertNotNil(hkDict, nil);
  STAssertNotNil([GTMHotKeyTextField displayStringForHotKey:hkDict], nil);
  
  hkDict = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithUnsignedInt:114],
            kGTMHotKeyKeyCodeKey,
            [NSNumber numberWithUnsignedInt:NSCommandKeyMask],
            kGTMHotKeyModifierFlagsKey,
            nil];
  STAssertNotNil(hkDict, nil);
  STAssertNotNil([GTMHotKeyTextField displayStringForHotKey:hkDict], nil);

  hkDict = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithBool:NO],
            kGTMHotKeyDoubledModifierKey,
            [NSNumber numberWithUnsignedInt:NSCommandKeyMask],
            kGTMHotKeyModifierFlagsKey,
            nil];
  STAssertNotNil(hkDict, nil);
  STAssertNotNil([GTMHotKeyTextField displayStringForHotKey:hkDict], nil);
  
  hkDict = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithBool:NO],
            kGTMHotKeyDoubledModifierKey,
            [NSNumber numberWithUnsignedInt:114],
            kGTMHotKeyKeyCodeKey,
            nil];
  STAssertNotNil(hkDict, nil);
  STAssertNil([GTMHotKeyTextField displayStringForHotKey:hkDict], nil);
  
  hkDict = [NSDictionary dictionary];
  STAssertNotNil(hkDict, nil);
  STAssertNil([GTMHotKeyTextField displayStringForHotKey:hkDict], nil);

  STAssertNil([GTMHotKeyTextField displayStringForHotKey:nil], nil);
  
}

- (void)testGTMHotKeyDictionaryAppearsValid {
  NSDictionary *hkDict;

  hkDict = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithBool:NO],
            kGTMHotKeyDoubledModifierKey,
            [NSNumber numberWithUnsignedInt:114],
            kGTMHotKeyKeyCodeKey,
            [NSNumber numberWithUnsignedInt:NSCommandKeyMask],
            kGTMHotKeyModifierFlagsKey,
            nil];
  STAssertNotNil(hkDict, nil);
  STAssertTrue([GTMHotKeyTextField isValidHotKey:hkDict], nil);
  
  hkDict = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithUnsignedInt:114],
            kGTMHotKeyKeyCodeKey,
            [NSNumber numberWithUnsignedInt:NSCommandKeyMask],
            kGTMHotKeyModifierFlagsKey,
            nil];
  STAssertNotNil(hkDict, nil);
  STAssertFalse([GTMHotKeyTextField isValidHotKey:hkDict], nil);
  
  hkDict = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithBool:NO],
            kGTMHotKeyDoubledModifierKey,
            [NSNumber numberWithUnsignedInt:NSCommandKeyMask],
            kGTMHotKeyModifierFlagsKey,
            nil];
  STAssertNotNil(hkDict, nil);
  STAssertFalse([GTMHotKeyTextField isValidHotKey:hkDict], nil);
  
  hkDict = [NSDictionary dictionaryWithObjectsAndKeys:
            [NSNumber numberWithBool:NO],
            kGTMHotKeyDoubledModifierKey,
            [NSNumber numberWithUnsignedInt:114],
            kGTMHotKeyKeyCodeKey,
            nil];
  STAssertNotNil(hkDict, nil);
  STAssertFalse([GTMHotKeyTextField isValidHotKey:hkDict], nil);
  
  hkDict = [NSDictionary dictionary];
  STAssertNotNil(hkDict, nil);
  STAssertFalse([GTMHotKeyTextField isValidHotKey:hkDict], nil);
  
  STAssertFalse([GTMHotKeyTextField isValidHotKey:nil], nil);
  
  // Make sure it doesn't choke w/ an object of the wrong time (since the dicts
  // have to be saved/reloaded.
  hkDict = (id)[NSString string];
  STAssertNotNil(hkDict, nil);
  STAssertFalse([GTMHotKeyTextField isValidHotKey:hkDict], nil);
}

@end
