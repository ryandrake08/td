//
//  TBSetupDetailsTableViewController.h
//  td
//
//  Created by Ryan Drake on 9/24/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupDetailsTableViewController.h"

@implementation TBSetupDetailsTableViewController

- (void)viewWillAppear:(BOOL)animated {
    NSIndexPath* selectedRowIndexPath = [[self tableView] indexPathForSelectedRow];
    if(selectedRowIndexPath) {
        [[self tableView] reloadRowsAtIndexPaths:@[selectedRowIndexPath] withRowAnimation:UITableViewRowAnimationNone];
    }
    [super viewWillAppear:animated];
}

@end
