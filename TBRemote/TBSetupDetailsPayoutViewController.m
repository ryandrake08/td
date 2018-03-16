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

    if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"amount"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    }

    return [self setObjectToCell:cell];
}

@end
