//
//  TBCurrencyImageTransformer.m
//  td
//
//  Created by Ryan Drake on 8/27/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBCommon.h"
#import "TBCurrencyImageTransformer.h"

@implementation TBCurrencyImageTransformer

+ (Class)transformedValueClass {
    return [TBImage class];
}

+ (BOOL)allowsReverseTransformation {
    return NO;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        if([value isEqualToString:@"EUR"]) {
            return [TBImage imageNamed:@"b_note_euro"];
        } else if([value isEqualToString:@"INR"]) {
            return [TBImage imageNamed:@"b_note_rupee"];
        } else if([value isEqualToString:@"GBP"]) {
            return [TBImage imageNamed:@"b_note_sterling"];
        } else if([value isEqualToString:@"JPY"] || [value isEqualToString:@"CNY"]) {
            return [TBImage imageNamed:@"b_note_yen_yuan"];
        } else {
            // use dollar as fallback if currency is USD or not supported with a dedicated image
            return [TBImage imageNamed:@"b_note_dollar"];
        }
    }
    return nil;
}

+ (void)initialize {
    if (self == [TBCurrencyImageTransformer class]) {
        TBCurrencyImageTransformer* currencyTransformer = [[TBCurrencyImageTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:currencyTransformer
                                        forName:@"TBCurrencyImageTransformer"];
    }
}

@end
