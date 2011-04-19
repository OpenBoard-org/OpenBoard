//
//  GTMBase64Test.m
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
#import "GTMBase64.h"
#include <stdlib.h> // for randiom/srandomdev

static void FillWithRandom(char *data, NSUInteger len) {
  char *max = data + len;
  for ( ; data < max ; ++data) {
    *data = random() & 0xFF;
  }
}

static BOOL NoEqualChar(NSData *data) {
  const char *scan = [data bytes];
  const char *max = scan + [data length];
  for ( ; scan < max ; ++scan) {
    if (*scan == '=') {
      return NO;
    }
  }
  return YES;
}

@interface GTMBase64Test : GTMTestCase 
@end

@implementation GTMBase64Test

- (void)setUp {
  // seed random from /dev/random
  srandomdev();
}

- (void)testBase64 {
  // generate a range of sizes w/ random content
  for (int x = 1 ; x < 1024 ; ++x) {
    NSMutableData *data = [NSMutableData data];
    STAssertNotNil(data, @"failed to alloc data block");
    
    [data setLength:x];
    FillWithRandom([data mutableBytes], [data length]);
    
    // w/ *Bytes apis
    NSData *encoded = [GTMBase64 encodeBytes:[data bytes] length:[data length]];
    STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                   @"encoded size via *Bytes apis should be a multiple of 4");
    NSData *dataPrime = [GTMBase64 decodeBytes:[encoded bytes]
                                        length:[encoded length]];
    STAssertEqualObjects(data, dataPrime,
                         @"failed to round trip via *Bytes apis");

    // w/ *Data apis
    encoded = [GTMBase64 encodeData:data];
    STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                   @"encoded size via *Data apis should be a multiple of 4");
    dataPrime = [GTMBase64 decodeData:encoded];
    STAssertEqualObjects(data, dataPrime,
                         @"failed to round trip via *Data apis");
    
    // Bytes to String and back
    NSString *encodedString = [GTMBase64 stringByEncodingBytes:[data bytes]
                                                        length:[data length]];
    STAssertEquals(([encodedString length] % 4), (NSUInteger)0,
                   @"encoded size for Bytes to Strings should be a multiple of 4");
    dataPrime = [GTMBase64 decodeString:encodedString];
    STAssertEqualObjects(data, dataPrime,
                         @"failed to round trip for Bytes to Strings");

    // Data to String and back
    encodedString = [GTMBase64 stringByEncodingData:data];
    STAssertEquals(([encodedString length] % 4), (NSUInteger)0,
                   @"encoded size for Data to Strings should be a multiple of 4");
    dataPrime = [GTMBase64 decodeString:encodedString];
    STAssertEqualObjects(data, dataPrime,
                         @"failed to round trip for Bytes to Strings");
  }
  
  {
    // now test all byte values
    NSMutableData *data = [NSMutableData data];
    STAssertNotNil(data, @"failed to alloc data block");
    
    [data setLength:256];
    unsigned char *scan = (unsigned char*)[data mutableBytes];
    for (int x = 0 ; x <= 255 ; ++x) {
      *scan++ = x;
    }
    
    // w/ *Bytes apis
    NSData *encoded = [GTMBase64 encodeBytes:[data bytes] length:[data length]];
    STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                   @"encoded size via *Bytes apis should be a multiple of 4");
    NSData *dataPrime = [GTMBase64 decodeBytes:[encoded bytes]
                                        length:[encoded length]];
    STAssertEqualObjects(data, dataPrime,
                         @"failed to round trip via *Bytes apis");

    // w/ *Data apis
    encoded = [GTMBase64 encodeData:data];
    STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                   @"encoded size via *Data apis should be a multiple of 4");
    dataPrime = [GTMBase64 decodeData:encoded];
    STAssertEqualObjects(data, dataPrime,
                         @"failed to round trip via *Data apis");

    // Bytes to String and back
    NSString *encodedString = [GTMBase64 stringByEncodingBytes:[data bytes]
                                                        length:[data length]];
    STAssertEquals(([encodedString length] % 4), (NSUInteger)0,
                   @"encoded size for Bytes to Strings should be a multiple of 4");
    dataPrime = [GTMBase64 decodeString:encodedString];
    STAssertEqualObjects(data, dataPrime,
                         @"failed to round trip for Bytes to Strings");

    // Data to String and back
    encodedString = [GTMBase64 stringByEncodingData:data];
    STAssertEquals(([encodedString length] % 4), (NSUInteger)0,
                   @"encoded size for Data to Strings should be a multiple of 4");
    dataPrime = [GTMBase64 decodeString:encodedString];
    STAssertEqualObjects(data, dataPrime,
                         @"failed to round trip for Data to Strings");
  }
  
  {
    // test w/ a mix of spacing characters
    
    // generate some data, encode it, and add spaces
    NSMutableData *data = [NSMutableData data];
    STAssertNotNil(data, @"failed to alloc data block");
    
    [data setLength:253]; // should get some padding chars on the end
    FillWithRandom([data mutableBytes], [data length]);

    NSString *encodedString = [GTMBase64 stringByEncodingData:data];
    NSMutableString *encodedAndSpaced =
      [[encodedString mutableCopy] autorelease];

    NSString *spaces[] = { @"\t", @"\n", @"\r", @" " };
    const NSUInteger numSpaces = sizeof(spaces) / sizeof(NSString*);
    for (int x = 0 ; x < 512 ; ++x) {
      NSUInteger offset = random() % ([encodedAndSpaced length] + 1);
      [encodedAndSpaced insertString:spaces[random() % numSpaces]
                             atIndex:offset];
    }

    // we'll need it as data for apis
    NSData *encodedAsData =
      [encodedAndSpaced dataUsingEncoding:NSASCIIStringEncoding];
    STAssertNotNil(encodedAsData, @"failed to extract from string");
    STAssertEquals([encodedAsData length], [encodedAndSpaced length],
                   @"lengths for encoded string and data didn't match?");
    
    // all the decode modes
    NSData *dataPrime = [GTMBase64 decodeData:encodedAsData];
    STAssertEqualObjects(data, dataPrime,
                         @"failed Data decode w/ spaces");
    dataPrime = [GTMBase64 decodeBytes:[encodedAsData bytes]
                                length:[encodedAsData length]];
    STAssertEqualObjects(data, dataPrime,
                         @"failed Bytes decode w/ spaces");
    dataPrime = [GTMBase64 decodeString:encodedAndSpaced];
    STAssertEqualObjects(data, dataPrime,
                         @"failed String decode w/ spaces");
  }
}

