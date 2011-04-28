//
//  GTMAddressBook.m
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

#import "GTMABAddressBook.h"
#import "GTMGarbageCollection.h"

NSString *const kGTMABUnknownPropertyName = @"UNKNOWN_PROPERTY";

typedef struct {
  ABPropertyType pType;
  Class class;
} TypeClassNameMap;

@interface GTMABMultiValue ()
- (unsigned long*)mutations;
@end

@interface GTMABMutableMultiValue ()
// Checks to see if a value is a valid type to be stored in this multivalue
- (BOOL)checkValueType:(id)value;
@end

@interface GTMABMultiValueEnumerator : NSEnumerator {
 @private
  __weak ABMultiValueRef ref_;  // ref_ cached from enumeree_
  GTMABMultiValue *enumeree_;
  unsigned long mutations_;
  NSUInteger count_;
  NSUInteger index_;
  BOOL useLabels_;
}
+ (id)valueEnumeratorFor:(GTMABMultiValue*)enumeree;
+ (id)labelEnumeratorFor:(GTMABMultiValue*)enumeree;
- (id)initWithEnumeree:(GTMABMultiValue*)enumeree useLabels:(BOOL)useLabels;
- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state 
                                  objects:(id *)stackbuf 
                                    count:(NSUInteger)len;
@end

@implementation GTMABAddressBook
+ (GTMABAddressBook *)addressBook {
  return [[[self alloc] init] autorelease];
}

- (id)init {
  if ((self = [super init])) {
    addressBook_ = ABAddressBookCreate();
    if (!addressBook_) {
      // COV_NF_START
      [self release];
      self = nil;
      // COV_NF_END
    }
  }
  return self;
}

- (void)dealloc {
  if (addressBook_) {
    CFRelease(addressBook_);
  }
  [super dealloc];
}

- (BOOL)save {
  return [self saveAndReturnError:NULL];
}

- (BOOL)saveAndReturnError:(NSError **)error {
  CFErrorRef cfError = NULL;
  bool wasGood = ABAddressBookSave(addressBook_, &cfError);
  GTMCFAutorelease(cfError);
  if (error) {
    *error = (NSError *)cfError;  // COV_NF_LINE
  }
  return wasGood ? YES : NO;
}

- (BOOL)hasUnsavedChanges {
  return ABAddressBookHasUnsavedChanges(addressBook_);
}

- (void)revert {
  ABAddressBookRevert(addressBook_);
}

- (BOOL)addRecord:(GTMABRecord *)record {
  // Note: we check for bad data here because of radar
  // 6201258 Adding a NULL record using ABAddressBookAddRecord crashes
  if (!record) return NO;
  CFErrorRef cfError = NULL;
  bool wasGood = ABAddressBookAddRecord(addressBook_, 
                                        [record recordRef], &cfError);
  if (cfError) {
    // COV_NF_START
    _GTMDevLog(@"Error in [%@ %@]: %@", 
               [self class], NSStringFromSelector(_cmd), cfError);
    CFRelease(cfError);  
    // COV_NF_END
  }
  return wasGood ? YES : NO;
}

- (BOOL)removeRecord:(GTMABRecord *)record {
  // Note: we check for bad data here because of radar
  // 6201276 Removing a NULL record using ABAddressBookRemoveRecord crashes
  if (!record) return NO;
  CFErrorRef cfError = NULL;
  bool wasGood = ABAddressBookRemoveRecord(addressBook_, 
                                           [record recordRef], &cfError);
  if (cfError) {
    // COV_NF_START
    _GTMDevLog(@"Error in [%@ %@]: %@", 
               [self class], NSStringFromSelector(_cmd), cfError);
    CFRelease(cfError);
    // COV_NF_END
  }
  return wasGood ? YES : NO;
}  

- (NSArray *)people {
  NSArray *people 
    = GTMCFAutorelease(ABAddressBookCopyArrayOfAllPeople(addressBook_));
  NSMutableArray *result = [NSMutableArray arrayWithCapacity:[people count]];
  for (id person in people) {
    [result addObject:[GTMABPerson recordWithRecord:person]];
  }
  return result;
}

