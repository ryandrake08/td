//
//  TBCurrencyCodeTransformer.m
//  td
//
//  Created by Ryan Drake on 8/23/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBCurrencyCodeTransformer.h"
#import "TBCurrencyNumberFormatter.h"

@implementation TBCurrencyCodeTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

+ (BOOL)allowsReverseTransformation {
    return YES;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return [TBCurrencyNumberFormatter supportedCurrenciesForCodes][value];
    }
    return nil;
}

- (id)reverseTransformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return[TBCurrencyNumberFormatter supportedCodesForCurrencies][value];
    }
    return nil;
}

+ (void)initialize {
    if (self == [TBCurrencyCodeTransformer class]) {
        TBCurrencyCodeTransformer* currencyTransformer = [[TBCurrencyCodeTransformer alloc] init];
        // register it with the name that we refer to it with
        [NSValueTransformer setValueTransformer:currencyTransformer
                                        forName:@"TBCurrencyCodeTransformer"];
    }
}

@end
