//
//  TBSetupDetailsTableViewController.h
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsTableViewController.h"

@implementation TBSetupDetailsTableViewController

- (UITableViewCell*)setObjectToCell:(UITableViewCell*)cell {
    if([cell respondsToSelector:@selector(setObject:)]) {
        [cell performSelector:@selector(setObject:) withObject:[self object]];
    }
    return cell;
}

#pragma mark UITableViewDataSource

// default just gets a cell and sets the object
- (UITableViewCell*)tableView:(UITableView*)tableView cellForRowAtIndexPath:(NSIndexPath*)indexPath {
    UITableViewCell* cell = [super tableView:tableView cellForRowAtIndexPath:indexPath];
    return [self setObjectToCell:cell];
}

#pragma mark Navigation

// default calls setObject on the viewController if supported
- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    UIViewController* destinationController = [segue destinationViewController];

    // set object to whichever one is selected
    if([destinationController respondsToSelector:@selector(setObject:)]) {
        [destinationController performSelector:@selector(setObject:) withObject:[self object]];
    }
}

@end
