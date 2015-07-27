//
//  TBCurrencyNumberFormatter.m
//  td
//
//  Created by Ryan Drake on 7/26/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBCurrencyNumberFormatter.h"

@implementation TBCurrencyNumberFormatter

- (void)setCurrencyCode:(NSString *)currencyCode {
    [super setCurrencyCode:currencyCode];
    if([currencyCode isEqualToString:@"XXX"]) {
        [super setCurrencySymbol:@"₧"];
        [super setPositiveFormat:@"#,##0 ¤"];
        [super setNegativeFormat:@"#,##0 ¤"];
    } else {
        [super setCurrencySymbol:nil];
        [super setPositiveFormat:nil];
        [super setNegativeFormat:nil];
    }
}

@end
