//
//  GTMAddressBookTest.m
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
#import "GTMABAddressBook.h"

@interface GTMABAddressBookTest : GTMTestCase {
 @private
  GTMABAddressBook *book_;
}
@end


@implementation GTMABAddressBookTest
- (void)setUp {
  // Create a book forcing it out of it's autorelease pool.
  // I force it out of the release pool, so that we will see any errors
  // for it immediately at teardown, and it will be clear which release
  // caused us problems.
  NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
  book_ = [[GTMABAddressBook addressBook] retain];
  [pool release];
  STAssertNotNil(book_, nil);
  STAssertFalse([book_ hasUnsavedChanges], nil);
}

- (void)tearDown {
  [book_ release];
}

- (void)testGenericAddressBook {
  STAssertEqualObjects([GTMABAddressBook localizedLabel:kABHomeLabel], 
                       @"home", 
                       nil);
  STAssertThrows([GTMABRecord recordWithRecord:nil], nil);
}

- (void)testAddingAndRemovingPerson {
  // Create a person
  GTMABPerson *person = [GTMABPerson personWithFirstName:@"Bart" 
                                                lastName:@"Simpson"];
  STAssertNotNil(person, nil);
  
  // Add person
  NSArray *people = [book_ people];
  STAssertFalse([people containsObject:person], nil);
  STAssertTrue([book_ addRecord:person], nil);
  
  // Normally this next line would be STAssertTrue, however due to
  // Radar 6200638: ABAddressBookHasUnsavedChanges doesn't work
  // We will check to make sure it stays broken ;-)
  STAssertFalse([book_ hasUnsavedChanges], nil);
  
  people = [book_ people];
  STAssertNotNil(people, nil);
  // Normally this next line would be STAssertTrue, however due to
  // Radar 6200703: ABAddressBookAddRecord doesn't add an item to the people 
  //                array until it's saved
  // We will check to make sure it stays broken ;-)
  STAssertFalse([people containsObject:person], nil);
  
  // Save book_
  STAssertTrue([book_ save], nil);
  people = [book_ people];
  STAssertNotNil(people, nil);
  STAssertTrue([people containsObject:person], nil);
  
  ABRecordID recordID = [person recordID];
  STAssertNotEquals(recordID, kABRecordInvalidID, nil);
  
  GTMABRecord *record = [book_ personForId:recordID];
  STAssertEqualObjects(record, person, nil);
  
  // Remove person
  STAssertTrue([book_ removeRecord:person], nil);
  // Normally this next line would be STAssertTrue, however due to
  // Radar 6200638: ABAddressBookHasUnsavedChanges doesn't work
  // We will check to make sure it stays broken ;-)
  STAssertFalse([book_ hasUnsavedChanges], nil);
  
  // Normally this next line would be STAssertFalse, however due to
  // Radar 6200703: ABAddressBookAddRecord doesn't add an item to the people 
  //                array until it's saved
  // We will check to make sure it stays broken ;-)
  STAssertTrue([people containsObject:person], nil);
  
  // Save Book
  STAssertTrue([book_ save], nil);
  people = [book_ people];
  STAssertFalse([book_ hasUnsavedChanges], nil);
  STAssertFalse([people containsObject:person], nil); 
  record = [book_ personForId:recordID];
  STAssertNil(record, nil);
  
  // Revert book_
  STAssertTrue([book_ addRecord:person], nil);
  // Normally this next line would be STAssertTrue, however due to
  // Radar 6200638: ABAddressBookHasUnsavedChanges doesn't work
  // We will check to make sure it stays broken ;-)
  STAssertFalse([book_ hasUnsavedChanges], nil);
  
  [book_ revert];
  STAssertFalse([book_ hasUnsavedChanges], nil);
  
  // Bogus data
  STAssertFalse([book_ addRecord:nil], nil);
  STAssertFalse([book_ removeRecord:nil], nil);
  
  STAssertNotNULL([book_ addressBookRef], nil);
  
}

