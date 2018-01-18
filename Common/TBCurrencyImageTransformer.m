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
        return @{@"USD":[TBImage imageNamed:@"b_note_dollar"],
                 @"EUR":[TBImage imageNamed:@"b_note_euro"],
                 @"INR":[TBImage imageNamed:@"b_note_rupee"],
                 @"GBP":[TBImage imageNamed:@"b_note_sterling"],
                 @"JPY":[TBImage imageNamed:@"b_note_yen_yuan"],
                 @"CNY":[TBImage imageNamed:@"b_note_yen_yuan"]}[value];
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
