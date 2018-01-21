//
//  TBCurrencyNumberFormatter.m
//  td
//
//  Created by Ryan Drake on 7/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBCurrencyNumberFormatter.h"

@implementation TBCurrencyNumberFormatter

- (instancetype)init {
    if(self = [super init]) {
        [self setNumberStyle:NSNumberFormatterCurrencyStyle];
    }
    return self;
}

- (NSString*)stringFromNumber:(NSNumber*)number {
    if([number isEqualToNumber:@([number longLongValue])]) {
        [self setMinimumFractionDigits:0];
    } else {
        [self setMinimumFractionDigits:2];
    }
    return [super stringFromNumber:number];
}

- (void)setCurrencyCode:(NSString *)currencyCode {
    [super setCurrencyCode:currencyCode];
    if([currencyCode isEqualToString:@"XPT"]) {
        [super setCurrencySymbol:NSLocalizedString(@"Points", nil)];
        [super setPositiveFormat:@"#,##0 ¤"];
        [super setNegativeFormat:@"#,##0 ¤"];
    } else if([currencyCode isEqualToString:@"XPB"]) {
        [super setCurrencySymbol:NSLocalizedString(@"Bucks", nil)];
        [super setPositiveFormat:@"#,##0 ¤"];
        [super setNegativeFormat:@"#,##0 ¤"];
    }
}

+ (NSArray*)supportedCurrencies {
    return @[NSLocalizedString(@"Dollar", nil),
             NSLocalizedString(@"Euro", nil),
             NSLocalizedString(@"Rupee", nil),
             NSLocalizedString(@"Pound", nil),
             NSLocalizedString(@"Yen", nil),
             NSLocalizedString(@"Yuan", nil),
             NSLocalizedString(@"Bucks", nil),
             NSLocalizedString(@"Points", nil)];
}

+ (NSArray*)supportedCodes {
    return @[@"USD",
             @"EUR",
             @"INR",
             @"GBP",
             @"JPY",
             @"CNY",
             @"XPB",
             @"XPT"];
}

+(NSDictionary*)supportedCodesForCurrencies {
    return [NSDictionary dictionaryWithObjects:[[self class] supportedCodes] forKeys:[[self class] supportedCurrencies]];
}

+(NSDictionary*)supportedCurrenciesForCodes {
    return [NSDictionary dictionaryWithObjects:[[self class] supportedCurrencies] forKeys:[[self class] supportedCodes]];
}

@end