- (NSArray *)groups {
  NSArray *groups 
    = GTMCFAutorelease(ABAddressBookCopyArrayOfAllGroups(addressBook_));
  NSMutableArray *result = [NSMutableArray arrayWithCapacity:[groups count]];
  for (id group in groups) {
    [result addObject:[GTMABGroup recordWithRecord:group]];
  }
  return result;
}

- (ABAddressBookRef)addressBookRef {
  return addressBook_;
}

- (GTMABPerson *)personForId:(ABRecordID)uniqueId {
  GTMABPerson *person = nil;
  ABRecordRef ref = ABAddressBookGetPersonWithRecordID(addressBook_, uniqueId);
  if (ref) {
    person = [GTMABPerson recordWithRecord:ref];
  }
  return person;
}

- (GTMABGroup *)groupForId:(ABRecordID)uniqueId {
  GTMABGroup *group = nil;
  ABRecordRef ref = ABAddressBookGetGroupWithRecordID(addressBook_, uniqueId);
  if (ref) {
    group = [GTMABGroup recordWithRecord:ref];
  }
  return group;
}

+ (NSString *)localizedLabel:(CFStringRef)label {
  return GTMCFAutorelease(ABAddressBookCopyLocalizedLabel(label));
}

@end

@implementation GTMABRecord
+ (id)recordWithRecord:(ABRecordRef)record {
  return [[[self alloc] initWithRecord:record] autorelease];
}

- (id)initWithRecord:(ABRecordRef)record {
  if ((self = [super init])) {
    if ([self class] == [GTMABRecord class]) {
      [self autorelease];
      [self doesNotRecognizeSelector:_cmd];
    }
    if (!record) {
      [self release];
      self = nil;
    } else {
      record_ = CFRetain(record);
    }
  }
  return self;
}

- (NSUInteger)hash {
  // This really isn't completely valid due to
  // 6203836 ABRecords hash to their address
  // but it's the best we can do without knowing what properties
  // are in a record, and we don't have an API for that.
  return CFHash(record_);
}

- (BOOL)isEqual:(id)object {
  // This really isn't completely valid due to
  // 6203836 ABRecords hash to their address
  // but it's the best we can do without knowing what properties
  // are in a record, and we don't have an API for that.
  return [object respondsToSelector:@selector(recordRef)] 
    && CFEqual(record_, [object recordRef]);
}

- (void)dealloc {
  if (record_) {
    CFRelease(record_);
  }
  [super dealloc];
}

- (ABRecordRef)recordRef {
  return record_;
}

- (ABRecordID)recordID {
  return ABRecordGetRecordID(record_);
}

- (id)valueForProperty:(ABPropertyID)property {
  id value = GTMCFAutorelease(ABRecordCopyValue(record_, property));
  if (value) {
    if ([[self class] typeOfProperty:property] & kABMultiValueMask) {
      value = [[[GTMABMultiValue alloc] initWithMultiValue:value] autorelease];
    }
  }
  return value;
}

- (BOOL)setValue:(id)value forProperty:(ABPropertyID)property {
  if (!value) return NO;
  // We check the type here because of
  // Radar 6201046 ABRecordSetValue returns true even if you pass in a bad type
  //               for a value  
  TypeClassNameMap fullTypeMap[] = {
    { kABStringPropertyType, [NSString class] },
    { kABIntegerPropertyType, [NSNumber class] },
    { kABRealPropertyType, [NSNumber class] },
    { kABDateTimePropertyType, [NSDate class] },
    { kABDictionaryPropertyType, [NSDictionary class] },
    { kABMultiStringPropertyType, [GTMABMultiValue class] },
    { kABMultiRealPropertyType, [GTMABMultiValue class] },
    { kABMultiDateTimePropertyType, [GTMABMultiValue class] },
    { kABMultiDictionaryPropertyType, [GTMABMultiValue class] }
  };
  ABPropertyType type = [[self class] typeOfProperty:property];
  BOOL wasFound = NO;
  for (size_t i = 0; i < sizeof(fullTypeMap) / sizeof(TypeClassNameMap); ++i) {
    if (fullTypeMap[i].pType == type) {
      wasFound = YES;
      if (![[value class] isSubclassOfClass:fullTypeMap[i].class]) {
        return NO;
      }
    }
  }
  if (!wasFound) {
    return NO;
  }
  if (type & kABMultiValueMask) {
    value = (id)[value multiValueRef];
  }
  CFErrorRef cfError = nil;
  bool wasGood = ABRecordSetValue(record_, property, (CFTypeRef)value, &cfError);
  if (cfError) {
    // COV_NF_START
    _GTMDevLog(@"Error in [%@ %@]: %@", 
               [self class], NSStringFromSelector(_cmd), cfError);
    CFRelease(cfError);
    // COV_NF_END
  }
  return wasGood ? YES : NO;
}

