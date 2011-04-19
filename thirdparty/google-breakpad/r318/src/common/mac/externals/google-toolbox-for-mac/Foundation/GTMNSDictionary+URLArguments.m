//
//  GTMNSDictionary+URLArguments.m
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

#import "GTMNSDictionary+URLArguments.h"
#import "GTMNSString+URLArguments.h"
#import "GTMMethodCheck.h"
#import "GTMDefines.h"

@implementation NSDictionary (GTMNSDictionaryURLArgumentsAdditions)

GTM_METHOD_CHECK(NSString, gtm_stringByEscapingForURLArgument);

- (NSString *)gtm_httpArgumentsString {
  NSMutableArray* arguments = [NSMutableArray arrayWithCapacity:[self count]];
  NSString* key;
  GTM_FOREACH_KEY(key, self) {
    [arguments addObject:[NSString stringWithFormat:@"%@=%@",
                          [key gtm_stringByEscapingForURLArgument],
                          [[[self objectForKey:key] description] gtm_stringByEscapingForURLArgument]]];
  }

  return [arguments componentsJoinedByString:@"&"];
}

@end
