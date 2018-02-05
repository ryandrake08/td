//
//  TBSetupPayoutViewController.m
//  TBPhone
//
//  Created by Ryan Drake on 2/4/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupPayoutViewController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TBSetupDetailsTableViewController.h"
#import "TTTOrdinalNumberFormatter.h"

@implementation TBSetupPayoutViewController

- (void)viewDidLoad {
    [super viewDidLoad];
    // default is to look at forced_payouts. if a different payout is needed, previous view controller will set it
    if([self arrangedObjects] == nil) {
        [self setArrangedObjects:[self configuration][@"forced_payouts"]];
    }
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
}

- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    UIViewController* destinationController = [segue destinationViewController];

    // if we can set a configuration, set it
    if([destinationController respondsToSelector:@selector(setConfiguration:)]) {
        [destinationController performSelector:@selector(setConfiguration:) withObject:[self configuration]];
    } else {
        NSLog(@"Warning: Segue destination does not respond to setConfiguration:");
    }

    // set arranged objects to whichever one is selected
    if([destinationController respondsToSelector:@selector(setObject:)]) {
        NSIndexPath* selectedIndexPath = [[self tableView] indexPathForSelectedRow];
        NSMutableDictionary* selectedPlace = [self arrangedObjectForIndexPath:selectedIndexPath];
        [destinationController performSelector:@selector(setObject:) withObject:selectedPlace];
    } else {
        NSLog(@"Warning: Segue destination does not respond to setObject:");
    }
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"SetupPayoutCell" forIndexPath:indexPath];
    NSDictionary* object = [super arrangedObjectForIndexPath:indexPath];

    // place
    TTTOrdinalNumberFormatter* placeFormatter = [[TTTOrdinalNumberFormatter alloc] init];
    NSString* placeString = [NSString localizedStringWithFormat:@"%@ place", [placeFormatter stringFromNumber:@([indexPath row] + 1)]];

    // payout
    TBCurrencyNumberFormatter* payoutFormatter = [[TBCurrencyNumberFormatter alloc] init];
    [payoutFormatter setCurrencyCode:object[@"currency"]];
    NSString* payoutString = [payoutFormatter stringFromNumber:object[@"amount"]];
    [[cell textLabel] setText:placeString];
    [[cell detailTextLabel] setText:payoutString];
    return cell;
}

- (id)newObject {
    return [@{@"amount":@0, @"currency":@"USD"} mutableCopy];
}

@end