- (BOOL)removeValueForProperty:(ABPropertyID)property {
  CFErrorRef cfError = nil;
  // We check to see if the value is in the property because of:
  // Radar 6201005 ABRecordRemoveValue returns true for value that aren't 
  //               in the record
  id value = [self valueForProperty:property];
  bool wasGood = value && ABRecordRemoveValue(record_, property, &cfError);
  if (cfError) {
    // COV_NF_START
    _GTMDevLog(@"Error in [%@ %@]: %@", 
               [self class], NSStringFromSelector(_cmd), cfError);
    CFRelease(cfError);
    // COV_NF_END
  }
  return wasGood ? YES : NO;
}

- (NSString *)compositeName {
  return GTMCFAutorelease(ABRecordCopyCompositeName(record_));
}

// COV_NF_START
// Both of these methods are to be overridden by their subclasses
+ (ABPropertyType)typeOfProperty:(ABPropertyID)property {
  [self doesNotRecognizeSelector:_cmd];
  return kABInvalidPropertyType;
}

+ (NSString *)localizedPropertyName:(ABPropertyID)property {
  [self doesNotRecognizeSelector:_cmd];
  return nil; 
}
// COV_NF_END
@end

@implementation GTMABPerson

+ (GTMABPerson *)personWithFirstName:(NSString *)first 
                            lastName:(NSString *)last {
  GTMABPerson *person = [[[self alloc] init] autorelease];
  if (person) {
    BOOL isGood = YES;
    if (first) {
      isGood = [person setValue:first forProperty:kABPersonFirstNameProperty];
    }
    if (isGood && last) {
      isGood = [person setValue:last forProperty:kABPersonLastNameProperty];
    }
    if (!isGood) {
      // COV_NF_START
      // Marked as NF because I don't know how to force an error
      [person release];
      person = nil;
      // COV_NF_END
    }
  }
  return person;
}

- (id)init {
  ABRecordRef person = ABPersonCreate();
  self = [super initWithRecord:person];
  if (person) {
    CFRelease(person);
  } 
  return self;
}

- (BOOL)setImageData:(NSData *)data {
  CFErrorRef cfError = NULL;
  bool wasGood = NO;
  if (!data) {
    wasGood = ABPersonRemoveImageData([self recordRef], &cfError);
  } else {
    // We verify that the data is good because of:
    // Radar 6202868 ABPersonSetImageData should validate image data
    UIImage *image = [UIImage imageWithData:data];
    wasGood = image && ABPersonSetImageData([self recordRef], 
                                            (CFDataRef)data, &cfError);
  }
  if (cfError) {
    // COV_NF_START
    _GTMDevLog(@"Error in [%@ %@]: %@", 
               [self class], NSStringFromSelector(_cmd), cfError);
    CFRelease(cfError);
    // COV_NF_END
  }
  return wasGood ? YES : NO;
}

- (UIImage *)image {
  return [UIImage imageWithData:[self imageData]];
}

- (BOOL)setImage:(UIImage *)image {
  NSData *data = UIImagePNGRepresentation(image);
  return [self setImageData:data];
}

- (NSData *)imageData {
  return GTMCFAutorelease(ABPersonCopyImageData([self recordRef]));
}

- (NSString *)description {
  return [NSString stringWithFormat:@"%@ %@ %@ %d", 
          [self class], 
          [self valueForProperty:kABPersonFirstNameProperty],
          [self valueForProperty:kABPersonLastNameProperty],
          [self recordID]];
}