- (void)testWebSafeBase64 {
  // loop to test w/ and w/o padding
  for (int paddedLoop = 0; paddedLoop < 2 ; ++paddedLoop) {
    BOOL padded = (paddedLoop == 1);
  
    // generate a range of sizes w/ random content
    for (int x = 1 ; x < 1024 ; ++x) {
      NSMutableData *data = [NSMutableData data];
      STAssertNotNil(data, @"failed to alloc data block");

      [data setLength:x];
      FillWithRandom([data mutableBytes], [data length]);

      // w/ *Bytes apis
      NSData *encoded = [GTMBase64 webSafeEncodeBytes:[data bytes]
                                               length:[data length]
                                               padded:padded];
      if (padded) {
        STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                       @"encoded size via *Bytes apis should be a multiple of 4");
      } else {
        STAssertTrue(NoEqualChar(encoded),
                     @"encoded via *Bytes apis had a base64 padding char");
      }
      NSData *dataPrime = [GTMBase64 webSafeDecodeBytes:[encoded bytes]
                                                 length:[encoded length]];
      STAssertEqualObjects(data, dataPrime,
                           @"failed to round trip via *Bytes apis");
      
      // w/ *Data apis
      encoded = [GTMBase64 webSafeEncodeData:data padded:padded];
      if (padded) {
        STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                       @"encoded size via *Data apis should be a multiple of 4");
      } else {
        STAssertTrue(NoEqualChar(encoded),
                     @"encoded via *Data apis had a base64 padding char");
      }
      dataPrime = [GTMBase64 webSafeDecodeData:encoded];
      STAssertEqualObjects(data, dataPrime,
                           @"failed to round trip via *Data apis");
      
      // Bytes to String and back
      NSString *encodedString =
        [GTMBase64 stringByWebSafeEncodingBytes:[data bytes]
                                         length:[data length]
                                         padded:padded];
      if (padded) {
        STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                       @"encoded size via *Bytes apis should be a multiple of 4");
      } else {
        STAssertTrue(NoEqualChar(encoded),
                     @"encoded via Bytes to Strings had a base64 padding char");
      }
      dataPrime = [GTMBase64 webSafeDecodeString:encodedString];
      STAssertEqualObjects(data, dataPrime,
                           @"failed to round trip for Bytes to Strings");
      
      // Data to String and back
      encodedString =
        [GTMBase64 stringByWebSafeEncodingData:data padded:padded];
      if (padded) {
        STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                       @"encoded size via *Data apis should be a multiple of 4");
      } else {
        STAssertTrue(NoEqualChar(encoded),
                     @"encoded via Data to Strings had a base64 padding char");
      }
      dataPrime = [GTMBase64 webSafeDecodeString:encodedString];
      STAssertEqualObjects(data, dataPrime,
                           @"failed to round trip for Data to Strings");
    }

    {
      // now test all byte values
      NSMutableData *data = [NSMutableData data];
      STAssertNotNil(data, @"failed to alloc data block");
      
      [data setLength:256];
      unsigned char *scan = (unsigned char*)[data mutableBytes];
      for (int x = 0 ; x <= 255 ; ++x) {
        *scan++ = x;
      }
      
      // w/ *Bytes apis
      NSData *encoded =
        [GTMBase64 webSafeEncodeBytes:[data bytes]
                               length:[data length]
                               padded:padded];
      if (padded) {
        STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                       @"encoded size via *Bytes apis should be a multiple of 4");
      } else {
        STAssertTrue(NoEqualChar(encoded),
                   @"encoded via *Bytes apis had a base64 padding char");
      }
      NSData *dataPrime = [GTMBase64 webSafeDecodeBytes:[encoded bytes]
                                                 length:[encoded length]];
      STAssertEqualObjects(data, dataPrime,
                           @"failed to round trip via *Bytes apis");
      
      // w/ *Data apis
      encoded = [GTMBase64 webSafeEncodeData:data padded:padded];
      if (padded) {
        STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                       @"encoded size via *Data apis should be a multiple of 4");
      } else {
        STAssertTrue(NoEqualChar(encoded),
                   @"encoded via *Data apis had a base64 padding char");
      }
      dataPrime = [GTMBase64 webSafeDecodeData:encoded];
      STAssertEqualObjects(data, dataPrime,
                           @"failed to round trip via *Data apis");
      
      // Bytes to String and back
      NSString *encodedString =
        [GTMBase64 stringByWebSafeEncodingBytes:[data bytes]
                                         length:[data length]
                                         padded:padded];
      if (padded) {
        STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                       @"encoded size via *Bytes apis should be a multiple of 4");
      } else {
        STAssertTrue(NoEqualChar(encoded),
                   @"encoded via Bytes to Strings had a base64 padding char");
      }
      dataPrime = [GTMBase64 webSafeDecodeString:encodedString];
      STAssertEqualObjects(data, dataPrime,
                           @"failed to round trip for Bytes to Strings");
      
      // Data to String and back
      encodedString =
        [GTMBase64 stringByWebSafeEncodingData:data padded:padded];
      if (padded) {
        STAssertEquals(([encoded length] % 4), (NSUInteger)0,
                       @"encoded size via *Data apis should be a multiple of 4");
      } else {
        STAssertTrue(NoEqualChar(encoded),
                   @"encoded via Data to Strings had a base64 padding char");
      }
      dataPrime = [GTMBase64 webSafeDecodeString:encodedString];
      STAssertEqualObjects(data, dataPrime,
                           @"failed to round trip for Data to Strings");
    }

    {
      // test w/ a mix of spacing characters
      
      // generate some data, encode it, and add spaces
      NSMutableData *data = [NSMutableData data];
      STAssertNotNil(data, @"failed to alloc data block");
      
      [data setLength:253]; // should get some padding chars on the end
      FillWithRandom([data mutableBytes], [data length]);
      
      NSString *encodedString = [GTMBase64 stringByWebSafeEncodingData:data
                                                                padded:padded];
      NSMutableString *encodedAndSpaced =
        [[encodedString mutableCopy] autorelease];
      
      NSString *spaces[] = { @"\t", @"\n", @"\r", @" " };
      const NSUInteger numSpaces = sizeof(spaces) / sizeof(NSString*);
      for (int x = 0 ; x < 512 ; ++x) {
        NSUInteger offset = random() % ([encodedAndSpaced length] + 1);
        [encodedAndSpaced insertString:spaces[random() % numSpaces]
                               atIndex:offset];
      }
      
      // we'll need it as data for apis
      NSData *encodedAsData =
        [encodedAndSpaced dataUsingEncoding:NSASCIIStringEncoding];
      STAssertNotNil(encodedAsData, @"failed to extract from string");
      STAssertEquals([encodedAsData length], [encodedAndSpaced length],
                     @"lengths for encoded string and data didn't match?");
      
      // all the decode modes
      NSData *dataPrime = [GTMBase64 webSafeDecodeData:encodedAsData];
      STAssertEqualObjects(data, dataPrime,
                           @"failed Data decode w/ spaces");
      dataPrime = [GTMBase64 webSafeDecodeBytes:[encodedAsData bytes]
                                         length:[encodedAsData length]];
      STAssertEqualObjects(data, dataPrime,
                           @"failed Bytes decode w/ spaces");
      dataPrime = [GTMBase64 webSafeDecodeString:encodedAndSpaced];
      STAssertEqualObjects(data, dataPrime,
                           @"failed String decode w/ spaces");
    }
  } // paddedLoop
}

