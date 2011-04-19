//
//  GTMTheme.h
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

#import "GTMDefines.h"
#import <AppKit/AppKit.h>

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5

// Sent whenever the theme changes. Object => GTMTheme that changed
GTM_EXTERN NSString *kGTMThemeDidChangeNotification;

enum {
  GTMThemeStyleTabBarSelected,
  GTMThemeStyleTabBarDeselected,
  GTMThemeStyleWindow,
  GTMThemeStyleToolBar,
  GTMThemeStyleToolBarButton,
  GTMThemeStyleToolBarButtonPressed,
  GTMThemeStyleBookmarksBar,
};
typedef NSUInteger GTMThemeStyle;

// GTMTheme provides a range of values for procedural drawing of UI elements 
// based on interpolation of a single background color

@interface GTMTheme : NSObject {
 @private
  NSColor *backgroundColor_;  // bound to user defaults 
  NSImage *backgroundImage_;  // bound to user defaults 
  NSMutableDictionary *values_; // cached values
}

// Access the global theme. By default this is bound to user defaults
+ (GTMTheme *)defaultTheme;
+ (void)setDefaultTheme:(GTMTheme *)theme;

// returns base theme color
- (NSColor *)backgroundColor;

// base background color
- (NSImage *)backgroundImage;

// NSColor (or pattern color) for the background of the window
- (NSColor *)windowBackgroundColor:(BOOL)active;

// NSGradient for specific usage, active indicates whether the window is key
- (NSGradient *)gradientForStyle:(GTMThemeStyle)style active:(BOOL)active;

// Outline color for stroke, active indicates whether the window is key
- (NSColor *)strokeColorForStyle:(GTMThemeStyle)style active:(BOOL)active;

// Indicates whether luminance is dark or light
- (BOOL)styleIsDark:(GTMThemeStyle)style active:(BOOL)active;

// Background style for this style and state
- (NSBackgroundStyle)interiorBackgroundStyleForStyle:(GTMThemeStyle)style
                                              active:(BOOL)active;

// NSColor version of the gradient (for window backgrounds, etc)
- (NSColor *)patternColorForStyle:(GTMThemeStyle)style active:(BOOL)active;
@end

#endif // MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
