//
//  TBSetupPayoutsViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSetupPayoutsViewController.h"
#import "TBCurrencyNumberFormatter.h"
#import "TournamentSession.h"
#import <Foundation/Foundation.h>

// TBSetupPayoutsArrayController implements a new object
@interface TBSetupPayoutsArrayController : NSArrayController

@end

@implementation TBSetupPayoutsArrayController

- (id)newObject {
    return @0L;
}

@end


@interface TBSetupPayoutsViewController () <NSTableViewDataSource, NSTableViewDelegate>

@end

@implementation TBSetupPayoutsViewController

#pragma mark NSTableViewDataSource

- (id)tableView:(NSTableView*)aTableView objectValueForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    if([[aTableColumn identifier] isEqualToString:@"Round"]) {
        return @(rowIndex+1);
    }
    return nil;
}

#pragma mark NSTableViewDelegate

- (NSView *)tableView:(NSTableView*)aTableView viewForTableColumn:(NSTableColumn*)aTableColumn row:(NSInteger)rowIndex {
    NSTableCellView* result = [aTableView makeViewWithIdentifier:aTableColumn.identifier owner:self];
    if([[aTableColumn identifier] isEqualToString:@"Payout"]) {
        if([[result textField] formatter] == nil) {
            [[result textField] setFormatter:[[TBCurrencyNumberFormatter alloc] init]];
        }
//        NSNumber* object = [[[self arrayController] arrangedObjects] objectAtIndex:rowIndex];
//        [[[result textField] formatter] setCurrencyCode:object[@"payout_currency"]];
    }
    return result;
}

@end
