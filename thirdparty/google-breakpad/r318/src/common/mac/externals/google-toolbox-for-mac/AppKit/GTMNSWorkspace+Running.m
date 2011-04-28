//
//  GTMNSWorkspace+Running.m
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

#import "GTMNSWorkspace+Running.h"
#import <Carbon/Carbon.h>
#import <unistd.h>
#import "GTMGarbageCollection.h"
#import "GTMSystemVersion.h"

NSString *const kGTMWorkspaceRunningPSN = @"PSN";
NSString *const kGTMWorkspaceRunningFlavor = @"Flavor";
NSString *const kGTMWorkspaceRunningAttributes = @"Attributes";
NSString *const kGTMWorkspaceRunningParentPSN = @"ParentPSN";
NSString *const kGTMWorkspaceRunningFileType = @"FileType";
NSString *const kGTMWorkspaceRunningFileCreator = @"FileCreator";
NSString *const kGTMWorkspaceRunningPID = @"pid";
NSString *const kGTMWorkspaceRunningLSBackgroundOnly = @"LSBackgroundOnly";
NSString *const kGTMWorkspaceRunningLSUIElement = @"LSUIElement";
NSString *const kGTMWorkspaceRunningIsHidden = @"IsHiddenAttr";
NSString *const kGTMWorkspaceRunningCheckedIn = @"IsCheckedInAttr";
NSString *const kGTMWorkspaceRunningLSUIPresentationMode 
  = @"LSUIPresentationMode";
NSString *const kGTMWorkspaceRunningBundlePath = @"BundlePath";
NSString *const kGTMWorkspaceRunningBundleVersion = @"CFBundleVersion";

@implementation NSWorkspace (GTMWorkspaceRunningAdditions)

/// Returns a YES/NO if a process w/ the given identifier is running
- (BOOL)gtm_isAppWithIdentifierRunning:(NSString *)identifier {
  if ([identifier length] == 0) return NO;
  NSArray *launchedApps = [self launchedApplications];
  NSArray *buildIDs 
    = [launchedApps valueForKey:@"NSApplicationBundleIdentifier"];
  return [buildIDs containsObject:identifier];
}

- (NSDictionary *)gtm_processInfoDictionaryForPID:(pid_t)pid {
  NSDictionary *dict = nil;
  ProcessSerialNumber psn;
  if (GetProcessForPID(pid, &psn) == noErr) {
    dict = [self gtm_processInfoDictionaryForPSN:&psn];
  }
  return dict;
}

- (NSDictionary *)gtm_processInfoDictionaryForPSN:(ProcessSerialNumberPtr)psn {
  NSDictionary *dict = nil;
  if (psn) {
    CFDictionaryRef cfDict 
      = ProcessInformationCopyDictionary(psn, 
                                         kProcessDictionaryIncludeAllInformationMask);
    dict = GTMCFAutorelease(cfDict);
  }
  return dict;
}

- (NSDictionary *)gtm_processInfoDictionary {
  NSDictionary *dict = nil;
  ProcessSerialNumber selfNumber;
  if (MacGetCurrentProcess(&selfNumber) == noErr) {
    dict = [self gtm_processInfoDictionaryForPSN:&selfNumber];
  }
  return dict;
}

- (NSDictionary *)gtm_processInfoDictionaryForActiveApp {
  NSDictionary *processDict = nil;
  ProcessSerialNumber psn;
  OSStatus status = GetFrontProcess(&psn);
  if (status == noErr) {
    processDict = [self gtm_processInfoDictionaryForPSN:&psn];
  }
  return processDict;
}

- (BOOL)gtm_wasLaunchedAsLoginItem {
  // If the launching process was 'loginwindow', we were launched as a login 
  // item
  return [self gtm_wasLaunchedByProcess:@"com.apple.loginwindow"];
}

- (BOOL)gtm_wasLaunchedByProcess:(NSString*)bundleid {
  BOOL wasLaunchedByProcess = NO;
  NSDictionary *processInfo = [self gtm_processInfoDictionary];
  if (processInfo) {
    NSNumber *processNumber 
      = [processInfo objectForKey:kGTMWorkspaceRunningParentPSN];
    ProcessSerialNumber parentPSN 
      = [self gtm_numberToProcessSerialNumber:processNumber];
    NSDictionary *parentProcessInfo 
      = [self gtm_processInfoDictionaryForPSN:&parentPSN];
    NSString *parentBundle 
      = [parentProcessInfo objectForKey:kGTMWorkspaceRunningBundleIdentifier];
    wasLaunchedByProcess 
      = [parentBundle isEqualToString:bundleid];
  }
  return wasLaunchedByProcess;
}

- (BOOL)gtm_processSerialNumber:(ProcessSerialNumber*)outPSN 
                   withBundleID:(NSString*)bundleID {
  if (!outPSN || [bundleID length] == 0) {
    return NO;
  }
  
  NSArray *apps = [self launchedApplications];
  
  NSEnumerator *enumerator = [apps objectEnumerator];
  NSDictionary *dict;
  
  while ((dict = [enumerator nextObject])) {
    NSString *nextID = [dict objectForKey:@"NSApplicationBundleIdentifier"];
    
    if ([nextID isEqualToString:bundleID]) {
      NSNumber *psn 
        = [dict objectForKey:@"NSApplicationProcessSerialNumberLow"];
      outPSN->lowLongOfPSN = [psn unsignedIntValue];
      
      psn = [dict objectForKey:@"NSApplicationProcessSerialNumberHigh"];
      outPSN->highLongOfPSN = [psn unsignedIntValue];
      
      return YES;
    }
  }
  
  return NO;
}

- (ProcessSerialNumber)gtm_numberToProcessSerialNumber:(NSNumber*)number {
  // There is a bug in Tiger where they were packing ProcessSerialNumbers
  // incorrectly into the longlong that they stored in the dictionary.
  // This fixes it.
  ProcessSerialNumber outPSN = { kNoProcess, kNoProcess};
  if (number) {
    long long temp = [number longLongValue];
    UInt32 hi = (UInt32)((temp >> 32) & 0x00000000FFFFFFFFLL);
    UInt32 lo = (UInt32)((temp >> 0) & 0x00000000FFFFFFFFLL);
#if MAC_OS_X_VERSION_MIN_REQUIRED >= MAC_OS_X_VERSION_10_5
    outPSN.highLongOfPSN = hi;
    outPSN.lowLongOfPSN = lo;
#else  // MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_4
    if ([GTMSystemVersion isLeopardOrGreater]) {
      outPSN.highLongOfPSN = hi;
      outPSN.lowLongOfPSN = lo;
    } else {
#if TARGET_RT_BIG_ENDIAN
      outPSN.highLongOfPSN = hi;
      outPSN.lowLongOfPSN = lo;
#else
      outPSN.highLongOfPSN = lo;
      outPSN.lowLongOfPSN = hi;
#endif  // TARGET_RT_BIG_ENDIAN
    }
#endif  // MAC_OS_X_VERSION_MIN_REQUIRED <= MAC_OS_X_VERSION_10_4
  }
  return outPSN;
}
@end
