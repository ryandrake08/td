//
//  TBDateStringTransformer.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 7/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBDateStringTransformer.h"
#import "TBColor+CSS.h"

@implementation TBDateStringTransformer

+ (Class)transformedValueClass {
    return [NSDate class];
}

+ (BOOL)allowsReverseTransformation {
    return YES;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setLocale:[NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"]];
        [dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss"];
        return [dateFormatter dateFromString:value];
    }
    return nil;
}

- (id)reverseTransformedValue:(id)value {
    if([value isKindOfClass:[NSDate class]]) {
        NSDateFormatter* dateFormatter = [[NSDateFormatter alloc] init];
        [dateFormatter setLocale:[NSLocale localeWithLocaleIdentifier:@"en_US_POSIX"]];
        [dateFormatter setDateFormat:@"yyyy-MM-dd'T'HH:mm:ss"];
        return [dateFormatter stringFromDate:value];
    }
    return nil;
}

+ (void)initialize {
    if (self == [TBDateStringTransformer class]) {
        TBDateStringTransformer* dateTransformer = [[TBDateStringTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:dateTransformer
                                        forName:@"TBDateStringTransformer"];
    }
}

@end