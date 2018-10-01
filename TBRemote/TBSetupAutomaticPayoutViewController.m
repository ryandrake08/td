//
//  TBSetupAutomaticPayoutViewController.m
//  TBPhone
//
//  Created by Ryan Drake on 2/12/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupAutomaticPayoutViewController.h"
#import "TBKVOTableViewCell.h"
#import "TBPayoutShapeNumberFormatter.h"

@interface TBSetupAutomaticPayoutViewController ()

@end

@implementation TBSetupAutomaticPayoutViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    if([self configuration][@"automatic_payouts"] == nil) {
        [self configuration][@"automatic_payouts"] = [[NSMutableDictionary alloc] init];
    }
}

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    // create a cell
    if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"automatic_payouts.percent_seats_paid"]) {
        NSNumberFormatter* percentFormatter = [[NSNumberFormatter alloc] init];
        [percentFormatter setNumberStyle:NSNumberFormatterPercentStyle];
        [percentFormatter setMaximumFractionDigits:0];
        [(TBFormattedKVOTableViewCell*) cell setFormatter:percentFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"automatic_payouts.payout_shape"]) {
        TBPayoutShapeNumberFormatter* shapeFormatter = [[TBPayoutShapeNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:shapeFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"automatic_payouts.pay_the_bubble"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"automatic_payouts.pay_knockouts"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    }

    [(TBKVOTableViewCell*)cell setObject:[self configuration]];

    return cell;
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
}

@end
