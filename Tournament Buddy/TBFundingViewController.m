//
//  TBFundingViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBFundingViewController.h"
#import "TBFundingTableCellView.h"

@implementation TBFundingViewController

#pragma mark NSTableView

- (NSView*)tableView:(NSTableView*)tableView viewForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row {
    // Retrieve to get the @"MyView" from the pool or,
    // if no version is available in the pool, load the Interface Builder version
    TBFundingTableCellView* result = [tableView makeViewWithIdentifier:[tableColumn identifier] owner:self];

    // Set up the cell's members
    id blindLevels = [[self configuration] objectForKey:@"blind_levels"];
    [result setBlindLevels:[blindLevels count]];

    // Return the result
    return result;
}

@end