- (void)testErrors {
  const int something = 0;
  NSString *nonAscString = [NSString stringWithUTF8String:"This test ©™®๒०᠐٧"];

  STAssertNil([GTMBase64 encodeData:nil], @"it worked?");
  STAssertNil([GTMBase64 decodeData:nil], @"it worked?");
  STAssertNil([GTMBase64 encodeBytes:NULL length:10], @"it worked?");
  STAssertNil([GTMBase64 encodeBytes:&something length:0], @"it worked?");
  STAssertNil([GTMBase64 decodeBytes:NULL length:10], @"it worked?");
  STAssertNil([GTMBase64 decodeBytes:&something length:0], @"it worked?");
  STAssertNil([GTMBase64 stringByEncodingData:nil], @"it worked?");
  STAssertNil([GTMBase64 stringByEncodingBytes:NULL length:10], @"it worked?");
  STAssertNil([GTMBase64 stringByEncodingBytes:&something length:0], @"it worked?");
  STAssertNil([GTMBase64 decodeString:nil], @"it worked?");
  // test some pads at the end that aren't right
  STAssertNil([GTMBase64 decodeString:@"=="], @"it worked?"); // just pads
  STAssertNil([GTMBase64 decodeString:@"vw="], @"it worked?"); // missing pad (in state 2)
  STAssertNil([GTMBase64 decodeString:@"vw"], @"it worked?"); // missing pad (in state 2)
  STAssertNil([GTMBase64 decodeString:@"NNw"], @"it worked?"); // missing pad (in state 3)
  STAssertNil([GTMBase64 decodeString:@"vw=v"], @"it worked?"); // missing pad, has something else
  STAssertNil([GTMBase64 decodeString:@"v="], @"it worked?"); // missing a needed char, has pad instead
  STAssertNil([GTMBase64 decodeString:@"v"], @"it worked?"); // missing a needed char
  STAssertNil([GTMBase64 decodeString:@"vw== vw"], @"it worked?");
  STAssertNil([GTMBase64 decodeString:nonAscString], @"it worked?");
  STAssertNil([GTMBase64 decodeString:@"@@@not valid###"], @"it worked?");
  // carefully crafted bad input to make sure we don't overwalk
  STAssertNil([GTMBase64 decodeString:@"WD=="], @"it worked?");
  
  STAssertNil([GTMBase64 webSafeEncodeData:nil padded:YES], @"it worked?");
  STAssertNil([GTMBase64 webSafeDecodeData:nil], @"it worked?");
  STAssertNil([GTMBase64 webSafeEncodeBytes:NULL length:10 padded:YES],
              @"it worked?");
  STAssertNil([GTMBase64 webSafeEncodeBytes:&something length:0 padded:YES],
              @"it worked?");
  STAssertNil([GTMBase64 webSafeDecodeBytes:NULL length:10], @"it worked?");
  STAssertNil([GTMBase64 webSafeDecodeBytes:&something length:0], @"it worked?");
  STAssertNil([GTMBase64 stringByWebSafeEncodingData:nil padded:YES],
              @"it worked?");
  STAssertNil([GTMBase64 stringByWebSafeEncodingBytes:NULL
                                               length:10
                                               padded:YES],
              @"it worked?");
  STAssertNil([GTMBase64 stringByWebSafeEncodingBytes:&something
                                               length:0
                                               padded:YES],
              @"it worked?");
  STAssertNil([GTMBase64 webSafeDecodeString:nil], @"it worked?");
  // test some pads at the end that aren't right
  STAssertNil([GTMBase64 webSafeDecodeString:@"=="], @"it worked?"); // just pad chars
  STAssertNil([GTMBase64 webSafeDecodeString:@"aw="], @"it worked?"); // missing pad
  STAssertNil([GTMBase64 webSafeDecodeString:@"aw=a"], @"it worked?"); // missing pad, has something else
  STAssertNil([GTMBase64 webSafeDecodeString:@"a"], @"it worked?"); // missing a needed char
  STAssertNil([GTMBase64 webSafeDecodeString:@"a="], @"it worked?"); // missing a needed char, has pad instead
  STAssertNil([GTMBase64 webSafeDecodeString:@"aw== a"], @"it worked?"); // missing pad
  STAssertNil([GTMBase64 webSafeDecodeString:nonAscString], @"it worked?");
  STAssertNil([GTMBase64 webSafeDecodeString:@"@@@not valid###"], @"it worked?");
  // carefully crafted bad input to make sure we don't overwalk
  STAssertNil([GTMBase64 webSafeDecodeString:@"WD=="], @"it worked?");

  // make sure our local helper is working right
  STAssertFalse(NoEqualChar([NSData dataWithBytes:"aa=zz" length:5]), @"");
}

@end
