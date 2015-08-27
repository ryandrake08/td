//
//  TBSeatingViewController.m
//  td
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSeatingViewController.h"
#import "TBMovementWindowController.h"
#import "TBSound.h"
#import "NSObject+FBKVOController.h"

@interface TBSeatingViewController () <NSMenuDelegate>

// Configuration window
@property (strong) TBMovementWindowController* movementWindowController;

// Sounds
@property (strong) TBSound* rebalanceSound;

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

- (void)viewWillDisappear {
    [[self movementWindowController] close];
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
    [[self session] fundPlayer:context[@"player_id"] withFunding:context[@"funding_id"]];
}

- (IBAction)bustPlayerFromMenuItem:(NSMenuItem*)sender {
    NSNumber* playerId = [sender representedObject];
    [[self session] bustPlayer:playerId withBlock:^(NSArray* movements) {
        if([movements count] > 0) {
            // setup movement window if needed
            if([self movementWindowController] == nil) {
                [self setMovementWindowController:[[TBMovementWindowController alloc] initWithWindowNibName:@"TBMovementWindow"]];
            }
            
            // show window
            [[self movementWindowController] showWindow:self];

            for(NSDictionary* movement in movements) {
                // inject player
                NSMutableDictionary* newMovement = [[NSMutableDictionary alloc] initWithDictionary:movement];
                NSString* playerId = movement[@"player_id"];
                NSDictionary* player = [[self session] playersLookup][playerId];
                newMovement[@"player"] = player;
                [[[self movementWindowController] arrayController] addObject:newMovement];
            }

            // alert sound
            if([self rebalanceSound] == nil) {
                [self setRebalanceSound:[[TBSound alloc] initWithResource:@"s_rebalance" extension:@"caf"]];
            }
            [[self rebalanceSound] play];
        }
    }];
}

- (IBAction)unseatPlayerFromMenuItem:(NSMenuItem*)sender {
    NSNumber* playerId = [sender representedObject];
    [[self session] unseatPlayer:playerId withBlock:nil];
}

- (void)updateActionMenu:(NSMenu*)menu forTableCellView:(NSTableCellView*)cell {
    // its object is the model object for this player
    id seatedPlayer = [cell objectValue];

    // current blind level
    NSNumber* currentBlindLevel = [[self session] currentBlindLevel];

    // remove all funding sources
    for(id item = [menu itemWithTag:1]; item != nil; item = [menu itemWithTag:1]) {
        [menu removeItem:item];
    }

    // add funding sources
    [[[self session] fundingSources] enumerateObjectsUsingBlock:^(id source, NSUInteger idx, BOOL* stop) {
        id context = @{@"player_id":seatedPlayer[@"player_id"], @"funding_id":@(idx)};

        // enable if we can still use this source
        NSNumber* last = source[@"forbid_after_blind_level"];

        // shortcut key
        NSString* keyEquiv = @"";
        if(idx < 9) {
            keyEquiv = [@(idx+1) stringValue];
        }

        // create a menu item for this funding option
        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:source[@"name"] action:@selector(fundPlayerFromMenuItem:) keyEquivalent:keyEquiv];
        [item setRepresentedObject:context];
        [item setTarget:self];
        [item setEnabled:(last == nil || !([last compare:currentBlindLevel] == NSOrderedAscending))];
        [item setTag:1];
        [menu insertItem:item atIndex:idx];
    }];

    NSMenuItem* item = nil;

    // set up bust function
    item = [menu itemWithTag:2];
    [item setRepresentedObject:seatedPlayer[@"player_id"]];
    [item setEnabled:([currentBlindLevel integerValue] > 0) && ([seatedPlayer[@"buyin"] boolValue])];

    // add unseat function
    item = [menu itemWithTag:3];
    [item setRepresentedObject:seatedPlayer[@"player_id"]];
    [item setEnabled:(![seatedPlayer[@"buyin"] boolValue])];
}

#pragma mark NSMenuDelegate

- (void)menuNeedsUpdate:(NSMenu*)menu {
    // find the clicked tableView cell
    NSInteger row = [[self tableView] clickedRow];
    NSInteger col = [[self tableView] clickedColumn];
    if(row != -1 && col != -1) {
        NSTableCellView* cell = [[self tableView] viewAtColumn:col row:row makeIfNecessary:YES];

        // update menu content
        [self updateActionMenu:menu forTableCellView:cell];
    }
}

#pragma mark Actions

- (IBAction)manageButtonClicked:(id)sender {
    NSTableCellView* cell = (NSTableCellView*)[sender superview];
    NSTableView* tableView = (NSTableView*)[[cell superview] superview];
    NSMenu* menu = [tableView menu];

    // update menu content
    [self updateActionMenu:menu forTableCellView:cell];

    // show menu
    [NSMenu popUpContextMenu:menu withEvent:[NSApp currentEvent] forView:sender];
}

@end