+ (NSString *)localizedPropertyName:(ABPropertyID)property {
  return GTMCFAutorelease(ABPersonCopyLocalizedPropertyName(property)); 
}

+ (ABPersonCompositeNameFormat)compositeNameFormat {
  return ABPersonGetCompositeNameFormat();
}

+ (ABPropertyType)typeOfProperty:(ABPropertyID)property {
  return ABPersonGetTypeOfProperty(property);
}
@end

@implementation GTMABGroup

+ (GTMABGroup *)groupNamed:(NSString *)name {
  GTMABGroup *group = [[[self alloc] init] autorelease];
  if (group) {
    if (![group setValue:name forProperty:kABGroupNameProperty]) {
      // COV_NF_START
      // Can't get setValue to fail for me
      [group release];
      group = nil;
      // COV_NF_END
    }
  }
  return group;
}

- (id)init {
  ABRecordRef group = ABGroupCreate();
  self = [super initWithRecord:group];
  if (group) {
    CFRelease(group);
  } 
  return self;
}

- (NSArray *)members {
  NSArray *people 
    = GTMCFAutorelease(ABGroupCopyArrayOfAllMembers([self recordRef]));
  NSMutableArray *gtmPeople = [NSMutableArray arrayWithCapacity:[people count]];
  for (id person in people) {
    [gtmPeople addObject:[GTMABPerson recordWithRecord:(ABRecordRef)person]];
  }
  return gtmPeople;
}  

- (BOOL)addMember:(GTMABPerson *)person {
  CFErrorRef cfError = nil;
  // We check for person because of
  // Radar 6202860 Passing nil person into ABGroupAddMember crashes
  bool wasGood = person && ABGroupAddMember([self recordRef], 
                                            [person recordRef], &cfError);
  if (cfError) {
    // COV_NF_START
    _GTMDevLog(@"Error in [%@ %@]: %@", 
               [self class], NSStringFromSelector(_cmd), cfError);
    CFRelease(cfError);
    // COV_NF_END
  }
  return wasGood ? YES : NO;
}  

- (BOOL)removeMember:(GTMABPerson *)person {
  CFErrorRef cfError = nil;
  // We check for person because of
  // Radar 6202860 Passing nil person into ABGroupAddMember crashes
  // (I know this is remove, but it crashes there too)
  bool wasGood = person && ABGroupRemoveMember([self recordRef], 
                                               [person recordRef], &cfError);
  if (cfError) {
    // COV_NF_START
    _GTMDevLog(@"Error in [%@ %@]: %@", 
               [self class], NSStringFromSelector(_cmd), cfError);
    CFRelease(cfError);
    // COV_NF_END
  }
  return wasGood ? YES : NO;
}  

+ (ABPropertyType)typeOfProperty:(ABPropertyID)property {
  ABPropertyType type = kABInvalidPropertyType;
  if (property == kABGroupNameProperty) {
    type = kABStringPropertyType;
  } 
  return type;
}

+ (NSString *)localizedPropertyName:(ABPropertyID)property {
  NSString *name = kGTMABUnknownPropertyName;
  if (property == kABGroupNameProperty) {
    name = NSLocalizedStringFromTable(@"Name",
                                      @"GTMABAddressBook", 
                                      @"name property");
  }
  return name;
}

- (NSString *)description {
  return [NSString stringWithFormat:@"%@ %@ %d", 
          [self class], 
          [self valueForProperty:kABGroupNameProperty],
          [self recordID]];
}
@end

@implementation GTMABMultiValue
- (id)init {
  // Call super init and release so we don't leak
  [[super init] autorelease];
  [self doesNotRecognizeSelector:_cmd];
  return nil;  // COV_NF_LINE
}

- (id)initWithMultiValue:(ABMultiValueRef)multiValue {
  if ((self = [super init])) {
    if (!multiValue) {
      [self release];
      self = nil;
    } else {
      multiValue_ = CFRetain(multiValue);
    }
  }
  return self;
}

- (id)copyWithZone:(NSZone *)zone {
  return [[GTMABMultiValue alloc] initWithMultiValue:multiValue_];
}

