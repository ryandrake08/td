//
//  TBSetupDetailsChipsViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsChipsViewController.h"
#import "TBKVOTableViewCell.h"

@implementation TBSetupDetailsChipsViewController

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];

    if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"denomination"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    } else if([[(TBKVOTableViewCell*)cell keyPath] isEqualToString:@"count_available"]) {
        NSNumberFormatter* numberFormatter = [[NSNumberFormatter alloc] init];
        [(TBFormattedKVOTableViewCell*)cell setFormatter:numberFormatter];
    }

    return [self setObjectToCell:cell];
}

@end
