//
//  GTMTestHTTPServer.h
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

#import <Foundation/Foundation.h>

@class GTMHTTPServer;

// This is a HTTP Server that can respond to certain requests that look like
// Google service logins.  It takes extra url arguments to tell it what to
// return for testing the code using it.  See GTMHTTPFetcherTest for an example
// of its usage.
@interface GTMTestHTTPServer : NSObject {
  NSString *docRoot_;
  GTMHTTPServer *server_;
}

// Any url that isn't a specific server request (login, etc.), will be fetched
// off |docRoot| (to allow canned repsonses).
- (id)initWithDocRoot:(NSString *)docRoot;

// fetch the port the server is running on
- (uint16_t)port;

@end
