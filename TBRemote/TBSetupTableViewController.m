//
//  TBSetupTableViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupTableViewController.h"

@implementation TBSetupTableViewController

// common tableView handling code, to reduce boilerplate in subclasses

- (NSInteger)numberOfSectionsInTableView:(UITableView*) tableView {
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

        // Remove from the table
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

// returns the object for a given indexPath
- (id)arrangedObjectForIndexPath:(NSIndexPath*)indexPath {
    return [self arrangedObjects][[indexPath row]];
}

// adds a new object to arrangedObjects
- (void)addArrangedObject:(id)anObject {
    NSIndexPath* newIndexPath = [NSIndexPath indexPathForRow:[[self arrangedObjects] count] inSection:0];

    // Add to the data source
    [[self arrangedObjects] addObject:anObject];

    // Insert to the table
    [[self tableView] insertRowsAtIndexPaths:@[newIndexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
}

@end
