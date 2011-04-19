//
//  GTMTestHTTPServer.m
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

#import "GTMTestHTTPServer.h"
#import "GTMHTTPServer.h"
#import "GTMRegex.h"

static NSArray *GetSubPatternsOfFirstStringMatchedByPattern(NSString *stringToSearch,
                                                            NSString *pattern) {
  GTMRegex *regex = [GTMRegex regexWithPattern:pattern];
  NSString *firstMatch = [regex firstSubStringMatchedInString:stringToSearch];
  NSArray *subPatterns = [regex subPatternsOfString:firstMatch];
  return subPatterns;
}

@implementation GTMTestHTTPServer

- (id)initWithDocRoot:(NSString *)docRoot {
  self = [super init];
  if (self) {
    docRoot_ = [docRoot copy];
    server_ = [[GTMHTTPServer alloc] initWithDelegate:self];
    NSError *error = nil;
    if ((docRoot == nil) || (![server_ start:&error])) {
      _GTMDevLog(@"Failed to start up the webserver (docRoot='%@', error=%@)",
                 docRoot_, error);
      [self release];
      return nil;
    }
  }
  return self;
}

- (void)dealloc {
  [docRoot_ release];
  [server_ release];
  [super dealloc];
}

- (uint16_t)port {
  return [server_ port];
}

- (GTMHTTPResponseMessage *)httpServer:(GTMHTTPServer *)server
                         handleRequest:(GTMHTTPRequestMessage *)request {
  _GTMDevAssert(server == server_, @"how'd we get a different server?!");
  UInt32 resultStatus = 0;
  NSData *data = nil;
  // clients should treat dates as opaque, generally
  NSString *modifiedDate = @"thursday";
  
  NSString *postString = @"";
  NSData *postData = [request body];
  if ([postData length] > 0) {
    postString = [[[NSString alloc] initWithData:postData
                                        encoding:NSUTF8StringEncoding] autorelease];
  }
  
  NSDictionary *allHeaders = [request allHeaderFieldValues];
  NSString *ifModifiedSince = [allHeaders objectForKey:@"If-Modified-Since"];
  NSString *authorization = [allHeaders objectForKey:@"Authorization"];
  NSString *path = [[request URL] absoluteString];
  
  if ([path hasSuffix:@".auth"]) {
    if (![authorization isEqualToString:@"GoogleLogin auth=GoodAuthToken"]) {
      GTMHTTPResponseMessage *response =
        [GTMHTTPResponseMessage emptyResponseWithCode:401];
      return response;
    } else {
      path = [path substringToIndex:[path length] - 5];
    }
  }
  
  NSString *overrideHeader = [allHeaders objectForKey:@"X-HTTP-Method-Override"];
  NSString *httpCommand = [request method];
  if ([httpCommand isEqualToString:@"POST"] && 
      [overrideHeader length] > 1) {
    httpCommand = overrideHeader;
  }
  NSArray *searchResult = nil;
  if ([path hasSuffix:@"/accounts/ClientLogin"]) {
    // it's a sign-in attempt; it's good unless the password is "bad" or
    // "captcha"
    
    // use regular expression to find the password
    NSString *password = @"";
    searchResult = GetSubPatternsOfFirstStringMatchedByPattern(path, @"Passwd=([^&\n]*)");
    if ([searchResult count] == 2) {
      password = [searchResult objectAtIndex:1];
    }
    
    if ([password isEqualToString:@"bad"]) {
      resultStatus = 403;
    } else if ([password isEqualToString:@"captcha"]) {
      NSString *loginToken = @"";
      NSString *loginCaptcha = @"";
      
      searchResult = GetSubPatternsOfFirstStringMatchedByPattern(postString, @"logintoken=([^&\n]*)");
      if ([searchResult count] == 2) {
        loginToken = [searchResult objectAtIndex:1];
      }
      
      searchResult = GetSubPatternsOfFirstStringMatchedByPattern(postString, @"logincaptcha=([^&\n]*)");
      if ([searchResult count] == 2) {
        loginCaptcha = [searchResult objectAtIndex:1];
      }
      
      if ([loginToken isEqualToString:@"CapToken"] &&
          [loginCaptcha isEqualToString:@"good"]) {
        resultStatus = 200;
      } else {
        // incorrect captcha token or answer provided
        resultStatus = 403;
      }
    } else {
      // valid username/password
      resultStatus = 200;
    }
  } else if ([httpCommand isEqualToString:@"DELETE"]) {
    // it's an object delete; read and return empty data
    resultStatus = 200;
  } else {
    // queries that have something like "?status=456" should fail with the
    // status code
    searchResult = GetSubPatternsOfFirstStringMatchedByPattern(path, @"status=([0-9]+)");
    if ([searchResult count] == 2) {
      resultStatus = [[searchResult objectAtIndex:1] intValue];
    } else if ([ifModifiedSince isEqualToString:modifiedDate]) {
      resultStatus = 304;
    } else {
      NSString *docPath = [docRoot_ stringByAppendingPathComponent:path];
      data = [NSData dataWithContentsOfFile:docPath];
      if (data) {
        resultStatus = 200;
      } else {
        resultStatus = 404; 
      }
    }
  }

  GTMHTTPResponseMessage *response =
    [GTMHTTPResponseMessage responseWithBody:data
                                 contentType:@"text/plain"
                                  statusCode:resultStatus];
  [response setValue:modifiedDate forHeaderField:@"Last-Modified"];
  [response setValue:[NSString stringWithFormat:@"TestCookie=%@", [path lastPathComponent]]
      forHeaderField:@"Set-Cookie"];
  return response;
}

@end
