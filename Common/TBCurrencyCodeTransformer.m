//
//  TBCurrencyCodeTransformer.m
//  td
//
//  Created by Ryan Drake on 8/23/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBCurrencyCodeTransformer.h"

@implementation TBCurrencyCodeTransformer

+ (Class)transformedValueClass {
    return [NSString class];
}

+ (BOOL)allowsReverseTransformation {
    return YES;
}

- (id)transformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return @{@"USD":NSLocalizedString(@"Dollar", nil),
                 @"EUR":NSLocalizedString(@"Euro", nil),
                 @"INR":NSLocalizedString(@"Rupee", nil),
                 @"GBP":NSLocalizedString(@"Pound", nil),
                 @"JPY":NSLocalizedString(@"Yen", nil),
                 @"CNY":NSLocalizedString(@"Yuan", nil),
                 @"XXX":NSLocalizedString(@"Points", nil)}[value];
    }
    return nil;
}

- (id)reverseTransformedValue:(id)value {
    if([value isKindOfClass:[NSString class]]) {
        return @{NSLocalizedString(@"Dollar", nil)  :@"USD",
                 NSLocalizedString(@"Euro", nil)    :@"EUR",
                 NSLocalizedString(@"Rupee", nil)   :@"INR",
                 NSLocalizedString(@"Pound", nil)   :@"GBP",
                 NSLocalizedString(@"Yen", nil)     :@"JPY",
                 NSLocalizedString(@"Yuan", nil)    :@"CNY",
                 NSLocalizedString(@"Points", nil)  :@"XXX"}[value];
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