- (void)testAddingAndRemovingGroup {
  // Create a group
  GTMABGroup *group = [GTMABGroup groupNamed:@"Test"];
  STAssertNotNil(group, nil);
  
  // Add group
  NSArray *groups = [book_ groups];
  STAssertFalse([groups containsObject:group], nil);
  STAssertTrue([book_ addRecord:group], nil);
  
  // Normally this next line would be STAssertTrue, however due to
  // Radar 6200638: ABAddressBookHasUnsavedChanges doesn't work
  // We will check to make sure it stays broken ;-)
  STAssertFalse([book_ hasUnsavedChanges], nil);
  
  groups = [book_ groups];
  STAssertNotNil(groups, nil);
  // Normally this next line would be STAssertTrue, however due to
  // Radar 6200703: ABAddressBookAddRecord doesn't add an item to the groups 
  //                array until it's saved
  // We will check to make sure it stays broken ;-)
  STAssertFalse([groups containsObject:group], nil);
  
  // Save book_
  STAssertTrue([book_ save], nil);
  groups = [book_ groups];
  STAssertNotNil(groups, nil);
  STAssertTrue([groups containsObject:group], nil);
  
  ABRecordID recordID = [group recordID];
  STAssertNotEquals(recordID, kABRecordInvalidID, nil);
  
  GTMABRecord *record = [book_ groupForId:recordID];
  STAssertEqualObjects(record, group, nil);
  
  // Remove group
  STAssertTrue([book_ removeRecord:group], nil);
  // Normally this next line would be STAssertTrue, however due to
  // Radar 6200638: ABAddressBookHasUnsavedChanges doesn't work
  // We will check to make sure it stays broken ;-)
  STAssertFalse([book_ hasUnsavedChanges], nil);
  
  // Normally this next line would be STAssertFalse, however due to
  // Radar 6200703: ABAddressBookAddRecord doesn't add an item to the groups 
  //                array until it's saved
  // We will check to make sure it stays broken ;-)
  STAssertTrue([groups containsObject:group], nil);
  
  // Save Book
  STAssertTrue([book_ save], nil);
  groups = [book_ groups];
  STAssertFalse([book_ hasUnsavedChanges], nil);
  STAssertFalse([groups containsObject:group], nil); 
  record = [book_ groupForId:recordID];
  STAssertNil(record, nil);

  // Revert book_
  STAssertTrue([book_ addRecord:group], nil);
  // Normally this next line would be STAssertTrue, however due to
  // Radar 6200638: ABAddressBookHasUnsavedChanges doesn't work
  // We will check to make sure it stays broken ;-)
  STAssertFalse([book_ hasUnsavedChanges], nil);
  
  [book_ revert];
  STAssertFalse([book_ hasUnsavedChanges], nil);
}

- (void)testPerson {
  GTMABPerson *person = [[[GTMABPerson alloc] initWithRecord:nil] autorelease];
  STAssertNil(person, nil);
  person = [GTMABPerson personWithFirstName:@"Bart"
                                   lastName:nil];
  STAssertNotNil(person, nil);
  STAssertEqualObjects([person compositeName], @"Bart", nil);
  NSString *firstName = [person valueForProperty:kABPersonFirstNameProperty];
  STAssertEqualObjects(firstName, @"Bart", nil);
  NSString *lastName = [person valueForProperty:kABPersonLastNameProperty];
  STAssertNil(lastName, nil);
  STAssertTrue([person removeValueForProperty:kABPersonFirstNameProperty], nil);
  STAssertFalse([person removeValueForProperty:kABPersonFirstNameProperty], nil);
  STAssertFalse([person removeValueForProperty:kABPersonLastNameProperty], nil);
  STAssertFalse([person setValue:nil forProperty:kABPersonFirstNameProperty], nil);
  STAssertFalse([person setValue:[NSNumber numberWithInt:1]
                     forProperty:kABPersonFirstNameProperty], nil);
  STAssertFalse([person setValue:@"Bart" 
                     forProperty:kABPersonBirthdayProperty], nil);
  
  ABPropertyType property 
    = [GTMABPerson typeOfProperty:kABPersonLastNameProperty];
  STAssertEquals(property, (ABPropertyType)kABStringPropertyType, nil);
  
  NSString *string 
    = [GTMABPerson localizedPropertyName:kABPersonLastNameProperty];
  STAssertEqualObjects(string, @"Last", nil);
  
  string = [GTMABPerson localizedPropertyName:kABRecordInvalidID];
  STAssertEqualObjects(string, kGTMABUnknownPropertyName, nil);
  
  string = [person description];
  STAssertNotNil(string, nil);
  
  ABPersonCompositeNameFormat format = [GTMABPerson compositeNameFormat];
  STAssertTrue(format == kABPersonCompositeNameFormatFirstNameFirst ||
               format == kABPersonCompositeNameFormatLastNameFirst, nil);
  
  NSData *data = [person imageData];
  STAssertNil(data, nil);
  STAssertTrue([person setImageData:nil], nil);
  data = [person imageData];
  STAssertNil(data, nil);
  UIImage *image = [UIImage imageNamed:@"phone.png"];
  STAssertNotNil(image, nil);
  data = UIImagePNGRepresentation(image);
  STAssertTrue([person setImageData:data], nil);
  NSData *data2 = [person imageData];
  STAssertEqualObjects(data, data2, nil);
  STAssertTrue([person setImageData:nil], nil);
  data = [person imageData];
  STAssertNil(data, nil);  
  
  STAssertTrue([person setImage:image], nil);
  UIImage *image2 = [person image];
  STAssertNotNil(image2, nil);
  STAssertEqualObjects(UIImagePNGRepresentation(image),
                       UIImagePNGRepresentation(image2), nil);
  
  person = [GTMABPerson personWithFirstName:@"Bart"
                                   lastName:@"Simpson"];

  data = [NSData dataWithBytes:"a" length:1];
  STAssertFalse([person setImageData:data], nil);
  
  GTMABMutableMultiValue *value 
    = [GTMABMutableMultiValue valueWithPropertyType:kABStringPropertyType];
  STAssertNotNil(value, nil);
  STAssertNotEquals([value addValue:@"222-222-2222" 
                          withLabel:kABHomeLabel], 
                    kABMultiValueInvalidIdentifier, nil);
  STAssertNotEquals([value addValue:@"333-333-3333" 
                          withLabel:kABWorkLabel], 
                    kABMultiValueInvalidIdentifier, nil);
  STAssertTrue([person setValue:value forProperty:kABPersonPhoneProperty], nil);
  id value2 = [person valueForProperty:kABPersonPhoneProperty];
  STAssertNotNil(value2, nil);
  STAssertEqualObjects(value, value2, nil);
  STAssertEquals([value hash], [value2 hash], nil);
  STAssertNotEquals([person hash], (NSUInteger)0, nil);
}

