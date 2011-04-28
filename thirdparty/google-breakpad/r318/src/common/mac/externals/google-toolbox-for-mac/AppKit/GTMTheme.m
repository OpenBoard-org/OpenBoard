//
//  GTMTheme.m
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

#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5

#import "GTMTheme.h"
#import "GTMNSColor+Luminance.h"

static GTMTheme *gGTMDefaultTheme = nil;
NSString *kGTMThemeDidChangeNotification = @"kGTMThemeDidChangeNotification";

@interface GTMTheme ()
- (void)bindToUserDefaults;
- (void)sendChangeNotification;
@end

@implementation GTMTheme

+ (void)setDefaultTheme:(GTMTheme *)theme {
  if (gGTMDefaultTheme != theme) {
    [gGTMDefaultTheme release];
    gGTMDefaultTheme = [theme retain]; 
    [gGTMDefaultTheme sendChangeNotification];
  }
}

+ (GTMTheme *)defaultTheme {
  @synchronized (self) {
    if (!gGTMDefaultTheme) {
      gGTMDefaultTheme = [[self alloc] init];
      [gGTMDefaultTheme bindToUserDefaults];
    }
  }
  return gGTMDefaultTheme; 
}

- (void)bindToUserDefaults {
  NSUserDefaultsController * controller
    = [NSUserDefaultsController sharedUserDefaultsController];
  [self bind:@"backgroundColor"
    toObject:controller
 withKeyPath:@"values.GTMThemeBackgroundColor"
     options:[NSDictionary dictionaryWithObjectsAndKeys:
              NSUnarchiveFromDataTransformerName,
              NSValueTransformerNameBindingOption,
              nil]];
  
  [self bind:@"backgroundImage"
    toObject:controller
 withKeyPath:@"values.GTMThemeBackgroundImageData"
     options:[NSDictionary dictionaryWithObjectsAndKeys:
              NSUnarchiveFromDataTransformerName,
              NSValueTransformerNameBindingOption,
              nil]];
}

- (id)init {
  self = [super init];
  if (self != nil) {
    values_ = [[NSMutableDictionary alloc] init];
  }
  return self;
}

- (void)finalize {
  [self unbind:@"backgroundColor"];
  [self unbind:@"backgroundImage"];
  [super finalize];
}

- (void)dealloc {
  [self unbind:@"backgroundColor"];
  [self unbind:@"backgroundImage"];
  [values_ release];
  [super dealloc];
}

- (void)sendChangeNotification {
  [[NSNotificationCenter defaultCenter]
    postNotificationName:kGTMThemeDidChangeNotification 
                  object:self];
}

- (id)keyForSelector:(SEL)selector 
               style:(GTMThemeStyle)style
              active:(BOOL)active {
  return [NSString stringWithFormat:@"%p.%d.%d", selector, style, active];
}

- (id)valueForSelector:(SEL)selector 
                 style:(GTMThemeStyle)style
                active:(BOOL)active {
  id value = [values_ objectForKey:
               [self keyForSelector:selector style:style active:active]];
  return value;
}

- (void)cacheValue:(id)value
       forSelector:(SEL)selector 
             style:(GTMThemeStyle)style 
            active:(BOOL)active {
  id key = [self keyForSelector:selector style:style active:active];
  if (key && value) [values_ setObject:value forKey:key];
}

- (void)setBackgroundColor:(NSColor *)value {
  if (backgroundColor_ != value) {
    [backgroundColor_ release];
    backgroundColor_ = [value retain];
    [values_ removeAllObjects];
    [self sendChangeNotification];
  }
}
- (NSColor *)backgroundColor {
  // For nil, we return a color that works with a normal textured window
  if (!backgroundColor_) 
    return [NSColor colorWithCalibratedWhite:0.75 alpha:1.0];
  return backgroundColor_; 
}

- (void)setBackgroundImage:(NSImage *)value {
  if (backgroundImage_ != value) {
    [backgroundImage_ release];
    backgroundImage_ = [value retain];
    [self sendChangeNotification];
  }
}

- (NSColor *)windowBackgroundColor:(BOOL)active {
  NSColor *color = nil;
  if (backgroundImage_) {
    // TODO(alcor): dim images when disabled
    color = [NSColor colorWithPatternImage:backgroundImage_];
  } else if (backgroundColor_) {
   color = [self patternColorForStyle:GTMThemeStyleWindow active:active];    
  }
  return color;
}

- (NSImage *)backgroundImage {
  return backgroundImage_; 
}

