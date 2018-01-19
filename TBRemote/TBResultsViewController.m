//
//  TBResultsViewController.m
//  td
//
//  Created by Ryan Drake on 8/30/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBResultsViewController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TTTOrdinalNumberFormatter.h"
#import "TournamentSession.h"
#import "NSObject+FBKVOController.h"
#import "TBAppDelegate.h"

@interface TBResultsViewController () <UITableViewDataSource>

@property (nonatomic, strong) TournamentSession* session;

// number formatters
@property (nonatomic, strong) TTTOrdinalNumberFormatter* placeFormatter;

@end

@implementation TBResultsViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // number formatters
    _placeFormatter = [[TTTOrdinalNumberFormatter alloc] init];

    // register for KVO
    [[self KVOController] observe:self keyPath:@"session.state.results" options:NSKeyValueObservingOptionInitial block:^(id observer, TBResultsViewController* object, NSDictionary *change) {
        // update table view cells
        [[observer tableView] reloadData];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return [[[self session] state][@"results"] count];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    if(indexPath.section == 0) {
        // create a cell
        UITableViewCell* cell = [tableView dequeueReusableCellWithIdentifier:@"ResultsCell" forIndexPath:indexPath];

        // get result for this row
        NSDictionary* result = [[self session] state][@"results"][indexPath.row];

        // place
        NSString* place = [[self placeFormatter] stringFromNumber:result[@"place"]];

        // payout
        TBCurrencyNumberFormatter* payoutFormatter = [[TBCurrencyNumberFormatter alloc] init];
        [payoutFormatter setCurrencyCode:result[@"payout_currency"]];
        NSString* payout = [payoutFormatter stringFromNumber:result[@"payout"]];

        // setup cell
        [(UILabel*)[cell viewWithTag:100] setText:place];
        [(UILabel*)[cell viewWithTag:101] setText:result[@"name"]];
        [(UILabel*)[cell viewWithTag:102] setText:payout];
        return cell;
    } else {
        NSLog(@"TBResultsViewController tableView:cellForRowAtIndexPath: invalid section");
        abort();
    }
}

@end
