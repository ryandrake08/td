//
//  NSDictionary+Changed.h
//  td
//
//  Created by Ryan Drake on 9/1/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <Foundation/Foundation.h>

@interface NSDictionary (Changed)

- (NSMutableDictionary*)mutableDictionaryWithChangesFromDictionary:(NSDictionary*)other;

- (NSDictionary*)dictionaryWithChangesFromDictionary:(NSDictionary*)other;

- (NSSet*)missingKeysPresentInDictionary:(NSDictionary*)other;

@end
