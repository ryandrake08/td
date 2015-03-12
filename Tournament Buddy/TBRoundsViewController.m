//
//  TBRoundsViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRoundsViewController.h"
#import "TBRoundsTableCellView.h"

@interface RoundsArrayController : NSArrayController

@end

@implementation RoundsArrayController

- (id)arrangedObjects {
    NSArray* a = [super arrangedObjects];
    if ([a count]) {
        return [a subarrayWithRange:NSMakeRange(1, [a count]-1)];
    } else {
        return a;
    }
}

@end

@implementation TBRoundsViewController

#pragma mark NSTableView

- (NSView*)tableView:(NSTableView*)tableView viewForTableColumn:(NSTableColumn*)tableColumn row:(NSInteger)row {
    // Retrieve to get the @"MyView" from the pool or,
    // if no version is available in the pool, load the Interface Builder version
    TBRoundsTableCellView* result = [tableView makeViewWithIdentifier:[tableColumn identifier] owner:self];

    // Set up the cell's members
    [result setRoundNumber:row+1];

    // Return the result
    return result;
}

@end