- (id)mutableCopyWithZone:(NSZone *)zone {
  return [[GTMABMutableMultiValue alloc] initWithMultiValue:multiValue_];
}

- (NSUInteger)hash {
  // I'm implementing hash instead of using CFHash(multiValue_) because
  // 6203854 ABMultiValues hash to their address
  NSUInteger count = [self count];
  NSUInteger hash = 0;
  for (NSUInteger i = 0; i < count;  ++i) {
    NSString *label = [self labelAtIndex:i];
    id value = [self valueAtIndex:i];
    hash += [label hash];
    hash += [value hash];
  }
  return hash;
}

- (BOOL)isEqual:(id)object {
  // I'm implementing isEqual instea of using CFEquals(multiValue,...) because
  // 6203854 ABMultiValues hash to their address
  // and it appears CFEquals just calls through to hash to compare them.
  BOOL isEqual = NO;
  if ([object respondsToSelector:@selector(multiValueRef)]) { 
    isEqual = multiValue_ == [object multiValueRef];
    if (!isEqual) {
      NSUInteger count = [self count];
      NSUInteger objCount = [object count];
      isEqual = count == objCount;
      for (NSUInteger i = 0; isEqual && i < count;  ++i) {
        NSString *label = [self labelAtIndex:i];
        NSString *objLabel = [object labelAtIndex:i];
        isEqual = [label isEqual:objLabel];
        if (isEqual) {
          id value = [self valueAtIndex:i];
          id objValue = [object valueAtIndex:i];
          isEqual = [value isEqual:objValue];
        }
      }
    }
  }
  return isEqual;
}

- (void)dealloc {
  if (multiValue_) {
    CFRelease(multiValue_);
  }
  [super dealloc];
}

- (ABMultiValueRef)multiValueRef {
  return multiValue_;
}

- (NSUInteger)count {
  return ABMultiValueGetCount(multiValue_);
}

- (id)valueAtIndex:(NSUInteger)idx {
  id value = nil;
  if (idx < [self count]) {
    value = GTMCFAutorelease(ABMultiValueCopyValueAtIndex(multiValue_, idx));
    ABPropertyType type = [self propertyType];
    if (type == kABIntegerPropertyType 
        || type == kABRealPropertyType
        || type == kABDictionaryPropertyType) {
      // This is because of
      // 6208390 Integer and real values don't work in ABMultiValueRefs
      // Apparently they forget to add a ref count on int, real and 
      // dictionary values in ABMultiValueCopyValueAtIndex, although they do 
      // remember them for all other types.
      // Once they fix this, this will lead to a leak, but I figure the leak
      // is better than the crash. Our unittests will test to make sure that
      // this is the case, and once we find a system that has this fixed, we
      // can conditionalize this code. Look for testRadar6208390 in
      // GTMABAddressBookTest.m
      // Also, search for 6208390 below and fix the fast enumerator to actually
      // be somewhat performant when this is fixed.
      [value retain];
    }
  }
  return value;
}

- (NSString *)labelAtIndex:(NSUInteger)idx {
  NSString *label = nil;
  if (idx < [self count]) {
    label = GTMCFAutorelease(ABMultiValueCopyLabelAtIndex(multiValue_, idx));
  }
  return label;
}

- (ABMultiValueIdentifier)identifierAtIndex:(NSUInteger)idx {
  ABMultiValueIdentifier identifier = kABMultiValueInvalidIdentifier;
  if (idx < [self count]) {
    identifier = ABMultiValueGetIdentifierAtIndex(multiValue_, idx);
  }
  return identifier;
}

- (NSUInteger)indexForIdentifier:(ABMultiValueIdentifier)identifier {
  NSUInteger idx = ABMultiValueGetIndexForIdentifier(multiValue_, identifier);
  return idx == (NSUInteger)kCFNotFound ? (NSUInteger)NSNotFound : idx;
}

- (ABPropertyType)propertyType {
  return ABMultiValueGetPropertyType(multiValue_);
}

- (id)valueForIdentifier:(ABMultiValueIdentifier)identifier {
  return [self valueAtIndex:[self indexForIdentifier:identifier]];
}

- (NSString *)labelForIdentifier:(ABMultiValueIdentifier)identifier {
  return [self labelAtIndex:[self indexForIdentifier:identifier]];
}

