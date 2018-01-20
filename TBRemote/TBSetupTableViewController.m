//
//  TBSetupTableViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupTableViewController.h"
#import "TBNotifications.h"
#import "TBSetupDetailsTableViewController.h"

@implementation TBSetupTableViewController

// common tableView handling code, to reduce boilerplate in subclasses

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return [[self arrangedObjects] count];
}

- (UITableViewCellEditingStyle)tableView:(UITableView*)tableView editingStyleForRowAtIndexPath:(NSIndexPath*)indexPath {
    return UITableViewCellEditingStyleDelete;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete from the data source
        [[self arrangedObjects] removeObjectAtIndex:[indexPath row]];

        // notify that configuration changed (delete)
        [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];

        // Remove from the table
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

// returns the object for a given indexPath
- (id)arrangedObjectForIndexPath:(NSIndexPath*)indexPath {
    return [self arrangedObjects][[indexPath row]];
}

// generates a new object (MUST be overridden by subclasses)
- (id)newObject {
    return nil;
}

#pragma mark Navigation

- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    TBSetupDetailsTableViewController* newController = [segue destinationViewController];
    NSIndexPath* indexPath = [[self tableView] indexPathForSelectedRow];
    [newController setObject:[self arrangedObjectForIndexPath:indexPath]];
    [newController setConfiguration:[self configuration]];
}

#pragma mark Actions

- (IBAction)addItem:(id)sender {
    NSIndexPath* newIndexPath = [NSIndexPath indexPathForRow:[[self arrangedObjects] count] inSection:0];

    // Add to the data source
    [[self arrangedObjects] addObject:[self newObject]];

    // notify that configuration changed (add)
    [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];

    // Insert to the table and scroll
    [[self tableView] insertRowsAtIndexPaths:@[newIndexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    [[self tableView] scrollToRowAtIndexPath:newIndexPath atScrollPosition:UITableViewScrollPositionBottom animated:YES];
}

@end
