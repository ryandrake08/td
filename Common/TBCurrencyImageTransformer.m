//
//  TBCurrencyImageTransformer.m
//  td
//
//  Created by Ryan Drake on 8/27/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "TBCurrencyImageTransformer.h"

@implementation TBCurrencyImageTransformer

+ (Class)transformedValueClass {
    return [NSImage class];
}

+ (BOOL)allowsReverseTransformation {
    return NO;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return @{@"USD":[NSImage imageNamed:@"b_note_dollar"],
                 @"EUR":[NSImage imageNamed:@"b_note_euro"],
                 @"INR":[NSImage imageNamed:@"b_note_rupee"],
                 @"GBP":[NSImage imageNamed:@"b_note_sterling"],
                 @"JPY":[NSImage imageNamed:@"b_note_yen_yuan"],
                 @"CNY":[NSImage imageNamed:@"b_note_yen_yuan"]}[value];
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
