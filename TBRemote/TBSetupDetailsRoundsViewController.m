//
//  TBSetupDetailsRoundsViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsRoundsViewController.h"
#import "TBKVOTableViewCell.h"
#import "TBDurationNumberFormatter.h"

@implementation TBSetupDetailsRoundsViewController

#pragma mark UITableViewDataSource

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    if([self object][@"break_duration"] == nil) {
        return 1;
    }
    return [super numberOfSectionsInTableView:tableView];
}

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"duration"]) {
        TBDurationNumberFormatter* durationFormatter = [[TBDurationNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:durationFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"little_blind"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"big_blind"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"ante"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"break_duration"]) {
        TBDurationNumberFormatter* durationFormatter = [[TBDurationNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:durationFormatter];
    }

    return [self setObjectToCell:cell];
}

#pragma mark UITableViewDelegate

- (void)tableView:(UITableView*)tableView didSelectRowAtIndexPath:(NSIndexPath*)indexPath {
    // deselect call and reload table when "break after" is changed
    if([indexPath section] == 0 && [indexPath row] == 4) {
       [tableView deselectRowAtIndexPath:indexPath animated:YES];
       [tableView reloadData];
    }
}

@end
