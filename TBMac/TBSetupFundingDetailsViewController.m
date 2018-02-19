//
//  TBSetupFundingDetailsViewController.m
//  TBMac
//
//  Created by Ryan Drake on 2/18/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupFundingDetailsViewController.h"
#import "TBCurrencyNumberFormatter.h"

@implementation TBSetupFundingDetailsViewController

- (NSArray*)currencyList {
    return [TBCurrencyNumberFormatter supportedCurrencies];
}


@end
