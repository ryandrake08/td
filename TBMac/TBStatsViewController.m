//
//  TBStatsViewController.m
//  td
//
//  Created by Ryan Drake on 8/27/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBStatsViewController.h"

@interface TBStatsTableCellView : NSTableCellView

@property (weak) IBOutlet NSTextField* titleField;

@end

@implementation TBStatsTableCellView

@end

@implementation TBStatsViewController

#pragma mark NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    return 5;
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView*)aTableView viewForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    NSArray* titles = @[@"Current Round", @"Players Left", @"Entries", @"Average Stack", @"Elapsed Time"];
    NSArray* keyPaths = @[@"current_round_number_text", @"players_left_text", @"entries_text", @"average_stack_text", @"elapsed_time_text"];

    // TODO: Fix assertions here when [[self session] state] is empty
    
    TBStatsTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    if(rowIndex >= 0 && rowIndex < 5) {
        [[result titleField] setStringValue:NSLocalizedString(titles[rowIndex], nil)];
        [[result textField] bind:@"stringValue" toObject:[[self session] state] withKeyPath:keyPaths[rowIndex] options:nil];
    }
    return result;
}

@end
