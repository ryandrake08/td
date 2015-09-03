//
//  NSDictionary+Changed.m
//  td
//
//  Created by Ryan Drake on 9/1/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "NSDictionary+Changed.h"

@implementation NSDictionary (Changed)

- (NSDictionary*)dictionaryWithChangesFromDictionary:(NSDictionary*)other {
    NSMutableDictionary* changes = [NSMutableDictionary dictionary];
    [self enumerateKeysAndObjectsUsingBlock:^(id key, id obj, BOOL* stop) {
        if(![obj isEqual:other[key]]) {
            changes[key] = obj;
        }
    }];
    return [NSDictionary dictionaryWithDictionary:changes];
}

@end
