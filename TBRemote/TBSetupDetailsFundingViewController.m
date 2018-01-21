//
//  TBSetupDetailsFundingViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsFundingViewController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBEditableTableViewCell.h"
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

    if(indexPath.section == 0) {
        switch(indexPath.row) {
            case 1:
            {
                [(TBPickableTextTableViewCell*)cell setAllowedValues:@[kFundingTypeBuyin,
                                                                       kFundingTypeRebuy,
                                                                       kFundingTypeAddon]
                                                          withTitles:@[NSLocalizedString(@"Buyin", nil),
                                                                       NSLocalizedString(@"Rebuy", nil),
                                                                       NSLocalizedString(@"Addon", nil)]];
                break;
            }
            case 2:
            {
                NSNumberFormatter* costFormatter = [[NSNumberFormatter alloc] init];
                [costFormatter setNumberStyle:NSNumberFormatterDecimalStyle];
                [(TBEditableNumberTableViewCell*)cell setFormatter:costFormatter];
                break;
            }
            case 3:
            {
                NSDictionary* currenciesForCodes = [TBCurrencyNumberFormatter supportedCurrenciesForCodes];
                [(TBPickableTextTableViewCell*)cell setAllowedValues:[currenciesForCodes allKeys] withTitles:[currenciesForCodes allValues]];
                break;
            }
            case 4:
            {
                NSNumberFormatter* commissionFormatter = [[NSNumberFormatter alloc] init];
                [commissionFormatter setNumberStyle:NSNumberFormatterDecimalStyle];
                [(TBEditableNumberTableViewCell*)cell setFormatter:commissionFormatter];
                break;
            }
            case 5:
            {
                NSDictionary* currenciesForCodes = [TBCurrencyNumberFormatter supportedCurrenciesForCodes];
                [(TBPickableTextTableViewCell*)cell setAllowedValues:[currenciesForCodes allKeys] withTitles:[currenciesForCodes allValues]];
                break;
            }
            case 6:
            {
                NSNumberFormatter* equityFormatter = [[NSNumberFormatter alloc] init];
                [equityFormatter setNumberStyle:NSNumberFormatterDecimalStyle];
                [(TBEditableNumberTableViewCell*)cell setFormatter:equityFormatter];
                break;
            }
            case 7:
            {
                NSDictionary* currenciesForCodes = [TBCurrencyNumberFormatter supportedCurrenciesForCodes];
                [(TBPickableTextTableViewCell*)cell setAllowedValues:[currenciesForCodes allKeys] withTitles:[currenciesForCodes allValues]];
                [(TBEditableTableViewCell*)cell setObject:[self object]];
                break;
            }
        }
    } else if(indexPath.section == 1) {
        switch(indexPath.row) {
            case 0:
            {
                // build up list of blind levels
                NSMutableArray* blindLevelIndices = [[NSMutableArray alloc] initWithObjects:@[@0], nil];
                NSMutableArray* names = [[NSMutableArray alloc] initWithObjects:NSLocalizedString(@"Tournament Start", nil), nil];
                for(NSInteger i=1; i<[[self configuration][@"blind_levels"] count]; i++) {
                    [blindLevelIndices addObject:@(i)];
                    [names addObject:[NSString stringWithFormat:NSLocalizedString(@"Round %ld", @"Numbered blind level"), i]];
                }
                [(TBPickableTextTableViewCell*)cell setAllowedValues:blindLevelIndices withTitles:names];
                break;
            }
        }
    }

    // pass our object to each cell
    [(TBEditableTableViewCell*)cell setObject:[self object]];
    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    // deselect call and reload table when "forbid after" is changed
    if(indexPath.section == 0 && indexPath.row == 9) {
        [tableView deselectRowAtIndexPath:indexPath animated:YES];
        [tableView reloadData];
    }
}

@end
