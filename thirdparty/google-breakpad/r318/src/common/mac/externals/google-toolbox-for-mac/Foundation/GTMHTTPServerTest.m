//
//  GTMHTTPServerTest.m
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

#import <netinet/in.h>
#import <sys/socket.h>
#import <unistd.h>
#import "GTMSenTestCase.h"
#import "GTMUnitTestDevLog.h"
#import "GTMHTTPServer.h"
#import "GTMRegex.h"

@interface GTMHTTPServerTest : GTMTestCase {
  NSData *fetchedData_;
}
@end

@interface GTMHTTPServerTest (PrivateMethods)
- (NSData *)fetchFromPort:(unsigned short)port
                  payload:(NSString *)payload
                chunkSize:(NSUInteger)chunkSize;
- (NSFileHandle *)fileHandleSendingToPort:(unsigned short)port
                                  payload:(NSString *)payload
                                chunkSize:(NSUInteger)chunkSize;
- (void)readFinished:(NSNotification *)notification;
@end

// helper class
@interface TestServerDelegate : NSObject {
  NSMutableArray *requests_;
  NSMutableArray *responses_;
}
+ (id)testServerDelegate;
- (NSUInteger)requestCount;
- (GTMHTTPRequestMessage *)popRequest;
- (void)pushResponse:(GTMHTTPResponseMessage *)message;
@end

// helper that throws while handling its request
@interface TestThrowingServerDelegate : TestServerDelegate
- (GTMHTTPResponseMessage *)httpServer:(GTMHTTPServer *)server
                         handleRequest:(GTMHTTPRequestMessage *)request;
@end

// The timings used for waiting for replies
const NSTimeInterval kGiveUpInterval = 5.0;
const NSTimeInterval kRunLoopInterval = 0.01;

// the size we break writes up into to test the reading code and how long to
// wait between writes.
const NSUInteger kSendChunkSize = 12;
const NSTimeInterval kSendChunkInterval = 0.05;

// ----------------------------------------------------------------------------

@implementation GTMHTTPServerTest

- (void)testInit {
  // bad delegates
  [GTMUnitTestDevLog expectString:@"missing delegate"];
  STAssertNil([[GTMHTTPServer alloc] init], nil);
  [GTMUnitTestDevLog expectString:@"missing delegate"];
  STAssertNil([[GTMHTTPServer alloc] initWithDelegate:nil], nil);

  TestServerDelegate *delegate = [TestServerDelegate testServerDelegate];
  STAssertNotNil(delegate, nil);
  GTMHTTPServer *server =
    [[[GTMHTTPServer alloc] initWithDelegate:delegate] autorelease];
  STAssertNotNil(server, nil);

  // some attributes

  STAssertTrue([server delegate] == delegate, nil);

  [server setLocalhostOnly:NO];
  STAssertFalse([server localhostOnly], nil);
  [server setLocalhostOnly:YES];
  STAssertTrue([server localhostOnly], nil);
  
  STAssertEquals([server port], (uint16_t)0, nil);
  [server setPort:8080];
  STAssertEquals([server port], (uint16_t)8080, nil);
  [server setPort:80];
  STAssertEquals([server port], (uint16_t)80, nil);
  
  // description (atleast 10 chars)
  STAssertGreaterThan([[server description] length], (NSUInteger)10, nil);
}

- (void)testStartStop {
  TestServerDelegate *delegate1 = [TestServerDelegate testServerDelegate];
  STAssertNotNil(delegate1, nil);
  GTMHTTPServer *server1 =
    [[[GTMHTTPServer alloc] initWithDelegate:delegate1] autorelease];
  STAssertNotNil(server1, nil);
  NSError *error = nil;
  STAssertTrue([server1 start:&error], @"failed to start (error=%@)", error);
  STAssertNil(error, @"error: %@", error);
  STAssertGreaterThanOrEqual([server1 port], (uint16_t)1024,
                             @"how'd we get a reserved port?");

  TestServerDelegate *delegate2 = [TestServerDelegate testServerDelegate];
  STAssertNotNil(delegate2, nil);
  GTMHTTPServer *server2 =
    [[[GTMHTTPServer alloc] initWithDelegate:delegate2] autorelease];
  STAssertNotNil(server2, nil);
  
  // try the reserved port
  [server2 setPort:666];
  error = nil;
  STAssertFalse([server2 start:&error], nil);
  STAssertNotNil(error, nil);
  STAssertEqualObjects([error domain], kGTMHTTPServerErrorDomain, nil);
  STAssertEquals([error code], (NSInteger)kGTMHTTPServerBindFailedError,
                 @"port should have been reserved");
  
  // try the same port
  [server2 setPort:[server1 port]];
  error = nil;
  STAssertFalse([server2 start:&error], nil);
  STAssertNotNil(error, nil);
  STAssertEqualObjects([error domain], kGTMHTTPServerErrorDomain, nil);
  STAssertEquals([error code], (NSInteger)kGTMHTTPServerBindFailedError,
                 @"port should have been in use");
  
  // try a random port again so we really start (prove two can run at once)
  [server2 setPort:0];
  error = nil;
  STAssertTrue([server2 start:&error], @"failed to start (error=%@)", error);
  STAssertNil(error, @"error: %@", error);
  
  // shut them down
  [server1 stop];
  [server2 stop];
}

