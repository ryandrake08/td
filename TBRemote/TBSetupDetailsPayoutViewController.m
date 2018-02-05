//
//  TBSetupDetailsPayoutViewController.m
//  TBPhone
//
//  Created by Ryan Drake on 2/4/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsPayoutViewController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBKVOTableViewCell.h"

@implementation TBSetupDetailsPayoutViewController

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if(indexPath.section == 0 && indexPath.row == 1) {
        [(TBPickableTextTableViewCell*)cell setAllowedValues:[TBCurrencyNumberFormatter supportedCodes] withTitles:[TBCurrencyNumberFormatter supportedCurrencies]];
    }

    // pass our object to each cell
    [(TBKVOTableViewCell*)cell setObject:[self object]];
    return cell;
}

@end
