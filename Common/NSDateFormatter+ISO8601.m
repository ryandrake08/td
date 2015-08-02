//
//  NSDateFormatter+ISO8601.m
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "NSDateFormatter+ISO8601.h"

@implementation NSDateFormatter (ISO8601)

+ (NSDateFormatter*)dateFormatterWithISO8601Format {
    return [[NSDateFormatter alloc] initWithISO8601Format];
}

- (instancetype)initWithISO8601Format {
    if((self = [self init])) {
        [self setLocale:[NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"]];
        [self setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss"];
    }
    return self;
}

@end
