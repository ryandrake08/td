//
//  TBSetupDetailsFundingViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsFundingViewController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBCurrencyCodeTransformer.h"
#import "TBKVOTableViewCell.h"
#import "TournamentSession.h"

@implementation TBSetupDetailsFundingViewController

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    if([self object][@"forbid_after_blind_level"] == nil) {
        return [super numberOfSectionsInTableView:tableView]-1;
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

    // special handling for equity currency, must equal payout_currency
    if([indexPath section] == 3 && [indexPath row] == 1) {
        NSString* payoutCurrency = @" ";
        if([self configuration][@"payout_currency"]) {
            TBCurrencyCodeTransformer* transformer = [[TBCurrencyCodeTransformer alloc] init];
            payoutCurrency = [transformer transformedValue:[self configuration][@"payout_currency"]];
        }
        [[cell detailTextLabel] setText:payoutCurrency];
        return cell;
    }

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
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"chips"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"forbid_after_blind_level"] && [indexPath section] == 5) {
        // build up list of blind levels
        NSMutableArray* blindLevelIndices = [[NSMutableArray alloc] initWithObjects:@0, nil];
        NSArray* names = [TournamentSession blindLevelNamesForConfiguration:[self configuration]];
        for(NSUInteger i=1; i<[[self configuration][@"blind_levels"] count]; i++) {
            [blindLevelIndices addObject:@(i)];
        }
        [(TBPickableTextTableViewCell*)cell setAllowedValues:blindLevelIndices withTitles:names];
    }

    return [self setObjectToCell:cell];
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    // deselect call and reload table when "forbid after" is changed
    if([indexPath section] == 4 && [indexPath row] == 0) {
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
        [tableView reloadData];
    }
}

@end
