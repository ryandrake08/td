//
//  TBSeatingViewController.m
//  td
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSeatingViewController.h"
#import "NSObject+FBKVOController.h"
#import "TBNotifications.h"
#import "TournamentSession.h"

@interface TBSeatingViewController () <NSMenuDelegate>

// global session
@property (strong) TournamentSession* session;

@end

@implementation TBSeatingViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // filter predicate to not show empty seats
    [[self arrayController] setFilterPredicate:[NSPredicate predicateWithFormat: @"seat_number != nil"]];

    // setup sort descriptors
    NSSortDescriptor* tableNumberSort = [[NSSortDescriptor alloc] initWithKey:@"table_number" ascending:YES];
    NSSortDescriptor* seatNumberSort = [[NSSortDescriptor alloc] initWithKey:@"seat_number" ascending:YES];

    // set sort descriptors for arrays
    [[self arrayController] setSortDescriptors:@[tableNumberSort, seatNumberSort]];

    // resize columns
    [[self tableView] sizeToFit];
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // set session
    [self setSession:representedObject];
}

#pragma mark TBPlayersViewDelegate

- (void)selectSeatForPlayerId:(NSString*)playerId {
    [[[self arrayController] arrangedObjects] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL* stop) {
        if([obj[@"player_id"] isEqualToString:playerId]) {
            [[self arrayController] setSelectionIndex:idx];
            [[self tableView] scrollRowToVisible:idx];
            *stop = YES;
        }
    }];
}

#pragma mark Menu and MenuItem utility

- (IBAction)fundPlayerFromMenuItem:(NSMenuItem*)sender {
    NSDictionary* context = [sender representedObject];
    NSArray* playerIds = context[@"player_ids"];
    NSNumber* fundingId = context[@"funding_id"];
    for(id playerId in playerIds) {
        NSLog(@"Funding player id: %@", playerId);
        [[self session] fundPlayer:playerId withFunding:fundingId];
    }
}

- (IBAction)bustPlayerFromMenuItem:(NSMenuItem*)sender {
    NSDictionary* context = [sender representedObject];
    NSArray* playerIds = context[@"player_ids"];
    for(id playerId in playerIds) {
        NSLog(@"Busting player id: %@", playerId);
        [[self session] bustPlayer:playerId withBlock:^(NSArray* movements) {
            if([movements count] > 0) {
                [[NSNotificationCenter defaultCenter] postNotificationName:kMovementsUpdatedNotification object:movements];
            }
        }];
    }
}

- (IBAction)unseatPlayerFromMenuItem:(NSMenuItem*)sender {
    NSDictionary* context = [sender representedObject];
    NSArray* playerIds = context[@"player_ids"];
    for(id playerId in playerIds) {
        NSLog(@"Unseating player id: %@", playerId);
        [[self session] unseatPlayer:playerId withBlock:nil];
    }
}

#pragma mark NSMenuDelegate

- (void)menuNeedsUpdate:(NSMenu*)menu {
    // find the selected indexes
    NSIndexSet* indexSet = [[self tableView] selectedRowIndexes];
    // If the clicked row is in selectedIndexes, then process all selectedIndexes. Otherwise, process only clickedRow.
    if ([[self tableView] clickedRow] != -1 && ![indexSet containsIndex:[[self tableView] clickedRow]]) {
        indexSet = [NSIndexSet indexSetWithIndex:[[self tableView] clickedRow]];
    }

    // array to hold all players
    NSMutableArray* playerIds = [[NSMutableArray alloc] init];
    __block NSUInteger buyins = 0;

    // build array of affected player_ids and count how many are bought in
    [indexSet enumerateIndexesUsingBlock:^(NSUInteger row, BOOL* stop) {
        NSTableCellView* cell = [[self tableView] viewAtColumn:0 row:row makeIfNecessary:YES];
        NSDictionary* player = [cell objectValue];
        [playerIds addObject:player[@"player_id"]];
        if([player[@"buyin"] boolValue]) {
            buyins++;
        }
    }];

    // current blind level
    NSNumber* currentBlindLevel = [[self session] state][@"current_blind_level"];

    // get list of players who have already bought in
    NSArray* uniqueEntries = [[self session] state][@"unique_entries"];

    // remove all funding sources
    for(id item = [menu itemWithTag:1]; item != nil; item = [menu itemWithTag:1]) {
        [menu removeItem:item];
    }

    // only add funding sources if one item selected, logic for which funding sources are allowed depends on player
    if([indexSet count] == 1) {
        // get player for this row
        NSUInteger row = [indexSet firstIndex];
        NSTableCellView* cell = [[self tableView] viewAtColumn:0 row:row makeIfNecessary:YES];
        NSDictionary* player = [cell objectValue];

        // BUSINESS LOGIC AROUND WHICH FUNDING SOURCES ARE ALLOWED WHEN

        // add buttons for each eligible funding source
        [[[self session] state][@"funding_sources"] enumerateObjectsUsingBlock:^(id source, NSUInteger idx, BOOL* stop) {
            id context = @{@"player_ids":playerIds, @"funding_id":@(idx)};
            NSNumber* last = source[@"forbid_after_blind_level"];
            if(last == nil || !([last compare:currentBlindLevel] == NSOrderedAscending)) {
                // create a menu item for this funding option
                NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:source[@"name"] action:@selector(fundPlayerFromMenuItem:) keyEquivalent:@""];
                [item setRepresentedObject:context];
                [item setTarget:self];
                [item setTag:1];

                if([source[@"type"] isEqual:kFundingTypeBuyin]) {
                    // buyins can happen at any time before forbid_after_blind_level, for any non-playing player
                    if(![player[@"buyin"] boolValue]) {
                        [menu insertItem:item atIndex:idx];
                    }
                } else if([source[@"type"] isEqual:kFundingTypeRebuy]) {
                    // rebuys can happen after round 0, before forbid_after_blind_level, for any player that has bought in at least once
                    if([currentBlindLevel unsignedIntegerValue] > 0 && [uniqueEntries containsObject:player[@"player_id"]]) {
                        [menu insertItem:item atIndex:idx];
                    }
                } else {
                    // addons can happen at any time before forbid_after_blind_level, for any playing player
                    if([player[@"buyin"] boolValue]) {
                        [menu insertItem:item atIndex:idx];
                    }
                }
            }
        }];
    }

    NSMenuItem* item = nil;

    // set up bust function. if game is running and all selected players are bought in, then enable the bust item
    item = [menu itemWithTag:2];
    [item setRepresentedObject:@{@"player_ids":playerIds}];
    [item setEnabled:([currentBlindLevel integerValue] > 0) && buyins == [indexSet count]];

    // add unseat function. if no selected players are bought in, then enable the unseat item
    item = [menu itemWithTag:3];
    [item setRepresentedObject:@{@"player_ids":playerIds}];
    [item setEnabled:buyins == 0];
}

#pragma mark Actions

- (IBAction)manageButtonClicked:(id)sender {
    NSTableCellView* cell = (NSTableCellView*)[sender superview];
    NSTableView* tableView = (NSTableView*)[[cell superview] superview];

    // select cell
    NSInteger row = [tableView rowForView:cell];
    NSIndexSet* indexSet = [NSIndexSet indexSetWithIndex:row];
    [tableView selectRowIndexes:indexSet byExtendingSelection:NO];

    // update menu content
    NSMenu* menu = [tableView menu];
    [NSMenu popUpContextMenu:menu withEvent:[NSApp currentEvent] forView:sender];
}

@end
