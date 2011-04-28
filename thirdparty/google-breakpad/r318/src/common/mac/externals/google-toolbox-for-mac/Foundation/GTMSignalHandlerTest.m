//
//  GTMSignalHandlerTest.m
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
#import "GTMSignalHandler.h"
#import "GTMUnitTestDevLog.h"

@interface GTMSignalHandlerTest : GTMTestCase
@end

@interface SignalCounter : NSObject {
 @public
  int signalCount_;
  int lastSeenSignal_;
}
- (int)count;
- (int)lastSeen;
- (void)countSignal:(int)signo;
+ (id)signalCounter;
@end // SignalCounter

@implementation SignalCounter
+ (id)signalCounter {
  return [[[[self class] alloc] init] autorelease];
}
- (int)count {
  return signalCount_;
}
- (int)lastSeen {
  return lastSeenSignal_;
}
// Count the number of times this signal handler has fired.
- (void)countSignal:(int)signo {
  signalCount_++;
  lastSeenSignal_ = signo;
}
@end

@implementation GTMSignalHandlerTest

// Spin the run loop so that the kqueue event notifications will get delivered.
- (void)giveSomeLove {
  NSDate *endTime = [NSDate dateWithTimeIntervalSinceNow:0.5];
  [[NSRunLoop currentRunLoop] runUntilDate:endTime];
}

- (void)testNillage {
  GTMSignalHandler *handler;

  // Just an init should return nil.
  [GTMUnitTestDevLog expectString:@"Don't call init, use "
                                  @"initWithSignal:target:action:"];
  handler = [[[GTMSignalHandler alloc] init] autorelease];
  STAssertNil(handler, nil);

  // Zero signal should return nil as well.
  handler = [[[GTMSignalHandler alloc] 
              initWithSignal:0
                      target:self
                      action:@selector(nomnomnom:)] autorelease];
  STAssertNil(handler, nil);

}

- (void)testSingleHandler {
  SignalCounter *counter = [SignalCounter signalCounter];
  STAssertNotNil(counter, nil);
  
  GTMSignalHandler *handler = [[GTMSignalHandler alloc]
                                initWithSignal:SIGWINCH
                                        target:counter
                                        action:@selector(countSignal:)];
  STAssertNotNil(handler, nil);
  raise(SIGWINCH);
  [self giveSomeLove];

  STAssertEquals([counter count], 1, nil);
  STAssertEquals([counter lastSeen], SIGWINCH, nil);

  raise(SIGWINCH);
  [self giveSomeLove];

  STAssertEquals([counter count], 2, nil);
  STAssertEquals([counter lastSeen], SIGWINCH, nil);

  // create a second one to make sure we're seding data where we want
  SignalCounter *counter2 = [SignalCounter signalCounter];
  STAssertNotNil(counter2, nil);
  [[[GTMSignalHandler alloc] initWithSignal:SIGUSR1
                                     target:counter2
                                     action:@selector(countSignal:)] autorelease];
  
  raise(SIGUSR1);
  [self giveSomeLove];
  
  STAssertEquals([counter count], 2, nil);
  STAssertEquals([counter lastSeen], SIGWINCH, nil);
  STAssertEquals([counter2 count], 1, nil);
  STAssertEquals([counter2 lastSeen], SIGUSR1, nil);

  [handler release];

  // The signal is still ignored (so we shouldn't die), but the
  // the handler method should not get called.
  raise(SIGWINCH);

  STAssertEquals([counter count], 2, nil);
  STAssertEquals([counter lastSeen], SIGWINCH, nil);
  STAssertEquals([counter2 count], 1, nil);
  STAssertEquals([counter2 lastSeen], SIGUSR1, nil);

}

- (void)testIgnore {
  SignalCounter *counter = [SignalCounter signalCounter];
  STAssertNotNil(counter, nil);

  [[[GTMSignalHandler alloc] initWithSignal:SIGUSR1
                                     target:counter
                                     action:NULL] autorelease];

  raise(SIGUSR1);
  [self giveSomeLove];
  STAssertEquals([counter count], 0, nil);

}

@end