- (void)testRequests {
  TestServerDelegate *delegate = [TestServerDelegate testServerDelegate];
  STAssertNotNil(delegate, nil);
  GTMHTTPServer *server =
    [[[GTMHTTPServer alloc] initWithDelegate:delegate] autorelease];
  STAssertNotNil(server, nil);
  NSError *error = nil;
  STAssertTrue([server start:&error], @"failed to start (error=%@)", error);
  STAssertNil(error, @"error: %@", error);

  // a request to test all the fields of a request object
  
  NSString *payload =
    @"PUT /some/server/path HTTP/1.0\r\n"
    @"Content-Length: 16\r\n"
    @"Custom-Header: Custom_Value\r\n"
    @"\r\n"
    @"this is the body";
  NSData *reply =
    [self fetchFromPort:[server port] payload:payload chunkSize:kSendChunkSize];
  STAssertNotNil(reply, nil);
  
  GTMHTTPRequestMessage *request = [delegate popRequest];
  STAssertEqualObjects([request version], @"HTTP/1.0", nil);
  STAssertEqualObjects([[request URL] absoluteString], @"/some/server/path", nil);
  STAssertEqualObjects([request method], @"PUT", nil);
  STAssertEqualObjects([request body],
                       [@"this is the body" dataUsingEncoding:NSUTF8StringEncoding],
                       nil);
  NSDictionary *allHeaders = [request allHeaderFieldValues];
  STAssertNotNil(allHeaders, nil);
  STAssertEquals([allHeaders count], (NSUInteger)2, nil);
  STAssertEqualObjects([allHeaders objectForKey:@"Content-Length"],
                       @"16", nil);
  STAssertEqualObjects([allHeaders objectForKey:@"Custom-Header"],
                       @"Custom_Value", nil);
  STAssertGreaterThan([[request description] length], (NSUInteger)10, nil);
  
  // test different request types (in simple form)

  typedef struct  {
    NSString *method;
    NSString *url;
  } TestData;
  
  TestData data[] = {
    { @"GET", @"/foo/bar" },
    { @"HEAD", @"/foo/baz" },
    { @"POST", @"/foo" },
    { @"PUT", @"/foo/spam" },
    { @"DELETE", @"/fooby/doo" },
    { @"TRACE", @"/something.html" },
    { @"CONNECT", @"/spam" },
    { @"OPTIONS", @"/wee/doggies" },
  };

  for (size_t i = 0; i < sizeof(data) / sizeof(TestData); i++) {
    payload = [NSString stringWithFormat:@"%@ %@ HTTP/1.0\r\n\r\n",
               data[i].method, data[i].url];
    STAssertNotNil(payload, nil);
    reply = [self fetchFromPort:[server port] 
                        payload:payload 
                      chunkSize:kSendChunkSize];
    STAssertNotNil(reply, // just want a reply in this test
                   @"failed of method %@", data[i].method);
    request = [delegate popRequest];
    STAssertEqualObjects([[request URL] absoluteString], data[i].url,
                         @"urls didn't match for index %d", i);
    STAssertEqualObjects([request method], data[i].method,
                         @"methods didn't match for index %d", i);
  }
  
  [server stop];
}

