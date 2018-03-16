//
//  TBSetupPayoutViewController.m
//  TBPhone
//
//  Created by Ryan Drake on 2/4/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupPayoutViewController.h"
#import "TBKVOTableViewCell.h"
#import "TBCurrencyNumberFormatter.h"
#import "TTTOrdinalNumberFormatter.h"

@implementation TBSetupPayoutViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // default is to look at forced_payouts. if a different payout is needed, previous view controller will set it
    [self setArrangedObjectsKeyPath:@"configuration.forced_payouts"];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupPayoutCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];

    if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"amount"]) {
        // place
        TTTOrdinalNumberFormatter* placeFormatter = [[TTTOrdinalNumberFormatter alloc] init];
        NSString* placeString = [NSString localizedStringWithFormat:@"%@ place", [placeFormatter stringFromNumber:@([indexPath row] + 1)]];
        [[cell textLabel] setText:placeString];

        // payout formatter
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    }

    [(TBKVOTableViewCell*)cell setObject:object];
    return cell;
}

- (id)newObject {
    return [@{@"amount":@0} mutableCopy];
}

@end
