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
    NSNumber* amount = @0;
    NSString* currency = @"USD";

    return [@{@"amount":amount, @"currency":currency} mutableCopy];
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
        NSDictionary* object = [[[self arrayController] arrangedObjects] objectAtIndex:rowIndex];
        [[[result textField] formatter] setCurrencyCode:object[@"currency"]];
    }
    return result;
}

@end
