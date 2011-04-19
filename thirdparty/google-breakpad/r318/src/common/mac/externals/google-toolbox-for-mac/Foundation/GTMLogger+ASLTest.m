//
//  GTMLogger+ASLTest.m
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

#import "GTMLogger+ASL.h"
#import "GTMSenTestCase.h"

@interface DummyASLClient : GTMLoggerASLClient
@end

static NSMutableArray *gDummyLog;  // weak

@implementation DummyASLClient

- (void)log:(NSString *)msg level:(int)level {
  NSString *line = [msg stringByAppendingFormat:@"@%d", level];
  [gDummyLog addObject:line];
}

@end


@interface GTMLogger_ASLTest : GTMTestCase
@end

@implementation GTMLogger_ASLTest

- (void)testCreation {
  GTMLogger *aslLogger = [GTMLogger standardLoggerWithASL];
  STAssertNotNil(aslLogger, nil);
  
  GTMLogASLWriter *writer = [GTMLogASLWriter aslWriter];
  STAssertNotNil(writer, nil);
}

- (void)testLogWriter {
  gDummyLog = [[[NSMutableArray alloc] init] autorelease];
  GTMLogASLWriter *writer = [[[GTMLogASLWriter alloc]
                              initWithClientClass:[DummyASLClient class]]
                             autorelease];
  

  STAssertNotNil(writer, nil);
  STAssertTrue([gDummyLog count] == 0, nil);

  // Log some messages
  [writer logMessage:@"unknown" level:kGTMLoggerLevelUnknown];
  [writer logMessage:@"debug" level:kGTMLoggerLevelDebug];
  [writer logMessage:@"info" level:kGTMLoggerLevelInfo];
  [writer logMessage:@"error" level:kGTMLoggerLevelError];
  [writer logMessage:@"assert" level:kGTMLoggerLevelAssert];
  
  // Inspect the logged message to make sure they were logged correctly. The 
  // dummy writer will save the messages w/ @level concatenated. The "level" 
  // will be the ASL level, not the GTMLogger level. GTMLogASLWriter will log
  // all 
  NSArray *expected = [NSArray arrayWithObjects:
                       @"unknown@5",
                       @"debug@5",
                       @"info@5",
                       @"error@3",
                       @"assert@1",
                       nil];
  
  STAssertEqualObjects(gDummyLog, expected, nil);
  
  gDummyLog = nil;
}

- (void)testASLClient {
  GTMLoggerASLClient *client = [[GTMLoggerASLClient alloc] init];
  STAssertNotNil(client, nil);
  [client release];
}

@end