- (NSBackgroundStyle)interiorBackgroundStyleForStyle:(GTMThemeStyle)style 
                                              active:(BOOL)active {
  id value = [self valueForSelector:_cmd style:style active:active];
  if (value) return [value intValue];
  
  NSGradient *gradient = [self gradientForStyle:style active:active];
  NSColor *color = [gradient interpolatedColorAtLocation:0.5];
  BOOL dark = [color gtm_isDarkColor];
  value = [NSNumber numberWithInt: dark ? NSBackgroundStyleLowered 
                                 : NSBackgroundStyleRaised];
  [self cacheValue:value forSelector:_cmd style:style active:active];
  return [value intValue];
}

- (BOOL)styleIsDark:(GTMThemeStyle)style active:(BOOL)active {
  id value = [self valueForSelector:_cmd style:style active:active];
  if (value) return [value boolValue];
  
  if (style == GTMThemeStyleToolBarButtonPressed) {
    value = [NSNumber numberWithBool:YES];
  } else {
    value = [NSNumber numberWithBool:[[self backgroundColor] gtm_isDarkColor]];
  }
  [self cacheValue:value forSelector:_cmd style:style active:active];
  return [value boolValue];
}

- (NSColor *)patternColorForStyle:(GTMThemeStyle)style active:(BOOL)active { 
  NSColor *color = [self valueForSelector:_cmd style:style active:active];
  if (color) return color;
  
  NSGradient *gradient = [self gradientForStyle:style active:active];
  if (gradient) {
    // create a gradient image for the background
    CGRect r = CGRectZero;
    // TODO(alcor): figure out a better way to get an image that is the right
    // size
    r.size = CGSizeMake(4, 36);
    size_t bytesPerRow = 4 * r.size.width;

    CGColorSpaceRef space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
    CGContextRef context = CGBitmapContextCreate(NULL,
                                                 r.size.width,
                                                 r.size.height,
                                                 8, 
                                                 bytesPerRow, 
                                                 space,
                                                 kCGImageAlphaPremultipliedFirst);
    CGColorSpaceRelease(space);
    NSGraphicsContext *nsContext
      = [NSGraphicsContext graphicsContextWithGraphicsPort:context flipped:YES];
    [NSGraphicsContext saveGraphicsState];
    [NSGraphicsContext setCurrentContext:nsContext];
    [gradient drawInRect:NSMakeRect(0, 0, r.size.width, r.size.height) 
                   angle:270];
    [NSGraphicsContext restoreGraphicsState];
    
    CGImageRef cgImage = CGBitmapContextCreateImage(context);
    CGContextRelease(context);
    NSBitmapImageRep *rep = nil;
    if (cgImage) {
      rep = [[[NSBitmapImageRep alloc] initWithCGImage:cgImage]
             autorelease];
      CGImageRelease(cgImage);
    }
    
    NSImage *image = [[[NSImage alloc] initWithSize:NSSizeFromCGSize(r.size)]
                      autorelease];
    [image addRepresentation:rep];
    
    color = [NSColor colorWithPatternImage:image];
  }
  [self cacheValue:color forSelector:_cmd style:style active:active];
  return color;
}

