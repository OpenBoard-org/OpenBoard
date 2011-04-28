//
//  GTMNSEnumerator+Filter.h
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

/// A generic category for methods that allow us to filter enumeratable
/// containers, inspired by C++ Standard Library's use of iterators.
/// Like in C++, these assume the underlying container is not modified during
/// the lifetime of the iterator.
///
@interface NSEnumerator (GTMEnumeratorFilterAdditions)

/// @argument predicate - the function return BOOL. will be applied to each element
/// @argument argument - optional argument to pass to predicate
/// @returns an enumerator that contains only elements where [element sel:argument] is true
- (NSEnumerator *)gtm_filteredEnumeratorByMakingEachObjectPerformSelector:(SEL)predicate
                                                               withObject:(id)argument;

/// @argument selector - the function return a transformed object. will be applied to each element
/// @argument argument - optional argument to pass to transformer
/// @returns an enumerator that contains the transformed elements
- (NSEnumerator *)gtm_enumeratorByMakingEachObjectPerformSelector:(SEL)selector
                                                       withObject:(id)argument;

/// @argument target - receiver for each method
/// @argument predicate - as  in, [target predicate: [self nextObject]], return a BOOL
/// @returns an enumerator that contains only elements where [element sel:argument] is true
- (NSEnumerator *)gtm_filteredEnumeratorByTarget:(id)target
                           performOnEachSelector:(SEL)predicate;

/// @argument target - receiver for each method
/// @argument sel - as  in, [target selector: [self nextObject]], return a transformed object
/// @returns an enumerator that contains the transformed elements
- (NSEnumerator *)gtm_enumeratorByTarget:(id)target
                   performOnEachSelector:(SEL)selector;

@end

