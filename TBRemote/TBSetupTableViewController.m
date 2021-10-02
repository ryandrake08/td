//
//  TBSetupTableViewController.m
//  td
//
//  Created by Ryan Drake on 9/23/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBSetupTableViewController.h"
#import "TBSetupDetailsTableViewController.h"
#import "TBNotifications.h"

@implementation TBSetupTableViewController

// common tableView handling code, to reduce boilerplate in subclasses

- (NSInteger)numberOfSectionsInTableView:(UITableView*)tableView {
    return 1;
}

- (NSInteger)tableView:(UITableView*)tableView numberOfRowsInSection:(NSInteger)section {
    return (NSInteger)[[self arrangedObjects] count];
}

- (UITableViewCellEditingStyle)tableView:(UITableView*)tableView editingStyleForRowAtIndexPath:(NSIndexPath*)indexPath {
    return UITableViewCellEditingStyleDelete;
}

- (void)tableView:(UITableView *)tableView commitEditingStyle:(UITableViewCellEditingStyle)editingStyle forRowAtIndexPath:(NSIndexPath *)indexPath {
    if (editingStyle == UITableViewCellEditingStyleDelete) {
        // Delete from the data source
        NSUInteger urow = (NSUInteger)[indexPath row];
        [[self arrangedObjects] removeObjectAtIndex:urow];

        // notify that configuration changed (delete)
        [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];

        // Remove from the table
        [tableView deleteRowsAtIndexPaths:@[indexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    }
}

// sets the arranged objects given a keypath or create an empty one
- (void)setArrangedObjectsKeyPath:(NSString*)keyPath {
    if([self arrangedObjects] == nil) {
        NSMutableArray* objects = [self valueForKeyPath:keyPath];
        if(objects == nil) {
            NSLog(@"Warning: keyPath %@ has no data, not even an empty array.", keyPath);
            objects = [[NSMutableArray alloc] init];
            [self setValue:objects forKeyPath:keyPath];
        }

        // is this needed?
        [self setArrangedObjects:objects];
    }
}

// returns the object for a given indexPath
- (id)arrangedObjectForIndexPath:(NSIndexPath*)indexPath {
    NSUInteger urow = (NSUInteger)[indexPath row];
    return [self arrangedObjects][urow];
}

// generates a new object (MUST be overridden by subclasses)
- (id)newObject {
    return nil;
}

#pragma mark Navigation

- (void)prepareForSegue:(UIStoryboardSegue*)segue sender:(id)sender {
    UIViewController* destinationController = [segue destinationViewController];

    // if we can set a configuration, set it
    if([destinationController respondsToSelector:@selector(setConfiguration:)]) {
        [destinationController performSelector:@selector(setConfiguration:) withObject:[self configuration]];
    }

    // set object to whichever one is selected
    if([destinationController respondsToSelector:@selector(setObject:)]) {
        NSIndexPath* selectedIndexPath = [[self tableView] indexPathForSelectedRow];
        id selectedObject = [self arrangedObjectForIndexPath:selectedIndexPath];
        [destinationController performSelector:@selector(setObject:) withObject:selectedObject];
    }
}

#pragma mark Actions

- (IBAction)addItem:(id)sender {
    // Add to the data source
    [[self arrangedObjects] addObject:[self newObject]];

    // notify that configuration changed (add)
    [[NSNotificationCenter defaultCenter] postNotificationName:kConfigurationUpdatedNotification object:nil];

    // figure out last added indexPath
    NSIndexPath* newIndexPath = [NSIndexPath indexPathForRow:(NSInteger)[[self arrangedObjects] count]-1 inSection:0];

    // Insert to the table and scroll
    [[self tableView] insertRowsAtIndexPaths:@[newIndexPath] withRowAnimation:UITableViewRowAnimationAutomatic];
    [[self tableView] scrollToRowAtIndexPath:newIndexPath atScrollPosition:UITableViewScrollPositionBottom animated:YES];
}

@end