- (void)testGroup {
  GTMABGroup *group = [[[GTMABGroup alloc] initWithRecord:nil] autorelease];
  STAssertNil(group, nil);
  group = [GTMABGroup groupNamed:@"TestGroup"];
  STAssertNotNil(group, nil);
  STAssertEqualObjects([group compositeName], @"TestGroup", nil);
  NSString *name = [group valueForProperty:kABGroupNameProperty];
  STAssertEqualObjects(name, @"TestGroup", nil);
  NSString *lastName = [group valueForProperty:kABPersonLastNameProperty];
  STAssertNil(lastName, nil);
  STAssertTrue([group removeValueForProperty:kABGroupNameProperty], nil);
  STAssertFalse([group removeValueForProperty:kABGroupNameProperty], nil);
  STAssertFalse([group removeValueForProperty:kABPersonLastNameProperty], nil);
  STAssertFalse([group setValue:nil forProperty:kABGroupNameProperty], nil);
  STAssertFalse([group setValue:[NSNumber numberWithInt:1]
                    forProperty:kABGroupNameProperty], nil);
  STAssertFalse([group setValue:@"Bart" 
                    forProperty:kABPersonBirthdayProperty], nil);  
  
  ABPropertyType property = [GTMABGroup typeOfProperty:kABGroupNameProperty];
  STAssertEquals(property, (ABPropertyType)kABStringPropertyType, nil);
  
  property = [GTMABGroup typeOfProperty:kABPersonLastNameProperty];
  STAssertEquals(property, (ABPropertyType)kABInvalidPropertyType, nil);
  
  NSString *string = [GTMABGroup localizedPropertyName:kABGroupNameProperty];
  STAssertEqualObjects(string, @"Name", nil);
  
  string = [GTMABGroup localizedPropertyName:kABPersonLastNameProperty];
  STAssertEqualObjects(string, kGTMABUnknownPropertyName, nil);
  
  string = [GTMABGroup localizedPropertyName:kABRecordInvalidID];
  STAssertEqualObjects(string, kGTMABUnknownPropertyName, nil);

  string = [group description];
  STAssertNotNil(string, nil);
  
  // Adding and removing members
  group = [GTMABGroup groupNamed:@"TestGroup2"];
  NSArray *members = [group members];
  STAssertEquals([members count], (NSUInteger)0, @"Members: %@", members);
  
  STAssertFalse([group addMember:nil], nil);
  
  members = [group members];
  STAssertEquals([members count], (NSUInteger)0, @"Members: %@", members);
  
  GTMABPerson *person = [GTMABPerson personWithFirstName:@"Bart"
                                                lastName:@"Simpson"];
  STAssertNotNil(person, nil);
  STAssertTrue([book_ addRecord:person], nil);
  STAssertTrue([book_ save], nil);
  STAssertTrue([group addMember:person], nil);
  STAssertTrue([book_ addRecord:group], nil);
  STAssertTrue([book_ save], nil);
  members = [group members];
  STAssertEquals([members count], (NSUInteger)1, @"Members: %@", members);
  STAssertTrue([group removeMember:person], nil);
  STAssertFalse([group removeMember:person], nil);
  STAssertFalse([group removeMember:nil], nil);
  STAssertTrue([book_ removeRecord:group], nil);
  STAssertTrue([book_ removeRecord:person], nil);
  STAssertTrue([book_ save], nil);
}


