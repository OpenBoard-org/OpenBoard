//
//  GTMFourCharCodeTest.m
//
//  Copyright 2008 Google Inc.
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
#import "GTMFourCharCode.h"

@interface GTMFourCharCodeTest : GTMTestCase
@end

@implementation GTMFourCharCodeTest

const FourCharCode kGTMHighMacOSRomanCode = 0xA5A8A9AA; // '•®©™'

- (void)testFourCharCode {
  GTMFourCharCode *fcc = [GTMFourCharCode fourCharCodeWithString:@"APPL"];
  STAssertNotNil(fcc, nil);
  STAssertEqualObjects([fcc stringValue], @"APPL", nil);
  STAssertEqualObjects([fcc numberValue], [NSNumber numberWithUnsignedInt:'APPL'], nil);
  STAssertEquals([fcc fourCharCode], (FourCharCode)'APPL', nil);

  STAssertEqualObjects([fcc description], @"GTMFourCharCode - APPL (0x4150504C)", nil);
  STAssertEquals([fcc hash], (NSUInteger)'APPL', nil);
  
  GTMFourCharCode *fcc2 = [GTMFourCharCode fourCharCodeWithFourCharCode:kGTMHighMacOSRomanCode];
  STAssertNotNil(fcc2, nil);
  STAssertEqualObjects([fcc2 stringValue], @"•®©™", nil);
  STAssertEqualObjects([fcc2 numberValue], [NSNumber numberWithUnsignedInt:kGTMHighMacOSRomanCode], nil);
  STAssertEquals([fcc2 fourCharCode], (FourCharCode)kGTMHighMacOSRomanCode, nil);
  
  STAssertNotEqualObjects(fcc, fcc2, nil);
 
  NSData *data = [NSKeyedArchiver archivedDataWithRootObject:fcc];
  STAssertNotNil(data, nil);
  fcc2 = (GTMFourCharCode*)[NSKeyedUnarchiver unarchiveObjectWithData:data];
  STAssertNotNil(fcc2, nil);
  STAssertEqualObjects(fcc, fcc2, nil);
  
  fcc = [[[GTMFourCharCode alloc] initWithFourCharCode:'\?\?\?\?'] autorelease];
  STAssertNotNil(fcc, nil);
  STAssertEqualObjects([fcc stringValue], @"????", nil);
  STAssertEqualObjects([fcc numberValue], [NSNumber numberWithUnsignedInt:'\?\?\?\?'], nil);
  STAssertEquals([fcc fourCharCode], (FourCharCode)'\?\?\?\?', nil);

  fcc = [[[GTMFourCharCode alloc] initWithString:@"????"] autorelease];
  STAssertNotNil(fcc, nil);
  STAssertEqualObjects([fcc stringValue], @"????", nil);
  STAssertEqualObjects([fcc numberValue], [NSNumber numberWithUnsignedInt:'\?\?\?\?'], nil);
  STAssertEquals([fcc fourCharCode], (FourCharCode)'\?\?\?\?', nil);

  fcc = [GTMFourCharCode fourCharCodeWithFourCharCode:1];
  STAssertNotNil(fcc, nil);
  STAssertEqualObjects([fcc stringValue], @"\0\0\0\1", nil);
  STAssertEqualObjects([fcc numberValue], [NSNumber numberWithUnsignedInt:1], nil);
  STAssertEquals([fcc fourCharCode], (FourCharCode)1, nil);
  
  
  fcc = [GTMFourCharCode fourCharCodeWithString:@"BADDSTRING"];
  STAssertNil(fcc, nil);
}

- (void)testStringWithCode {
  STAssertEqualObjects([GTMFourCharCode stringWithFourCharCode:'APPL'], @"APPL", nil);
  STAssertEqualObjects([GTMFourCharCode stringWithFourCharCode:1], @"\0\0\0\1", nil);
  STAssertEqualObjects([GTMFourCharCode stringWithFourCharCode:kGTMHighMacOSRomanCode], @"•®©™", nil);
}

@end
