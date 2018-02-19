//
//  TBFundingSourceFormatter.m
//  td
//
//  Created by Ryan Drake on 2/18/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBFundingSourceFormatter.h"
#import "TBCurrencyNumberFormatter.h"

@implementation TBFundingSourceFormatter

- (NSString*)stringForObjectValue:(id)object {
    NSString* formattedFunding = nil;

    if([object isKindOfClass:[NSDictionary class]]) {
        NSDictionary* fundingSource = (NSDictionary*)object;

        // format cost
        TBCurrencyNumberFormatter* costFormatter = [[TBCurrencyNumberFormatter alloc] init];
        [costFormatter setCurrencyCode:fundingSource[@"cost"][@"currency"]];
        NSString* formattedCost = [costFormatter stringFromNumber:fundingSource[@"cost"][@"amount"]];

        if([fundingSource[@"commission"][@"amount"] doubleValue] != 0.0) {
            // format commission
            TBCurrencyNumberFormatter* commissionFormatter = [[TBCurrencyNumberFormatter alloc] init];
            [commissionFormatter setCurrencyCode:fundingSource[@"commission"][@"currency"]];
            NSString* formattedCommission = [commissionFormatter stringFromNumber:fundingSource[@"commission"][@"amount"]];

            // combine
            formattedFunding = [NSString stringWithFormat:@"%@+%@", formattedCost, formattedCommission];
        } else {
            formattedFunding = [costFormatter stringFromNumber:fundingSource[@"cost"][@"amount"]];
        }
    }
    return formattedFunding;
}

@end
