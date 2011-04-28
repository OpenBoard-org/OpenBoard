//
//  GTMIPhoneUnitTestDelegate.m
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

#import "GTMIPhoneUnitTestDelegate.h"

#import "GTMDefines.h"
#if !GTM_IPHONE_SDK
#error GTMIPhoneUnitTestDelegate for iPhone only
#endif
#import <objc/runtime.h>
#import <stdio.h>
#import <UIKit/UIKit.h>
#import "GTMSenTestCase.h"

// Used for sorting methods below
static int MethodSort(const void *a, const void *b) {
  const char *nameA = sel_getName(method_getName(*(Method*)a));
  const char *nameB = sel_getName(method_getName(*(Method*)b));
  return strcmp(nameA, nameB);
}

// Return YES if class is subclass (1 or more generations) of SenTestCase
static BOOL IsTestFixture(Class aClass) {
  BOOL iscase = NO;
  Class testCaseClass = [SenTestCase class];
  Class superclass;
  for (superclass = aClass; 
       !iscase && superclass; 
       superclass = class_getSuperclass(superclass)) {
    iscase = superclass == testCaseClass ? YES : NO;
  }
  return iscase;
}

@implementation GTMIPhoneUnitTestDelegate

// Run through all the registered classes and run test methods on any
// that are subclasses of SenTestCase. Terminate the application upon
// test completion.
- (void)applicationDidFinishLaunching:(UIApplication *)application {
  [self runTests];
  
  if (!getenv("GTM_DISABLE_TERMINATION")) {
    // To help using xcodebuild, make the exit status 0/1 to signal the tests
    // success/failure.
    int exitStatus = (([self totalFailures] == 0U) ? 0 : 1);
    exit(exitStatus);
  }
}

// Run through all the registered classes and run test methods on any
// that are subclasses of SenTestCase. Print results and run time to
// the default output.
- (void)runTests {
  int count = objc_getClassList(NULL, 0);
  NSMutableData *classData
    = [NSMutableData dataWithLength:sizeof(Class) * count];
  Class *classes = (Class*)[classData mutableBytes];
  _GTMDevAssert(classes, @"Couldn't allocate class list");
  objc_getClassList(classes, count);
  totalFailures_ = 0;
  totalSuccesses_ = 0;
  NSString *suiteName = [[NSBundle mainBundle] bundlePath];
  NSDate *suiteStartDate = [NSDate date];
  NSString *suiteStartString
    = [NSString stringWithFormat:@"Test Suite '%@' started at %@\n",
                                 suiteName, suiteStartDate];
  fputs([suiteStartString UTF8String], stderr);
  fflush(stderr);
  for (int i = 0; i < count; ++i) {
    Class currClass = classes[i];
    if (IsTestFixture(currClass)) {
      NSDate *fixtureStartDate = [NSDate date];
      NSString *fixtureName = NSStringFromClass(currClass);
      NSString *fixtureStartString
        = [NSString stringWithFormat:@"Test Suite '%@' started at %@\n",
                                     fixtureName, fixtureStartDate];
      int fixtureSuccesses = 0;
      int fixtureFailures = 0;
      fputs([fixtureStartString UTF8String], stderr);
      fflush(stderr);
      id testcase = [[currClass alloc] init];
      _GTMDevAssert(testcase, @"Unable to instantiate Test Suite: '%@'\n",
                    fixtureName);
      unsigned int methodCount;
      Method *methods = class_copyMethodList(currClass, &methodCount);
      if (!methods) {
        // If the class contains no methods, head on to the next class
        NSString *output = [NSString stringWithFormat:@"Test Suite '%@' "
                            @"finished at %@.\nExecuted 0 tests, with 0 "
                            @"failures (0 unexpected) in 0 (0) seconds\n",
                            fixtureName, fixtureStartDate];
        
        fputs([output UTF8String], stderr);
        continue;
      }
      // This handles disposing of methods for us even if an
      // exception should fly. 
      [NSData dataWithBytesNoCopy:methods
                           length:sizeof(Method) * methodCount];
      // Sort our methods so they are called in Alphabetical order just
      // because we can.
      qsort(methods, methodCount, sizeof(Method), MethodSort);
      for (size_t j = 0; j < methodCount; ++j) {
        Method currMethod = methods[j];
        SEL sel = method_getName(currMethod);
        char *returnType = NULL;
        const char *name = sel_getName(sel);
        // If it starts with test, takes 2 args (target and sel) and returns
        // void run it.
        if (strstr(name, "test") == name) {
          returnType = method_copyReturnType(currMethod);
          if (returnType) {
            // This handles disposing of returnType for us even if an
            // exception should fly. Length +1 for the terminator, not that
            // the length really matters here, as we never reference inside
            // the data block.
            [NSData dataWithBytesNoCopy:returnType
                                 length:strlen(returnType) + 1];
          }
        }
        if (returnType  // True if name starts with "test"
            && strcmp(returnType, @encode(void)) == 0
            && method_getNumberOfArguments(currMethod) == 2) {
          BOOL failed = NO;
          NSDate *caseStartDate = [NSDate date];
          @try {
            [testcase performTest:sel];
          } @catch (NSException *exception) {
            failed = YES;
          }
          if (failed) {
            fixtureFailures += 1;
          } else {
            fixtureSuccesses += 1;
          }
          NSTimeInterval caseEndTime
            = [[NSDate date] timeIntervalSinceDate:caseStartDate];
          NSString *caseEndString
            = [NSString stringWithFormat:@"Test Case '-[%@ %s]' %@ (%0.3f "
                                         @"seconds).\n",
                                         fixtureName, name,
                                         failed ? @"failed" : @"passed",
                                         caseEndTime];
          fputs([caseEndString UTF8String], stderr);
          fflush(stderr);
        }
      }
      [testcase release];
      NSDate *fixtureEndDate = [NSDate date];
      NSTimeInterval fixtureEndTime
        = [fixtureEndDate timeIntervalSinceDate:fixtureStartDate];
      NSString *fixtureEndString
        = [NSString stringWithFormat:@"Test Suite '%@' finished at %@.\n"
                                     @"Executed %d tests, with %d failures (%d "
                                     @"unexpected) in %0.3f (%0.3f) seconds\n\n",
                                     fixtureName, fixtureEndDate,
                                     fixtureSuccesses + fixtureFailures, 
                                     fixtureFailures, fixtureFailures,
                                     fixtureEndTime, fixtureEndTime];
      
      fputs([fixtureEndString UTF8String], stderr);
      fflush(stderr);
      totalSuccesses_ += fixtureSuccesses;
      totalFailures_ += fixtureFailures;      
    }
  }
  NSDate *suiteEndDate = [NSDate date];
  NSTimeInterval suiteEndTime
    = [suiteEndDate timeIntervalSinceDate:suiteStartDate];
  NSString *suiteEndString
    = [NSString stringWithFormat:@"Test Suite '%@' finished at %@.\n"
                                 @"Executed %d tests, with %d failures (%d "
                                 @"unexpected) in %0.3f (%0.3f) seconds\n\n",
                                 suiteName, suiteEndDate,
                                 totalSuccesses_ + totalFailures_, 
                                 totalFailures_, totalFailures_,
                                 suiteEndTime, suiteEndTime];
  fputs([suiteEndString UTF8String], stderr);
  fflush(stderr);
}

- (NSUInteger)totalSuccesses {
  return totalSuccesses_;
}

- (NSUInteger)totalFailures {
  return totalFailures_;
}

@end
