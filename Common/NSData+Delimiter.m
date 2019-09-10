//
//  NSData+Delimiter.m
//  td
//
//  Created by Ryan Drake on 1/3/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "NSData+Delimiter.h"

@implementation NSData (Delimiter)

- (NSRange)rangeOfDataDelimitedBy:(uint8_t)delimiter {
    return [self rangeOfDataDelimetedBy:delimiter range:NSMakeRange(0, [self length])];
}

- (NSRange)rangeOfDataDelimetedBy:(uint8_t)delimiter range:(NSRange)searchRange {
    // look for a byte, return range up to and including byte
    const uint8_t* bytes = [self bytes];
    for(NSUInteger i=0; i<searchRange.length; i++) {
        if(bytes[i+searchRange.location] == delimiter) {
            return NSMakeRange(searchRange.location, searchRange.location+i+1);
        }
    }

    return NSMakeRange(0, 0);
}

@end
