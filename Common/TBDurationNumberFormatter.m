//
//  TBDurationNumberFormatter.m
//  td
//
//  Created by Ryan Drake on 3/13/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBDurationNumberFormatter.h"

@implementation TBDurationNumberFormatter

- (NSString*)stringForObjectValue:(id)anObject {

    if(![anObject isKindOfClass:[NSNumber class]]) {
        return nil;
    }

    NSInteger duration = [anObject integerValue];
    NSInteger minutes = (duration / 60000) % 60;
    NSInteger hours = (duration / 3600000);
    return [NSString stringWithFormat:@"%02ld:%02ld", (long)hours, (long)minutes];
}

- (BOOL)getObjectValue:(id*)obj forString:(NSString*)string errorDescription:(NSString**)error {

    NSScanner* scanner = [NSScanner scannerWithString:string];
    NSInteger hours, minutes;
    if([scanner scanInteger:&hours] &&
       [scanner scanString:@":" intoString:NULL] &&
       [scanner scanInteger:&minutes] &&
       [scanner isAtEnd]) {
        if(obj) {
            *obj = @(hours * 3600000 + minutes * 60000);
            return YES;
        }
    } else {
        if(error) {
            *error = NSLocalizedString(@"Format must be hours:minutes", @"Error converting");
        }
    }
    return NO;
}

@end