- (void)testResponses {

  // some quick init tests for invalid things
  STAssertNil([[GTMHTTPResponseMessage alloc] init], nil);
  STAssertNil([GTMHTTPResponseMessage responseWithBody:nil
                                           contentType:nil
                                            statusCode:99],
              nil);
  STAssertNil([GTMHTTPResponseMessage responseWithBody:nil
                                           contentType:nil
                                            statusCode:602],
              nil);
  
  TestServerDelegate *delegate = [TestServerDelegate testServerDelegate];
  STAssertNotNil(delegate, nil);
  GTMHTTPServer *server =
    [[[GTMHTTPServer alloc] initWithDelegate:delegate] autorelease];
  STAssertNotNil(server, nil);
  NSError *error = nil;
  STAssertTrue([server start:&error], @"failed to start (error=%@)", error);
  STAssertNil(error, @"error: %@", error);

  // test the html helper
  
  GTMHTTPResponseMessage *expectedResponse =
    [GTMHTTPResponseMessage responseWithHTMLString:@"Success!"];
  STAssertNotNil(expectedResponse, nil);
  STAssertGreaterThan([[expectedResponse description] length],
                      (NSUInteger)0, nil);
  [delegate pushResponse:expectedResponse];
  NSData *responseData = [self fetchFromPort:[server port]
                                     payload:@"GET /foo HTTP/1.0\r\n\r\n"
                                   chunkSize:kSendChunkSize];
  STAssertNotNil(responseData, nil);
  NSString *responseString =
    [[[NSString alloc] initWithData:responseData
                           encoding:NSUTF8StringEncoding] autorelease];
  STAssertNotNil(responseString, nil);
  STAssertTrue([responseString hasPrefix:@"HTTP/1.0 200 OK"], nil);
  STAssertTrue([responseString hasSuffix:@"Success!"], @"should end w/ our data");
  STAssertNotEquals([responseString rangeOfString:@"Content-Length: 8"].location,
                    (NSUInteger)NSNotFound, nil);
  STAssertNotEquals([responseString rangeOfString:@"Content-Type: text/html; charset=UTF-8"].location,
                    (NSUInteger)NSNotFound, nil);
  
  // test the plain code response
  
  expectedResponse = [GTMHTTPResponseMessage emptyResponseWithCode:299];
  STAssertNotNil(expectedResponse, nil);
  STAssertGreaterThan([[expectedResponse description] length],
                      (NSUInteger)0, nil);
  [delegate pushResponse:expectedResponse];
  responseData = [self fetchFromPort:[server port]
                             payload:@"GET /foo HTTP/1.0\r\n\r\n"
                           chunkSize:kSendChunkSize];
  STAssertNotNil(responseData, nil);
  responseString =
    [[[NSString alloc] initWithData:responseData
                           encoding:NSUTF8StringEncoding] autorelease];
  STAssertNotNil(responseString, nil);
  STAssertTrue([responseString hasPrefix:@"HTTP/1.0 299 "], nil);
  STAssertNotEquals([responseString rangeOfString:@"Content-Length: 0"].location,
                    (NSUInteger)NSNotFound, nil);
  STAssertNotEquals([responseString rangeOfString:@"Content-Type: text/html"].location,
                    (NSUInteger)NSNotFound, nil);

  // test the general api w/ extra header add

  expectedResponse =
    [GTMHTTPResponseMessage responseWithBody:[@"FOO" dataUsingEncoding:NSUTF8StringEncoding]
                                 contentType:@"some/type"
                                  statusCode:298];
  STAssertNotNil(expectedResponse, nil);
  STAssertGreaterThan([[expectedResponse description] length],
                      (NSUInteger)0, nil);
  [expectedResponse setValue:@"Custom_Value"
              forHeaderField:@"Custom-Header"];
  [expectedResponse setValue:nil
              forHeaderField:@"Custom-Header2"];
  [delegate pushResponse:expectedResponse];
  responseData = [self fetchFromPort:[server port]
                             payload:@"GET /foo HTTP/1.0\r\n\r\n"
                           chunkSize:kSendChunkSize];
  STAssertNotNil(responseData, nil);
  responseString =
    [[[NSString alloc] initWithData:responseData
                           encoding:NSUTF8StringEncoding] autorelease];
  STAssertNotNil(responseString, nil);
  STAssertTrue([responseString hasPrefix:@"HTTP/1.0 298"], nil);
  STAssertTrue([responseString hasSuffix:@"FOO"], @"should end w/ our data");
  STAssertNotEquals([responseString rangeOfString:@"Content-Length: 3"].location,
                    (NSUInteger)NSNotFound, nil);
  STAssertNotEquals([responseString rangeOfString:@"Content-Type: some/type"].location,
                    (NSUInteger)NSNotFound, nil);
  STAssertNotEquals([responseString rangeOfString:@"Custom-Header: Custom_Value"].location,
                    (NSUInteger)NSNotFound, nil);
  STAssertNotEquals([responseString rangeOfString:@"Custom-Header2: "].location,
                    (NSUInteger)NSNotFound, nil);
  
  [server stop];
}

