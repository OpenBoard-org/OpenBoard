//
//  GTMNSBezierPath+RoundRect.h
//
//  Category for adding utility functions for creating
//  round rectangles.
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

#import "GTMNSBezierPath+RoundRect.h"

@implementation NSBezierPath (GTMBezierPathRoundRectAdditions)


+ (NSBezierPath *)gtm_bezierPathWithRoundRect:(NSRect)rect
                                 cornerRadius:(CGFloat)radius {
  NSBezierPath *bezier = [NSBezierPath bezierPath];
  [bezier gtm_appendBezierPathWithRoundRect:rect cornerRadius:radius];
  return bezier;
}


- (void)gtm_appendBezierPathWithRoundRect:(NSRect)rect
                             cornerRadius:(CGFloat)radius {
  if (!NSIsEmptyRect(rect)) {
    if (radius > 0.0) {
      // Clamp radius to be no larger than half the rect's width or height.
      radius = MIN(radius, 0.5 * MIN(rect.size.width, rect.size.height));
      
      NSPoint topLeft = NSMakePoint(NSMinX(rect), NSMaxY(rect));
      NSPoint topRight = NSMakePoint(NSMaxX(rect), NSMaxY(rect));
      NSPoint bottomRight = NSMakePoint(NSMaxX(rect), NSMinY(rect));
      
      [self moveToPoint:NSMakePoint(NSMidX(rect), NSMaxY(rect))];
      [self appendBezierPathWithArcFromPoint:topLeft
                                     toPoint:rect.origin
                                      radius:radius];
      [self appendBezierPathWithArcFromPoint:rect.origin
                                     toPoint:bottomRight
                                      radius:radius];
      [self appendBezierPathWithArcFromPoint:bottomRight
                                     toPoint:topRight
                                      radius:radius];
      [self appendBezierPathWithArcFromPoint:topRight
                                     toPoint:topLeft
                                      radius:radius];
      [self closePath];
    } else {
      // When radius <= 0.0, use plain rectangle.
      [self appendBezierPathWithRect:rect];
    }
  }
}


@end
