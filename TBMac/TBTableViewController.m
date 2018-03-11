//
//  TBTableViewController.m
//  td
//
//  Created by Ryan Drake on 1/31/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBTableViewController.h"

@implementation TBTableViewController

#pragma mark NSTableViewDelegate

- (void)tableViewSelectionDidChange:(NSNotification*)notification {
    // all TBTableViewControllers scroll their tableViews when the selection changes
    [[self tableView] scrollRowToVisible:[[self tableView] selectedRow]];
}

@end
