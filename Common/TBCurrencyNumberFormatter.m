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

+(NSDictionary*)supportedCodesForCurrencies {
    return @{NSLocalizedString(@"Dollar", nil)  :@"USD",
             NSLocalizedString(@"Euro", nil)    :@"EUR",
             NSLocalizedString(@"Rupee", nil)   :@"INR",
             NSLocalizedString(@"Pound", nil)   :@"GBP",
             NSLocalizedString(@"Yen", nil)     :@"JPY",
             NSLocalizedString(@"Yuan", nil)    :@"CNY",
             NSLocalizedString(@"Bucks", nil)   :@"XPB",
             NSLocalizedString(@"Points", nil)  :@"XPT"};
}

+(NSDictionary*)supportedCurrenciesForCodes {
    return @{@"USD":NSLocalizedString(@"Dollar", nil),
             @"EUR":NSLocalizedString(@"Euro", nil),
             @"INR":NSLocalizedString(@"Rupee", nil),
             @"GBP":NSLocalizedString(@"Pound", nil),
             @"JPY":NSLocalizedString(@"Yen", nil),
             @"CNY":NSLocalizedString(@"Yuan", nil),
             @"XPB":NSLocalizedString(@"Bucks", nil),
             @"XPT":NSLocalizedString(@"Points", nil)};
}

@end
