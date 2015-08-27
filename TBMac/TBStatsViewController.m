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
    NSArray* keyPaths = @[@"currentRoundNumberText", @"playersLeftText", @"entriesText", @"averageStackText", @"elapsedTimeText"];

    TBStatsTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    if(rowIndex >= 0 && rowIndex < 5) {
        [[result titleField] setStringValue:NSLocalizedString(titles[rowIndex], nil)];
        [[result textField] bind:@"stringValue" toObject:[self session] withKeyPath:keyPaths[rowIndex] options:nil];
    }
    return result;
}

@end
