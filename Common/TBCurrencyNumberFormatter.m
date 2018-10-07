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
    NSString* code = [[NSLocale currentLocale] objectForKey:NSLocaleCurrencyCode];
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
                name = NSLocalizedString(@"Bucks", @"Special custom currency type");
            } else if([code isEqualToString:@"XPT"]) {
                name = NSLocalizedString(@"Points", @"Special custom currency type");
            } else if(@available(iOS 10.0, macOS 10.12, *)) {
                name = [[NSLocale currentLocale] localizedStringForCurrencyCode:code];
            } else if([code isEqualToString:@"USD"]) {
                name = NSLocalizedString(@"US Dollar", @"Currency name");
            } else if([code isEqualToString:@"EUR"]) {
                name = NSLocalizedString(@"Euro", @"Currency name");
            } else if([code isEqualToString:@"INR"]) {
                name = NSLocalizedString(@"Indian Rupee", @"Currency name");
            } else if([code isEqualToString:@"GBP"]) {
                name = NSLocalizedString(@"British Pound", @"Currency name");
            } else if([code isEqualToString:@"JPY"]) {
                name = NSLocalizedString(@"Japanese Yen", @"Currency name");
            } else if([code isEqualToString:@"CNY"]) {
                name = NSLocalizedString(@"Chinese Yuan", @"Currency name");
            }
            if(name == nil) {
                name = NSLocalizedString(@"Unknown", @"Unknown currency type");
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
