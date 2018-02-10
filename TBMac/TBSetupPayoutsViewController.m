//
//  TBSetupPayoutsViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupPayoutsViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TournamentSession.h"
#import <Foundation/Foundation.h>

// TBSetupPayoutsArrayController implements a new object
@interface TBSetupPayoutsArrayController : NSArrayController

@property (copy) NSString* defaultCurrency;

@end

@implementation TBSetupPayoutsArrayController

- (id)newObject {
    NSNumber* amount = @0;
    NSString* currency = [self defaultCurrency];

    return [@{@"amount":amount, @"currency":currency} mutableCopy];
}

@end


@interface TBSetupPayoutsViewController () <NSTableViewDataSource>

@property (strong) NSArray* currencyList;

@end

@implementation TBSetupPayoutsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // set up currency list
    _currencyList = [TBCurrencyNumberFormatter supportedCurrencies];
    [self willChangeValueForKey:@"currencyList"];
    [self didChangeValueForKey:@"currencyList"];


    // observe change to payout currency
    [[self KVOController] observe:self keyPath:@"representedObject.payout_currency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        id payoutCurrency = [self representedObject][@"payout_currency"];
        // set default currency for new objects
        [(TBSetupPayoutsArrayController*)[self arrayController] setDefaultCurrency:payoutCurrency];
        // reset currencies
        for(NSMutableDictionary* payout in [[self arrayController] arrangedObjects]) {
            [payout setValue:payoutCurrency forKey:@"currency"];
        }
        [[self tableView] reloadData];
    }];

}

#pragma mark NSTableViewDataSource

- (id)tableView:(NSTableView*)aTableView objectValueForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    if([[aTableColumn identifier] isEqualToString:@"Round"]) {
        return @(rowIndex+1);
    }
    return nil;
}

@end
