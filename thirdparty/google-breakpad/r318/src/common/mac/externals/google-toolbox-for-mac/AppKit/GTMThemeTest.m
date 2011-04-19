//
//  GTMThemeTest.m
//
//  Copyright 2009 Google Inc.
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

#import <Cocoa/Cocoa.h>

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5

#import "GTMSenTestCase.h"
#import "GTMTheme.h"

@interface GTMThemeTest : GTMTestCase
@end
  
@implementation GTMThemeTest

- (void)testTheming {
  GTMTheme *theme = [GTMTheme defaultTheme];
  
  // When there are no values, use window default colors
  STAssertEqualObjects([theme backgroundColor], 
                       [NSColor colorWithCalibratedWhite:0.75 alpha:1.0], nil);
  STAssertNil([theme windowBackgroundColor:YES], nil);
  STAssertNil([theme backgroundImage], nil);

  NSColor *color = [NSColor redColor];
  NSData *colorData = [NSArchiver archivedDataWithRootObject:color];
  [[NSUserDefaults standardUserDefaults] setObject:colorData
                                            forKey:@"GTMThemeBackgroundColor"];

  STAssertNotNil([theme windowBackgroundColor:YES], nil);
  STAssertNotNil([theme patternColorForStyle:GTMThemeStyleToolBar
                                      active:YES], nil);
  STAssertNotNil([theme strokeColorForStyle:GTMThemeStyleToolBar
                                     active:YES], nil);
  STAssertNotNil([theme gradientForStyle:GTMThemeStyleToolBar
                                  active:YES], nil);

  STAssertEqualObjects([theme backgroundColor], 
                       color, nil);
  
  // TODO(alcor): add more of these cases once the constants are more concrete
  STAssertEquals([theme interiorBackgroundStyleForStyle:GTMThemeStyleToolBar
                                               active:YES],
                 (NSBackgroundStyle)NSBackgroundStyleRaised, nil);
  
  [[NSUserDefaults standardUserDefaults] removeObjectForKey:
    @"GTMThemeBackgroundColor"];
}

@end

#endif // MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