- (void)testRequstEdgeCases {
  // test all the odd things about requests

  TestServerDelegate *delegate = [TestServerDelegate testServerDelegate];
  STAssertNotNil(delegate, nil);
  GTMHTTPServer *server =
    [[[GTMHTTPServer alloc] initWithDelegate:delegate] autorelease];
  STAssertNotNil(server, nil);
  NSError *error = nil;
  STAssertTrue([server start:&error], @"failed to start (error=%@)", error);
  STAssertNil(error, @"error: %@", error);
  
  // extra data (ie-pipelining)

  NSString *payload =
    @"GET /some/server/path HTTP/1.0\r\n"
    @"\r\n"
    @"GET /some/server/path/too HTTP/1.0\r\n"
    @"\r\n";
  // don't chunk this, we want to make sure both requests get to our server
  [GTMUnitTestDevLog expectString:@"Got 38 extra bytes on http request, "
                                    "ignoring them"];
  NSData *reply =
    [self fetchFromPort:[server port] payload:payload chunkSize:0];
  STAssertNotNil(reply, nil);
  STAssertEquals([delegate requestCount], (NSUInteger)1, nil);
  
  // close w/o full request
  {
    // local pool so we can force our handle to close
    NSAutoreleasePool *localPool = [[NSAutoreleasePool alloc] init];
    NSFileHandle *handle =
      [self fileHandleSendingToPort:[server port]
                            payload:@"GET /some/server/path HTTP/"
                          chunkSize:kSendChunkSize];
    STAssertNotNil(handle, nil);
    // spin the run loop so reads the start of the request
    NSDate* loopIntervalDate =
      [NSDate dateWithTimeIntervalSinceNow:kRunLoopInterval];
    [[NSRunLoop currentRunLoop] runUntilDate:loopIntervalDate]; 
    // make sure we see the request at this point
    STAssertEquals([server activeRequestCount], (NSUInteger)1,
                   @"should have started the request by now");
    // force the connection closed and drop the pool to get all the cleanup to
    // happen.
    [handle closeFile];
    [localPool drain];
    // spin the run loop so it should see the close
    loopIntervalDate = [NSDate dateWithTimeIntervalSinceNow:kRunLoopInterval];
    [[NSRunLoop currentRunLoop] runUntilDate:loopIntervalDate];
    // make sure we didn't get a request (1 is from test before) and make sure
    // we don't have some in flight.
    STAssertEquals([delegate requestCount], (NSUInteger)1,
                   @"shouldn't have gotten another request");
    STAssertEquals([server activeRequestCount], (NSUInteger)0,
                   @"should have cleaned up the pending connection");
  }
  
}

- (void)testExceptionDuringRequest {

  TestServerDelegate *delegate = [TestThrowingServerDelegate testServerDelegate];
  STAssertNotNil(delegate, nil);
  GTMHTTPServer *server =
    [[[GTMHTTPServer alloc] initWithDelegate:delegate] autorelease];
  STAssertNotNil(server, nil);
  NSError *error = nil;
  STAssertTrue([server start:&error], @"failed to start (error=%@)", error);
  STAssertNil(error, @"error: %@", error);
  [GTMUnitTestDevLog expectString:@"Exception trying to handle http request: "
   "To test our handling"];
  NSData *responseData = [self fetchFromPort:[server port]
                                     payload:@"GET /foo HTTP/1.0\r\n\r\n"
                                   chunkSize:kSendChunkSize];
  STAssertNotNil(responseData, nil);
  STAssertEquals([responseData length], (NSUInteger)0, nil);
  STAssertEquals([delegate requestCount], (NSUInteger)1, nil);
  STAssertEquals([server activeRequestCount], (NSUInteger)0, nil);
}

@end

// ----------------------------------------------------------------------------

@implementation GTMHTTPServerTest (PrivateMethods)

- (NSData *)fetchFromPort:(unsigned short)port
                  payload:(NSString *)payload
                chunkSize:(NSUInteger)chunkSize {
  fetchedData_ = nil;

  NSFileHandle *handle = [self fileHandleSendingToPort:port
                                               payload:payload
                                             chunkSize:chunkSize];

  NSNotificationCenter *center = [NSNotificationCenter defaultCenter];
  [center addObserver:self
             selector:@selector(readFinished:)
                 name:NSFileHandleReadToEndOfFileCompletionNotification
               object:handle];
  [handle readToEndOfFileInBackgroundAndNotify];

  // wait for our reply
  NSDate* giveUpDate = [NSDate dateWithTimeIntervalSinceNow:kGiveUpInterval];
  while (!fetchedData_ && [giveUpDate timeIntervalSinceNow] > 0) {
    NSDate* loopIntervalDate =
      [NSDate dateWithTimeIntervalSinceNow:kRunLoopInterval];
    [[NSRunLoop currentRunLoop] runUntilDate:loopIntervalDate]; 
  }

  [center removeObserver:self
                 name:NSFileHandleReadToEndOfFileCompletionNotification
               object:handle];
  
  NSData *result = [fetchedData_ autorelease];
  fetchedData_ = nil;
  return result;
}

