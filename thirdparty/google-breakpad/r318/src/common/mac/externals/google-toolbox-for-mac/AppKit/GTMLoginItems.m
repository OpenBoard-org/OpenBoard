//
//  GTMLoginItems.m
//  Based on AELoginItems from DTS.
//
//  Copyright 2007-2008 Google Inc.
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

#import "GTMLoginItems.h"
#import "GTMDefines.h"

#include <Carbon/Carbon.h>

// Exposed constants
NSString * const kGTMLoginItemsNameKey = @"Name";
NSString * const kGTMLoginItemsPathKey = @"Path";
NSString * const kGTMLoginItemsHiddenKey = @"Hide";

@interface GTMLoginItems (PrivateMethods)
+ (NSInteger)indexOfLoginItemWithValue:(id)value
                                forKey:(NSString *)key
                            loginItems:(NSArray *)items;
+ (BOOL)compileAndRunScript:(NSString *)script
                  withError:(NSError **)errorInfo;
@end

@implementation GTMLoginItems (PrivateMethods)

+ (NSInteger)indexOfLoginItemWithValue:(id)value
                                forKey:(NSString *)key
                            loginItems:(NSArray *)items {
  if (!value || !key || !items) return NSNotFound;
  NSDictionary *item = nil;
  NSInteger found = -1;
  GTM_FOREACH_OBJECT(item, items) {
    ++found;
    id itemValue = [item objectForKey:key];
    if (itemValue && [itemValue isEqual:value]) {
      return found;
    }
  }
  return NSNotFound;
}

+ (BOOL)compileAndRunScript:(NSString *)script
                  withError:(NSError **)errorInfo {
  if ([script length] == 0) {
    // COV_NF_START - no real way to test this
    if (errorInfo)
      *errorInfo = [NSError errorWithDomain:@"GTMLoginItems" code:-90 userInfo:nil];
    return NO;
    // COV_NF_END
  }
  NSAppleScript *query = [[[NSAppleScript alloc] initWithSource:script] autorelease];
  NSDictionary *errDict = nil;
  if ( ![query compileAndReturnError:&errDict]) {
    // COV_NF_START - no real way to test this
    if (errorInfo)
      *errorInfo = [NSError errorWithDomain:@"GTMLoginItems" code:-91 userInfo:errDict];
    return NO;
    // COV_NF_END
  }
  NSAppleEventDescriptor *scriptResult = [query executeAndReturnError:&errDict];
  if (!scriptResult) {
    // COV_NF_START - no real way to test this
    if (errorInfo)
      *errorInfo = [NSError errorWithDomain:@"GTMLoginItems" code:-92 userInfo:errDict];
    return NO;
    // COV_NF_END
  }
  // we don't process the result
  return YES;
}

@end

@implementation GTMLoginItems

+ (NSArray*)loginItems:(NSError **)errorInfo {
  NSDictionary *errDict = nil;
  // get the script compiled and saved off
  static NSAppleScript *query = nil;
  if (!query) {
    NSString *querySource = @"tell application \"System Events\" to get properties of login items";
    query = [[NSAppleScript alloc] initWithSource:querySource];
    if ( ![query compileAndReturnError:&errDict]) {
      // COV_NF_START - no real way to test this
      if (errorInfo)
        *errorInfo = [NSError errorWithDomain:@"GTMLoginItems" code:-1 userInfo:errDict];
      [query release];
      query = nil;
      return nil;
      // COV_NF_END
    }
  }
  // run the script
  NSAppleEventDescriptor *scriptResult = [query executeAndReturnError:&errDict];
  if (!scriptResult) {
    // COV_NF_START - no real way to test this
    if (errorInfo)
      *errorInfo = [NSError errorWithDomain:@"GTMLoginItems" code:-2 userInfo:errDict];
    return nil;
    // COV_NF_END
  }
  // build our results
  NSMutableArray *result = [NSMutableArray array];
  NSInteger count = [scriptResult numberOfItems];
  for (NSInteger i = 0; i < count; ++i) {
    NSAppleEventDescriptor *aeItem = [scriptResult descriptorAtIndex:i+1];
    NSAppleEventDescriptor *hidn = [aeItem descriptorForKeyword:kAEHidden];
    NSAppleEventDescriptor *nam = [aeItem descriptorForKeyword:pName];
    NSAppleEventDescriptor *ppth = [aeItem descriptorForKeyword:'ppth'];
    NSMutableDictionary *item = [NSMutableDictionary dictionary];
    if (hidn && [hidn booleanValue]) {
      [item setObject:[NSNumber numberWithBool:YES] forKey:kGTMLoginItemsHiddenKey];
    }
    if (nam) {
      NSString *name = [nam stringValue];
      if (name) {
        [item setObject:name forKey:kGTMLoginItemsNameKey];
      }
    }
    if (ppth) {
      NSString *path = [ppth stringValue];
      if (path) {
        [item setObject:path forKey:kGTMLoginItemsPathKey];
      }
    }
    [result addObject:item];
  }
  
  return result;
}

+ (BOOL)pathInLoginItems:(NSString *)path {
  NSArray *loginItems = [self loginItems:nil];
  NSInteger itemIndex = [self indexOfLoginItemWithValue:path
                                                 forKey:kGTMLoginItemsPathKey
                                             loginItems:loginItems];
  return (itemIndex != NSNotFound) ? YES : NO;
}

+ (BOOL)itemWithNameInLoginItems:(NSString *)name {
  NSArray *loginItems = [self loginItems:nil];
  NSInteger itemIndex = [self indexOfLoginItemWithValue:name
                                                 forKey:kGTMLoginItemsNameKey
                                             loginItems:loginItems];
  return (itemIndex != NSNotFound) ? YES : NO;
}

+ (void)addPathToLoginItems:(NSString*)path hide:(BOOL)hide {
  if (!path) return;
  // make sure it isn't already there
  if ([self pathInLoginItems:path]) return;
  // now append it
  NSString *scriptSource =
    [NSString stringWithFormat:
      @"tell application \"System Events\" to make new login item with properties { path:\"%s\", hidden:%s } at end",
      [path UTF8String],
      (hide ? "yes" : "no")];
  [self compileAndRunScript:scriptSource withError:nil];
}

+ (void)removePathFromLoginItems:(NSString*)path {
  if ([path length] == 0) return;
  NSString *scriptSource =
    [NSString stringWithFormat:
      @"tell application \"System Events\" to delete (login items whose path is \"%s\")",
      [path UTF8String]];
  [self compileAndRunScript:scriptSource withError:nil];
}

+ (void)removeItemWithNameFromLoginItems:(NSString *)name {
  if ([name length] == 0) return;
  NSString *scriptSource =
    [NSString stringWithFormat:
      @"tell application \"System Events\" to delete (login items whose name is \"%s\")",
      [name UTF8String]];
  [self compileAndRunScript:scriptSource withError:nil];
}

@end