- (void)testMultiValues {
  STAssertThrows([[GTMABMultiValue alloc] init], nil);
  STAssertThrows([[GTMABMutableMultiValue alloc] init], nil);
  GTMABMultiValue *value = [[GTMABMultiValue alloc] initWithMultiValue:nil];
  STAssertNil(value, nil);
  GTMABMutableMultiValue *mutValue 
    = [GTMABMutableMultiValue valueWithPropertyType:kABInvalidPropertyType];
  STAssertNil(mutValue, nil);
  mutValue 
    = [[[GTMABMutableMultiValue alloc]
        initWithMutableMultiValue:nil] autorelease];
  STAssertNil(mutValue, nil);
  mutValue 
    = [[[GTMABMutableMultiValue alloc]
        initWithMultiValue:nil] autorelease];
  STAssertNil(mutValue, nil);
  const ABPropertyType types[] = {
    kABStringPropertyType,
    kABIntegerPropertyType,
    kABRealPropertyType,
    kABDateTimePropertyType,
    kABDictionaryPropertyType,
    kABMultiStringPropertyType,
    kABMultiIntegerPropertyType,
    kABMultiRealPropertyType,
    kABMultiDateTimePropertyType,
    kABMultiDictionaryPropertyType
  };
  for (size_t i = 0; i < sizeof(types) / sizeof(ABPropertyType); ++i) {
    mutValue = [GTMABMutableMultiValue valueWithPropertyType:types[i]];
    STAssertNotNil(mutValue, nil);
    // Oddly the Apple APIs allow you to create a mutable multi value with
    // either a property type of kABFooPropertyType or kABMultiFooPropertyType
    // and apparently you get back basically the same thing. However if you
    // ask a type that you created with kABMultiFooPropertyType for it's type
    // it returns just kABFooPropertyType.
    STAssertEquals([mutValue propertyType], 
                   types[i] & ~kABMultiValueMask, nil);
  }
  mutValue = [GTMABMutableMultiValue valueWithPropertyType:kABStringPropertyType];
  STAssertNotNil(mutValue, nil);
  value = [[mutValue copy] autorelease];
  STAssertEqualObjects([value class], [GTMABMultiValue class], nil);
  mutValue = [[value mutableCopy] autorelease];
  STAssertEqualObjects([mutValue class], [GTMABMutableMultiValue class], nil);
  STAssertEquals([mutValue count], (NSUInteger)0, nil);
  STAssertNil([mutValue valueAtIndex:0], nil);
  STAssertNil([mutValue labelAtIndex:0], nil);
  STAssertEquals([mutValue identifierAtIndex:0], 
                 kABMultiValueInvalidIdentifier, nil);
  STAssertEquals([mutValue propertyType], 
                 (ABPropertyType)kABStringPropertyType, nil);
  ABMultiValueIdentifier ident = [mutValue addValue:nil 
                                          withLabel:kABHomeLabel];
  STAssertEquals(ident, kABMultiValueInvalidIdentifier, nil);
  ident = [mutValue addValue:@"val1" 
                   withLabel:nil];
  STAssertEquals(ident, kABMultiValueInvalidIdentifier, nil);
  ident = [mutValue insertValue:@"val1" 
                      withLabel:nil
                        atIndex:0];
  STAssertEquals(ident, kABMultiValueInvalidIdentifier, nil);
  ident = [mutValue insertValue:nil
                      withLabel:kABHomeLabel
                        atIndex:0];
  STAssertEquals(ident, kABMultiValueInvalidIdentifier, nil);
  ident = [mutValue addValue:@"val1" 
                   withLabel:kABHomeLabel];
  STAssertNotEquals(ident, kABMultiValueInvalidIdentifier, nil);
  ABMultiValueIdentifier identCheck = [mutValue identifierAtIndex:0];
  STAssertEquals(ident, identCheck, nil);
  NSUInteger idx = [mutValue indexForIdentifier:ident];
  STAssertEquals(idx, (NSUInteger)0, nil);
  STAssertTrue([mutValue replaceLabelAtIndex:0
                                   withLabel:kABWorkLabel], nil);
  STAssertFalse([mutValue replaceLabelAtIndex:10
                                    withLabel:kABWorkLabel], nil);
  STAssertTrue([mutValue replaceValueAtIndex:0
                                   withValue:@"newVal1"], nil);
  STAssertFalse([mutValue replaceValueAtIndex:10
                                    withValue:@"newVal1"], nil);
  
  STAssertEqualObjects([mutValue valueForIdentifier:ident], @"newVal1", nil);
  STAssertEqualObjects([mutValue labelForIdentifier:ident], 
                       (NSString *)kABWorkLabel, nil);
  
  ABMultiValueIdentifier ident2 = [mutValue insertValue:@"val2" 
                                              withLabel:kABOtherLabel 
                                                atIndex:0];
  STAssertNotEquals(ident2, kABMultiValueInvalidIdentifier, nil);
  STAssertNotEquals(ident2, ident, nil);
  ABMultiValueIdentifier ident3 = [mutValue insertValue:@"val3" 
                                              withLabel:kABPersonPhoneMainLabel
                                                atIndex:10];
  STAssertEquals(ident3, kABMultiValueInvalidIdentifier, nil);
  NSUInteger idx3 = [mutValue indexForIdentifier:ident3];
  STAssertEquals(idx3, (NSUInteger)NSNotFound, nil);
  STAssertTrue([mutValue removeValueAndLabelAtIndex:1], nil);
  STAssertFalse([mutValue removeValueAndLabelAtIndex:1], nil);
  
  NSUInteger idx4 
    = [mutValue indexForIdentifier:kABMultiValueInvalidIdentifier];
  STAssertEquals(idx4, (NSUInteger)NSNotFound, nil);
  
  STAssertNotNULL([mutValue multiValueRef], nil);
  
  // Enumerator test
  mutValue = [GTMABMutableMultiValue valueWithPropertyType:kABIntegerPropertyType];
  STAssertNotNil(mutValue, nil);
  for (int i = 0; i < 100; i++) {
    NSString *label = [NSString stringWithFormat:@"label %d", i];
    NSNumber *val = [NSNumber numberWithInt:i];
    STAssertNotEquals([mutValue addValue:val 
                               withLabel:(CFStringRef)label],
                      kABMultiValueInvalidIdentifier, nil);
  }
  int count = 0;
  for (NSString *label in [mutValue labelEnumerator]) {
    NSString *testLabel = [NSString stringWithFormat:@"label %d", count++];
    STAssertEqualObjects(label, testLabel, nil);
  }
  count = 0;
  value = [[mutValue copy] autorelease];
  for (NSNumber *val in [value valueEnumerator]) {
    STAssertEqualObjects(val, [NSNumber numberWithInt:count++], nil);
  }
   
  // Test messing with the values while we're enumerating them
  NSEnumerator *labelEnum = [mutValue labelEnumerator];
  NSEnumerator *valueEnum = [mutValue valueEnumerator];
  STAssertNotNil(labelEnum, nil);
  STAssertNotNil(valueEnum, nil);
  STAssertNotNil([labelEnum nextObject], nil);
  STAssertNotNil([valueEnum nextObject], nil);
  STAssertTrue([mutValue removeValueAndLabelAtIndex:0], nil);
  STAssertThrows([labelEnum nextObject], nil);
  STAssertThrows([valueEnum nextObject], nil);
  
  // Test messing with the values while we're fast enumerating them
  // Should throw an exception on the second access.
   BOOL exceptionThrown = NO;
  // Start at one because we removed index 0 above.
  count = 1;
  @try {
    for (NSString *label in [mutValue labelEnumerator]) {
      NSString *testLabel = [NSString stringWithFormat:@"label %d", count++];
      STAssertEqualObjects(label, testLabel, nil);
      STAssertTrue([mutValue removeValueAndLabelAtIndex:50], nil);
    }
  } @catch(NSException *e) {
    STAssertEqualObjects([e name], NSGenericException, @"Got %@ instead", e);
    STAssertEquals(count, 2, 
                   @"Should have caught it on the second access");
    exceptionThrown = YES;
  }  // COV_NF_LINE - because we always catch, this brace doesn't get exec'd
  STAssertTrue(exceptionThrown, @"We should have thrown an exception"
               @" because the values under the enumerator were modified");
  
}

