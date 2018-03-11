//
//  TBSetupPayoutViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupPayoutViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TournamentSession.h"
#import <Foundation/Foundation.h>

// TBSetupPayoutArrayController implements a new object
@interface TBSetupPayoutArrayController : NSArrayController

@property (copy) NSString* defaultCurrency;

@end

@implementation TBSetupPayoutArrayController

- (id)newObject {
    NSNumber* amount = @0;
    NSString* currency = [self defaultCurrency];

    return [@{@"amount":amount, @"currency":currency} mutableCopy];
}

@end

@interface TBSetupPayoutViewController () <NSTableViewDataSource>

@end

@implementation TBSetupPayoutViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // observe change to payout currency
    [[self KVOController] observe:self keyPath:@"representedObject.payout_currency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        // get currently configured currency
        id payoutCurrency = [self representedObject][@"payout_currency"];

        // set it as the default currency for new objects
        [(TBSetupPayoutArrayController*)[self arrayController] setDefaultCurrency:payoutCurrency];

        // reset all existing currencies
        for(NSMutableDictionary* payout in [[self arrayController] arrangedObjects]) {
            [payout setValue:payoutCurrency forKey:@"currency"];
        }

        // TODO: only resetting the currencies for the payouts managed by this arrayController. other payouts in configuraiton will not get reset

        // refresh
        [[self tableView] reloadData];
    }];
}

#pragma mark NSTableViewDataSource

- (id)tableView:(NSTableView*)aTableView objectValueForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    if([[aTableColumn identifier] isEqualToString:@"Place"]) {
        return @(rowIndex+1);
    }
    return nil;
}

@end
