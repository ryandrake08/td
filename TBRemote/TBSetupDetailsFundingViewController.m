//
//  TBSetupDetailsFundingViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsFundingViewController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBKVOTableViewCell.h"
#import "TournamentSession.h"

@implementation TBSetupDetailsFundingViewController

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    if([self object][@"forbid_after_blind_level"] == nil) {
        return 1;
    }
    return [super numberOfSectionsInTableView:tableView];
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    if(section == 1 && [self object][@"forbid_after_blind_level"] != nil) {
        return 1;
    }
    return [super tableView:tableView numberOfRowsInSection:section];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"type"]) {
        [(TBPickableTextTableViewCell*)cell setAllowedValues:@[kFundingTypeBuyin, kFundingTypeRebuy, kFundingTypeAddon] withTitles:@[NSLocalizedString(@"Buyin", nil), NSLocalizedString(@"Rebuy", nil), NSLocalizedString(@"Addon", nil)]];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"cost.amount"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"cost.currency"]) {
        [(TBPickableTextTableViewCell*)cell setAllowedValues:[TBCurrencyNumberFormatter supportedCodes] withTitles:[TBCurrencyNumberFormatter supportedCurrencies]];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"commission.amount"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"commission.currency"]) {
        [(TBPickableTextTableViewCell*)cell setAllowedValues:[TBCurrencyNumberFormatter supportedCodes] withTitles:[TBCurrencyNumberFormatter supportedCurrencies]];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"equity.amount"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"equity.currency"]) {
        [(TBPickableTextTableViewCell*)cell setAllowedValues:[TBCurrencyNumberFormatter supportedCodes] withTitles:[TBCurrencyNumberFormatter supportedCurrencies]];

        // if payout_currency is set, force equity_currency to equal payout_currency
        if([self configuration][@"payout_currency"]) {
            [[(TBKVOTableViewCell*)cell object] setValue:[self configuration][@"payout_currency"] forKeyPath:@"equity.currency"];
        }
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"chips"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"forbid_after_blind_level"] && [indexPath section] == 1) {
        // build up list of blind levels
        NSMutableArray* blindLevelIndices = [[NSMutableArray alloc] initWithObjects:@0, nil];
        NSMutableArray* names = [[NSMutableArray alloc] initWithObjects:NSLocalizedString(@"Tournament Start", nil), nil];
        for(NSInteger i=1; i<[[self configuration][@"blind_levels"] count]; i++) {
            [blindLevelIndices addObject:@(i)];
            [names addObject:[NSString stringWithFormat:NSLocalizedString(@"Round %ld", @"Numbered blind level"), i]];
        }
        [(TBPickableTextTableViewCell*)cell setAllowedValues:blindLevelIndices withTitles:names];
    }

    return [self setObjectToCell:cell];
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    // deselect call and reload table when "forbid after" is changed
    if([indexPath section] == 0 && [indexPath row] == 9) {
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
        [tableView reloadData];
    }
}

@end
