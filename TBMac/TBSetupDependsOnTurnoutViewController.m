//
//  TBSetupDependsOnTurnoutViewController.m
//  TBMac
//
//  Created by Ryan Drake on 3/10/18.
//  Copyright © 2018 HDna Studio. All rights reserved.
//

#import "TBSetupDependsOnTurnoutViewController.h"
#import "TBSetupPayoutViewController.h"

// TBSetupPayoutPlayerArrayController implements a new object
@interface TBSetupPayoutPlayerArrayController : NSArrayController

@end

@implementation TBSetupPayoutPlayerArrayController

- (id)newObject {
    NSNumber* buyins_count = @2;
    NSMutableArray* payouts = [[NSMutableArray alloc] init];

    NSDictionary* last = [[self arrangedObjects] lastObject];
    if(last != nil) {
        buyins_count = @([last[@"buyins_count"] intValue] + 1);
    }

    return [@{@"buyins_count":buyins_count, @"payouts":payouts} mutableCopy];
}

@end

@interface TBSetupDependsOnTurnoutViewController () <NSTableViewDelegate>

@property (weak) TBSetupPayoutViewController* payoutViewController;

@end

@implementation TBSetupDependsOnTurnoutViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // setup sort descriptors
    NSSortDescriptor* buyinsSort = [[NSSortDescriptor alloc] initWithKey:@"buyins_count" ascending:YES];
    [[self arrayController] setSortDescriptors:@[buyinsSort]];

    // select first object
    [[self tableView] selectRowIndexes:[NSIndexSet indexSetWithIndex:0] byExtendingSelection:NO];
}

- (void)prepareForSegue:(NSStoryboardSegue*)segue sender:(id)sender {
    // reference the container view controller
    if([[segue identifier] isEqualToString:@"embedPayoutView"]) {
        [self setPayoutViewController:[segue destinationController]];

        // set represented object for payout view
        [[self payoutViewController] setRepresentedObject:[self representedObject]];

        // set payout given selected row
        [self setPayoutForSelectedRow];
    }
}

- (void)setPayoutForSelectedRow {
    // get payout array for currently selected row
    NSInteger selectedRow = [[self tableView] selectedRow];

    // find object to bind to
    NSDictionary* object = [[self arrayController] arrangedObjects][selectedRow];

    // bind payout view's array controller
    [[[self payoutViewController] arrayController] bind:@"contentArray" toObject:object withKeyPath:@"payouts" options:nil];
}

#pragma mark NSTableViewDelegate

- (void)tableViewSelectionDidChange:(NSNotification*)notification {
    // set payout given selected row
    [self setPayoutForSelectedRow];
}

@end