- (unsigned long*)mutations {
  // We just need some constant non-zero value here so fast enumeration works.
  // Dereferencing self should give us the isa which will stay constant
  // over the enumeration.
  return (unsigned long*)self;
}

- (NSEnumerator *)valueEnumerator {
  return [GTMABMultiValueEnumerator valueEnumeratorFor:self];
}

- (NSEnumerator *)labelEnumerator {
  return [GTMABMultiValueEnumerator labelEnumeratorFor:self];
}

@end

@implementation GTMABMutableMultiValue
+ (id)valueWithPropertyType:(ABPropertyType)type {
  return [[[self alloc] initWithPropertyType:type] autorelease];
}

- (id)initWithPropertyType:(ABPropertyType)type {
  ABMutableMultiValueRef ref = nil;
  if (type != kABInvalidPropertyType) {
    ref = ABMultiValueCreateMutable(type);
  }
  self = [super initWithMultiValue:ref];
  if (ref) {
    CFRelease(ref);
  } 
  return self;
}

- (id)initWithMultiValue:(ABMultiValueRef)multiValue {
  ABMutableMultiValueRef ref = nil;
  if (multiValue) {
    ref = ABMultiValueCreateMutableCopy(multiValue);
  }
  self = [super initWithMultiValue:ref];
  if (ref) {
    CFRelease(ref);
  } 
  return self;
}

- (id)initWithMutableMultiValue:(ABMutableMultiValueRef)multiValue {
  return [super initWithMultiValue:multiValue];
}

- (BOOL)checkValueType:(id)value {
  BOOL isGood = NO;
  if (value) {
    TypeClassNameMap singleValueTypeMap[] = {
      { kABStringPropertyType, [NSString class] },
      { kABIntegerPropertyType, [NSNumber class] },
      { kABRealPropertyType, [NSNumber class] },
      { kABDateTimePropertyType, [NSDate class] },
      { kABDictionaryPropertyType, [NSDictionary class] },
    };
    ABPropertyType type = [self propertyType];
    for (size_t i = 0; 
         i < sizeof(singleValueTypeMap) / sizeof(TypeClassNameMap); ++i) {
      if (singleValueTypeMap[i].pType == type) {
        if ([[value class] isSubclassOfClass:singleValueTypeMap[i].class]) {
          isGood = YES;
          break;
        }
      }
    }
  }
  return isGood;
}

- (ABMultiValueIdentifier)addValue:(id)value withLabel:(CFStringRef)label {
  ABMultiValueIdentifier identifier = kABMultiValueInvalidIdentifier;
  // We check label and value here because of
  // radar 6202827  Passing nil info ABMultiValueAddValueAndLabel causes crash
  if (!label 
      || ![self checkValueType:value] 
      || !ABMultiValueAddValueAndLabel(multiValue_, 
                                       value, 
                                       label, 
                                       &identifier)) {
    identifier = kABMultiValueInvalidIdentifier;
  } else {
    mutations_++;
  }
  return identifier;
}

- (ABMultiValueIdentifier)insertValue:(id)value 
                            withLabel:(CFStringRef)label 
                              atIndex:(NSUInteger)idx {
  ABMultiValueIdentifier identifier = kABMultiValueInvalidIdentifier;
  // We perform a check here to ensure that we don't get bitten by
  // Radar 6202807 ABMultiValueInsertValueAndLabelAtIndex allows you to insert 
  //               values past end
  NSUInteger count = [self count];
  // We check label and value here because of
  // radar 6202827  Passing nil info ABMultiValueAddValueAndLabel causes crash
  if (idx > count
      || !label 
      || ![self checkValueType:value] 
      || !ABMultiValueInsertValueAndLabelAtIndex(multiValue_, 
                                                 value, 
                                                 label, 
                                                 idx, 
                                                 &identifier)) {
    identifier = kABMultiValueInvalidIdentifier;
  } else {
    mutations_++;
  }
  return identifier;
}