- (NSGradient *)gradientForStyle:(GTMThemeStyle)style active:(BOOL)active {
  NSGradient *gradient = [self valueForSelector:_cmd style:style active:active];
  if (gradient) return gradient;
  
  BOOL useDarkColors = backgroundImage_ != nil || style == GTMThemeStyleWindow;
  
  NSUInteger uses[4];
  if (useDarkColors) {
    uses[0] = GTMColorationBaseHighlight;
    uses[1] = GTMColorationBaseMidtone;
    uses[2] = GTMColorationBaseShadow;
    uses[3] = GTMColorationBasePenumbra;
  } else {
    uses[0] = GTMColorationLightHighlight;
    uses[1] = GTMColorationLightMidtone;
    uses[2] = GTMColorationLightShadow;
    uses[3] = GTMColorationLightPenumbra;
  }
  NSColor *backgroundColor = [self backgroundColor];
  switch (style) {
    case GTMThemeStyleTabBarDeselected: {
      NSColor *startColor = [[backgroundColor gtm_colorAdjustedFor:uses[2] 
                                                             faded:!active] 
            colorWithAlphaComponent:0.667];
      NSColor *endColor = [[backgroundColor gtm_colorAdjustedFor:uses[2] 
                                                           faded:!active] 
                           colorWithAlphaComponent:0.667];  
      
      gradient = [[[NSGradient alloc] initWithStartingColor:startColor
                                                endingColor:endColor]
                  autorelease];
      break;
    }
    case GTMThemeStyleTabBarSelected: {
      NSColor *startColor = [backgroundColor gtm_colorAdjustedFor:uses[0]
                                                           faded:!active];
      NSColor *endColor = [backgroundColor gtm_colorAdjustedFor:uses[1] 
                                                         faded:!active];  
      gradient = [[[NSGradient alloc] initWithStartingColor:startColor
                                                endingColor:endColor] 
                  autorelease];
      break;
    }
    case GTMThemeStyleWindow: {
      CGFloat luminance = [backgroundColor gtm_luminance];
      
      // Adjust luminance so it never hits black
      if (luminance < 0.5) {
        CGFloat adjustment = (0.5 - luminance) / 1.5;
        backgroundColor
          = [backgroundColor gtm_colorByAdjustingLuminance:adjustment];
      }
      NSColor *startColor = [backgroundColor gtm_colorAdjustedFor:uses[1] 
                                                            faded:!active];
      NSColor *endColor = [backgroundColor gtm_colorAdjustedFor:uses[2] 
                                                          faded:!active]; 
      
      
      if (!active) {
        startColor = [startColor gtm_colorByAdjustingLuminance:0.1 
                                                    saturation:0.5];
        endColor = [endColor gtm_colorByAdjustingLuminance:0.1
                                                saturation:0.5];
        
      }
      gradient = [[[NSGradient alloc] initWithStartingColor:startColor
                                                endingColor:endColor]
                  autorelease];
      break;
    }
    case GTMThemeStyleToolBar: {
      NSColor *startColor = [backgroundColor gtm_colorAdjustedFor:uses[1] 
                                                           faded:!active];
      NSColor *endColor = [backgroundColor gtm_colorAdjustedFor:uses[2] 
                                                         faded:!active]; 
      gradient = [[[NSGradient alloc] initWithStartingColor:startColor
                                                endingColor:endColor]
                  autorelease];
      break;
    }
    case GTMThemeStyleToolBarButton: {
      NSColor *startColor = [backgroundColor gtm_colorAdjustedFor:uses[0]
                                                            faded:!active];
      NSColor *midColor = [backgroundColor gtm_colorAdjustedFor:uses[1]
                                                          faded:!active];  
      NSColor *endColor = [backgroundColor gtm_colorAdjustedFor:uses[2]
                                                          faded:!active];  
      NSColor *glowColor = [backgroundColor gtm_colorAdjustedFor:uses[3]
                                                           faded:!active];  
      
      gradient = [[[NSGradient alloc] initWithColorsAndLocations:
                   startColor, 0.0,
                   midColor, 0.5,
                   endColor, 0.9,
                   glowColor, 1.0,
                   nil] autorelease];
      break;
    }
    case GTMThemeStyleToolBarButtonPressed: {
      NSColor *startColor = [backgroundColor 
                             gtm_colorAdjustedFor:GTMColorationBaseShadow 
                             faded:!active];
      NSColor *endColor = [backgroundColor
                           gtm_colorAdjustedFor:GTMColorationBaseMidtone
                           faded:!active];  
      gradient = [[[NSGradient alloc] initWithStartingColor:startColor
                                                endingColor:endColor]
                  autorelease];
      break;
    }
    case GTMThemeStyleBookmarksBar: {
      NSColor *startColor = [backgroundColor gtm_colorAdjustedFor:uses[2]
                                                              faded:!active];
      NSColor *endColor = [backgroundColor gtm_colorAdjustedFor:uses[3]
                                                            faded:!active];  
      
      gradient = [[[NSGradient alloc] initWithStartingColor:startColor
                                                endingColor:endColor]
                  autorelease];
      
      break;
    }
    default:
      _GTMDevLog(@"Unexpected style: %d", style);
      break;
  }
  
  [self cacheValue:gradient forSelector:_cmd style:style active:active];
  return gradient;
}

- (NSColor *)strokeColorForStyle:(GTMThemeStyle)style active:(BOOL)active {
  NSColor *color = [self valueForSelector:_cmd style:style active:active];
  if (color) return color;
  NSColor *backgroundColor = [self backgroundColor];
  switch (style) {
    case GTMThemeStyleToolBarButton:
      color = [[backgroundColor gtm_colorAdjustedFor:GTMColorationDarkShadow
                                               faded:!active]
               colorWithAlphaComponent:0.3];
      break;
    case GTMThemeStyleToolBar:
    case GTMThemeStyleBookmarksBar:
    default:
      color = [[self backgroundColor] gtm_colorAdjustedFor:GTMColorationBaseShadow 
                                                     faded:!active];
      break;
  }
  
  [self cacheValue:color forSelector:_cmd style:style active:active];
  return color;
}

@end

#endif // MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