- (void)testRadar6208390 {  
  ABPropertyType types[] = {
    kABStringPropertyType,
    kABIntegerPropertyType,
    kABRealPropertyType,
    kABDateTimePropertyType,
    kABDictionaryPropertyType
  };
  for (size_t j = 0; j < sizeof(types) / sizeof(ABPropertyType); ++j) {
    ABPropertyType type = types[j];
    ABMultiValueRef ref = ABMultiValueCreateMutable(type);
    STAssertNotNULL(ref, nil);
    NSString *label = [[NSString alloc] initWithString:@"label"];
    STAssertNotNil(label, nil);
    id val = nil;
    switch (type) {
      case kABDictionaryPropertyType:
        val = [[NSDictionary alloc] initWithObjectsAndKeys:@"1", @"1", nil];
        break;
        
      case kABStringPropertyType:
        val = [[NSString alloc] initWithFormat:@"value %d"];
        break;
        
      case kABIntegerPropertyType:
      case kABRealPropertyType:
        val = [[NSNumber alloc] initWithInt:143];
        break;
        
      case kABDateTimePropertyType:
        val = [[NSDate alloc] init];
        break;
    }
    STAssertNotNil(val, 
                   @"Testing type %d, %@", type, val);
    NSUInteger firstRetainCount = [val retainCount];
    STAssertNotEquals(firstRetainCount, 
                      (NSUInteger)0, 
                      @"Testing type %d, %@", type, val);
    
    ABMultiValueIdentifier identifier;
    STAssertTrue(ABMultiValueAddValueAndLabel(ref, 
                                              val, 
                                              (CFStringRef)label, 
                                              &identifier), 
                 @"Testing type %d, %@", type, val);    
    NSUInteger secondRetainCount = [val retainCount];
    STAssertEquals(firstRetainCount + 1, 
                   secondRetainCount,
                   @"Testing type %d, %@", type, val);
    [label release];
    [val release];
    NSUInteger thirdRetainCount = [val retainCount];
    STAssertEquals(firstRetainCount, 
                   thirdRetainCount, 
                   @"Testing type %d, %@", type, val);
    
    id oldVal = val;
    val = (id)ABMultiValueCopyValueAtIndex(ref, 0);
    NSUInteger fourthRetainCount = [val retainCount];
    
    // kABDictionaryPropertyTypes appear to do an actual copy, so the retain
    // count checking trick won't work. We only check the retain count if
    // we didn't get a new version.
    if (val == oldVal) {
      if (type == kABIntegerPropertyType 
          || type == kABRealPropertyType) {
        // We are verifying that yes indeed 6208390 is still broken
        STAssertEquals(fourthRetainCount, 
                       thirdRetainCount, 
                       @"Testing type %d, %@. If you see this error it may "
                       @"be time to update the code to change retain behaviors" 
                       @"with this os version", type, val);
      } else {
        STAssertEquals(fourthRetainCount, 
                       thirdRetainCount + 1, 
                       @"Testing type %d, %@", type, val);
        [val release];
      }
    } else {
      [val release];
    }
    CFRelease(ref);
  }
}

// Globals used by testRadar6240394.
static ABPropertyID gGTMTestID;
static const ABPropertyID *gGTMTestIDPtr;

void __attribute__((constructor))SetUpIDForTestRadar6240394(void) {
  // These must be set up BEFORE ABAddressBookCreate is called.
  gGTMTestID = kABPersonLastNameProperty;
  gGTMTestIDPtr = &kABPersonLastNameProperty;
}

- (void)testRadar6240394 {
  // As of iPhone SDK 2.1, the property IDs aren't initialized until 
  // ABAddressBookCreate is actually called. They will return zero until
  // then. Logged as radar 6240394.
  STAssertEquals(gGTMTestID, 0, @"If this isn't zero, Apple has fixed 6240394");
  (void)ABAddressBookCreate();
  STAssertEquals(*gGTMTestIDPtr, kABPersonLastNameProperty, 
                 @"If this doesn't work, something else has broken");
}
@end