- (NSFileHandle *)fileHandleSendingToPort:(unsigned short)port
                                  payload:(NSString *)payload
                                chunkSize:(NSUInteger)chunkSize {
  int fd = socket(AF_INET, SOCK_STREAM, 0);
  STAssertGreaterThan(fd, 0, @"failed to create socket");
  
  struct sockaddr_in addr;
  bzero(&addr, sizeof(addr));
  addr.sin_len    = sizeof(addr);
  addr.sin_family = AF_INET;
  addr.sin_port   = htons(port);
  addr.sin_addr.s_addr = htonl(0x7F000001);
  int connectResult =
  connect(fd, (struct sockaddr*)(&addr), (socklen_t)sizeof(addr));
  STAssertEquals(connectResult, 0, nil);
  
  NSFileHandle *handle =
  [[[NSFileHandle alloc] initWithFileDescriptor:fd
                                 closeOnDealloc:YES] autorelease];
  STAssertNotNil(handle, nil);
  
  NSData *payloadData = [payload dataUsingEncoding:NSUTF8StringEncoding];
  
  // we can send in one block or in chunked mode
  if (chunkSize > 0) {
    // we don't write the data in one large block, instead of write it out
    // in bits to help test the data collection code.
    NSUInteger length = [payloadData length];
    for (NSUInteger x = 0 ; x < length ; x += chunkSize) {
      NSUInteger dataChunkSize = length - x;
      if (dataChunkSize > chunkSize) {
        dataChunkSize = chunkSize;
      }
      NSData *dataChunk 
        = [payloadData subdataWithRange:NSMakeRange(x, dataChunkSize)];
      [handle writeData:dataChunk];
      // delay after all but the last chunk to give it time to be read.
      if ((x + chunkSize) < length) {
        NSDate* loopIntervalDate =
          [NSDate dateWithTimeIntervalSinceNow:kSendChunkInterval];
        [[NSRunLoop currentRunLoop] runUntilDate:loopIntervalDate]; 
      }
    }
  } else {
    [handle writeData:payloadData];
  }
  
  return handle;
}

- (void)readFinished:(NSNotification *)notification {
  NSDictionary *userInfo = [notification userInfo];
  fetchedData_ =
    [[userInfo objectForKey:NSFileHandleNotificationDataItem] retain];
}

@end

// ----------------------------------------------------------------------------

@implementation TestServerDelegate

- (id)init {
  self = [super init];
  if (self) {
    requests_ = [[NSMutableArray alloc] init];
    responses_ = [[NSMutableArray alloc] init];
  }
  return self;
}

- (void)dealloc {
  [requests_ release];
  [responses_ release];
  [super dealloc];
}

+ (id)testServerDelegate {
  return [[[[self class] alloc] init] autorelease];
}

- (NSUInteger)requestCount {
  return [requests_ count];
}

- (GTMHTTPRequestMessage *)popRequest {
  GTMHTTPRequestMessage *result = [[[requests_ lastObject] retain] autorelease];
  [requests_ removeLastObject];
  return result;
}

- (void)pushResponse:(GTMHTTPResponseMessage *)message {
  [responses_ addObject:message];
}

- (GTMHTTPResponseMessage *)httpServer:(GTMHTTPServer *)server
                         handleRequest:(GTMHTTPRequestMessage *)request {
  [requests_ addObject:request];
  
  GTMHTTPResponseMessage *result = nil;
  if ([responses_ count] > 0) {
    result = [[[responses_ lastObject] retain] autorelease];
    [responses_ removeLastObject];
  } else {
    result = [GTMHTTPResponseMessage responseWithHTMLString:@"success"];
  }
  return result;
}

@end

// ----------------------------------------------------------------------------

@implementation TestThrowingServerDelegate

- (GTMHTTPResponseMessage *)httpServer:(GTMHTTPServer *)server
                         handleRequest:(GTMHTTPRequestMessage *)request {
  // let the base do its normal work for counts, etc.
  [super httpServer:server handleRequest:request];
  NSException *exception =
    [NSException exceptionWithName:@"InternalTestingException"
                            reason:@"To test our handling"
                          userInfo:nil];
  @throw exception;
}

@end
