//
//  TBSetupDetailsChipsViewController.m
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsChipsViewController.h"
#import "TBChooseColorViewController.h"
#import "TBKVOTableViewCell.h"

@implementation TBSetupDetailsChipsViewController

#pragma mark UITableViewDataSource

- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];
    [(TBKVOTableViewCell*)cell setObject:[self object]];
    return cell;
}

#pragma mark Navigation

- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    [super prepareForSegue:segue sender:sender];

    TBChooseColorViewController* newController = [segue destinationViewController];
    [newController setObject:[self object]];
}

@end
