//
//  TBRoundsViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRoundsViewController.h"
#import "TBRoundsTableCellView.h"

@implementation TBRoundsViewController

#pragma mark NSTableView

- (NSView*)tableView:(NSTableView*)tableView viewForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row {
    // Retrieve to get the @"MyView" from the pool or,
    // if no version is available in the pool, load the Interface Builder version
    TBRoundsTableCellView* result = [tableView makeViewWithIdentifier:[tableColumn identifier] owner:self];

    // Return the result
    return result;
}

@end
