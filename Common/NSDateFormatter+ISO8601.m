//
//  NSDateFormatter+ISO8601.m
//  td
//
//  Created by Ryan Drake on 8/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "NSDateFormatter+ISO8601.h"

@implementation NSDateFormatter (ISO8601)

+ (NSDateFormatter*)iso8601DateFormatter {
    NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
    [dateFormatter setLocale:[NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"]];
    [dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss"];
    return dateFormatter;
}

@end
