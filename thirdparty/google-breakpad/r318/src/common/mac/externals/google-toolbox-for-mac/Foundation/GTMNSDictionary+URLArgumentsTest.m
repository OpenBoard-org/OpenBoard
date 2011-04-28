//
//  GTMNSDictionary+URLArgumentsTest.m
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

#import "GTMSenTestCase.h"
#import "GTMNSDictionary+URLArguments.h"
#import "GTMDefines.h"

@interface GTMNSDictionary_URLArgumentsTest : GTMTestCase
@end

@implementation GTMNSDictionary_URLArgumentsTest

- (void)testArgumentsString {
  STAssertEqualObjects([[NSDictionary dictionary] gtm_httpArgumentsString], @"",
                       @"- empty dictionary should give an empty string");
  STAssertEqualObjects([[NSDictionary dictionaryWithObject:@"123" forKey:@"abc"] gtm_httpArgumentsString],
                        @"abc=123",
                        @"- simple one-pair dictionary should work");
  NSDictionary* arguments = [NSDictionary dictionaryWithObjectsAndKeys:
                             @"1+1!=3 & 2*6/3=4", @"complex",
                                   @"specialkey", @"a+b",
                                                  nil];
  NSString* argumentString = [arguments gtm_httpArgumentsString];
  // check for individual pieces since order is not guaranteed
  NSString* component1 = @"a%2Bb=specialkey";
  NSString* component2 = @"complex=1%2B1%21%3D3%20%26%202%2A6%2F3%3D4";
  STAssertNotEquals([argumentString rangeOfString:component1].location, (NSUInteger)NSNotFound,
                    @"- '%@' not found in '%@'", component1, argumentString);
  STAssertNotEquals([argumentString rangeOfString:component2].location, (NSUInteger)NSNotFound,
                    @"- '%@' not found in '%@'", component2, argumentString);
  STAssertNotEquals([argumentString rangeOfString:@"&"].location, (NSUInteger)NSNotFound,
                    @"- special characters should be escaped");
  STAssertNotEquals([argumentString characterAtIndex:0], (unichar)'&',
                    @"- there should be no & at the beginning of the string");
  STAssertNotEquals([argumentString characterAtIndex:([argumentString length] - 1)], (unichar)'&',
                    @"- there should be no & at the end of the string");
}

@end
