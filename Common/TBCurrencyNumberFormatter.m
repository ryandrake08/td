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

- (void)setCurrencyCode:(NSString*)currencyCode {
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

+ (NSString*)defaultCurrencyCode {
    // pick a default currency based on user's configured locale
    NSString* code = [[NSLocale currentLocale] currencyCode];
    if([[[self class] supportedCodes] containsObject:code]) {
        return code;
    } else {
        return @"USD";
    }
}

+ (NSArray*)supportedCurrencies {
    static NSMutableArray* currencyNames = nil;
    if(currencyNames == nil) {
        currencyNames = [[NSMutableArray alloc] init];
        for(NSString* code in [[self class] supportedCodes]) {
            NSString* name;
            if([code isEqualToString:@"XPB"]) {
                name = NSLocalizedString(@"Bucks", nil);
            } else if([code isEqualToString:@"XPT"]) {
                name = NSLocalizedString(@"Points", nil);
            } else {
                name = [[NSLocale currentLocale] localizedStringForCurrencyCode:code];
            }
            if(name == nil) {
                name = NSLocalizedString(@"Unknown Currency", nil);
            }
            [currencyNames addObject:name];
        }
    }
    return currencyNames;
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

@end
