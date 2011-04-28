//
//  GTMAbstractDOListenerTest.m
//
//  Copyright 2006-2009 Google Inc.
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
#import "GTMAbstractDOListener.h"

// Needed for GTMIsGarbageCollectionEnabled
#import "GTMGarbageCollection.h"

@interface GTMAbstractDOListenerTest : GTMTestCase
@end

// TODO: we need to add more tests for this class.  Examples: send messages and
// send messages that are in the protocol.

@implementation GTMAbstractDOListenerTest

- (void)testAbstractDOListenerRelease {
  NSUInteger listenerCount = [[GTMAbstractDOListener allListeners] count];
  GTMAbstractDOListener *listener =
    [[GTMAbstractDOListener alloc] initWithRegisteredName:@"FOO"
                                                 protocol:@protocol(NSObject)
                                                     port:[NSPort port]];
  STAssertNotNil(listener, nil);

  // We throw an autorelease pool here because allStores does a couple of
  // autoreleased retains on us which would screws up our retain count
  // numbers.
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  STAssertEquals([[GTMAbstractDOListener allListeners] count],
                 listenerCount + 1, nil);
  [pool drain];

  if (!GTMIsGarbageCollectionEnabled()) {
    // Not much point with GC on.
    STAssertEquals([listener retainCount], (NSUInteger)1, nil);
  }

  [listener release];
  if (!GTMIsGarbageCollectionEnabled()) {
    STAssertEquals([[GTMAbstractDOListener allListeners] count], listenerCount,
                   nil);
  }
}

@end