- (BOOL)removeValueAndLabelAtIndex:(NSUInteger)idx {
  BOOL isGood = NO;
  NSUInteger count = [self count];
  if (idx < count) {
    if (ABMultiValueRemoveValueAndLabelAtIndex(multiValue_, 
                                               idx)) {
      mutations_++;
      isGood = YES;
    }
  }
  return isGood; 
}

- (BOOL)replaceValueAtIndex:(NSUInteger)idx withValue:(id)value {
  BOOL isGood = NO;
  NSUInteger count = [self count];
  if (idx < count && [self checkValueType:value]) {
    if (ABMultiValueReplaceValueAtIndex(multiValue_, 
                                        value, idx)) {
      mutations_++;
      isGood = YES;
    }
  }
  return isGood; 
}

- (BOOL)replaceLabelAtIndex:(NSUInteger)idx withLabel:(CFStringRef)label{
  BOOL isGood = NO;
  NSUInteger count = [self count];
  if (idx < count) {
    if (ABMultiValueReplaceLabelAtIndex(multiValue_, 
                                        label, 
                                        idx)) {
      mutations_++;
      isGood = YES;
    }
  }
  return isGood; 
}
      
- (unsigned long*)mutations {
  return &mutations_;
}
@end
      

@implementation GTMABMultiValueEnumerator

+ (id)valueEnumeratorFor:(GTMABMultiValue*)enumeree {
  return [[[self alloc] initWithEnumeree:enumeree useLabels:NO] autorelease];
}

+ (id)labelEnumeratorFor:(GTMABMultiValue*)enumeree  {
  return [[[self alloc] initWithEnumeree:enumeree useLabels:YES] autorelease];
}

- (id)initWithEnumeree:(GTMABMultiValue*)enumeree useLabels:(BOOL)useLabels {
  if ((self = [super init])) {
    if (enumeree) {
      enumeree_ = [enumeree retain];
      useLabels_ = useLabels;
    } else {
      // COV_NF_START
      // Since this is a private class where the enumeree creates us
      // there is no way we should ever get here.
      [self release];
      self = nil;
      // COV_NF_END
    }
  }
  return self;
}

- (void)dealloc {
  [enumeree_ release];
  [super dealloc];
}

- (NSUInteger)countByEnumeratingWithState:(NSFastEnumerationState *)state 
                                  objects:(id *)stackbuf 
                                    count:(NSUInteger)len {
  NSUInteger i;
  if (!ref_) {
    count_ = [enumeree_ count];
    ref_ = [enumeree_ multiValueRef];
  }
  
  for (i = 0; state->state < count_ && i < len; ++i, ++state->state) {
    if (useLabels_) {
      stackbuf[i] = GTMCFAutorelease(ABMultiValueCopyLabelAtIndex(ref_, 
                                                                  state->state));
    } else {
      // Yes this is slow, but necessary in light of radar 6208390
      // Once this is fixed we can go to something similar to the label
      // case which should speed stuff up again. Hopefully anybody who wants
      // real performance is willing to move down to the C API anyways.
      stackbuf[i] = [enumeree_ valueAtIndex:state->state];
    }
  }
    
  state->itemsPtr = stackbuf;
  state->mutationsPtr = [enumeree_ mutations];
  return i;
}

- (id)nextObject {
  id value = nil;
  if (!ref_) {
    count_ = [enumeree_ count];
    mutations_ = *[enumeree_ mutations];
    ref_ = [enumeree_ multiValueRef];

  }
  if (mutations_ != *[enumeree_ mutations]) {
    NSString *reason = [NSString stringWithFormat:@"*** Collection <%@> was "
                        "mutated while being enumerated", enumeree_];
    [[NSException exceptionWithName:NSGenericException
                             reason:reason
                           userInfo:nil] raise];
  }
  if (index_ < count_) {
    if (useLabels_) {
      value = GTMCFAutorelease(ABMultiValueCopyLabelAtIndex(ref_, 
                                                            index_));
    } else {
      // Yes this is slow, but necessary in light of radar 6208390
      // Once this is fixed we can go to something similar to the label
      // case which should speed stuff up again. Hopefully anybody who wants
      // real performance is willing to move down to the C API anyways.
      value = [enumeree_ valueAtIndex:index_];
    }
    index_ += 1;
  }
  return value;
}
@end

