//
//  TBTableViewController.m
//  td
//
//  Created by Ryan Drake on 10/10/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"

@implementation TBTableViewController

// Reload the selected row before view appears
- (void)viewWillAppear:(BOOL)animated {
    NSIndexPath* selectedRowIndexPath = [[self tableView] indexPathForSelectedRow];
    if(selectedRowIndexPath) {
        [[self tableView] reloadRowsAtIndexPaths:@[selectedRowIndexPath] withRowAnimation:UITableViewRowAnimationNone];
    }
    [super viewWillAppear:animated];
}


@end
