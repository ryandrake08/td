//
//  TBDateStringTransformer.m
//  td
//
//  Created by Ryan Drake on 7/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBDateStringTransformer.h"
#import "TBColor+CSS.h"
#import "NSDateFormatter+ISO8601.h"

@implementation TBDateStringTransformer

+ (Class)transformedValueClass {
    return [NSDate class];
}

+ (BOOL)allowsReverseTransformation {
    return YES;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return [[NSDateFormatter dateFormatterWithISO8601Format] dateFromString:value];
    }
    return nil;
}

- (id)reverseTransformedValue:(id)value {
    if([value isKindOfClass:[NSDate class]]) {
        return [[NSDateFormatter dateFormatterWithISO8601Format] stringFromDate:value];
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
