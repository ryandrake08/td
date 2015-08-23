//
//  TBSeatingViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSeatingViewController.h"
#import "TBMovementWindowController.h"
#import "TBResultsViewController.h"
#import "TBPlayersViewController.h"
#import "TBSound.h"
#import "NSObject+FBKVOController.h"

@interface TBSeatingViewController () <NSTableViewDelegate, NSMenuDelegate, TBPlayersViewDelegate>

// Configuration window
@property (strong) TBMovementWindowController* movementWindowController;

// View controllers
@property (strong) IBOutlet TBPlayersViewController* playersViewController;
@property (strong) IBOutlet TBResultsViewController* resultsViewController;

// Image to use for buyin icon
@property (strong) NSImage* currencyImage;

// UI elements
@property (weak) IBOutlet NSView* leftPaneView;
@property (weak) IBOutlet NSView* rightPaneView;

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

    // alloc sounds
    [self setRebalanceSound:[[TBSound alloc] initWithResource:@"s_rebalance" extension:@"caf"]];

    // register for KVO
    [[self KVOController] observe:[self session] keyPath:@"costCurrency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        NSDictionary* currencyImages = @{@"EUR":@"b_note_euro",@"INR":@"b_note_rupee",@"EGP":@"b_note_sterling",@"FKP":@"b_note_sterling",@"GIP":@"b_note_sterling",@"GGP":@"b_note_sterling",@"IMP":@"b_note_sterling",@"JEP":@"b_note_sterling",@"LBP":@"b_note_sterling",@"SHP":@"b_note_sterling",@"SYP":@"b_note_sterling",@"GBP":@"b_note_sterling",@"JPY":@"b_note_yen_yuan",@"CNY":@"b_note_yen_yuan"};
        NSString* imageName = currencyImages[[object costCurrency]];
        if(imageName == nil) {
            imageName = @"b_note_dollar";
        }
        [self setCurrencyImage:[NSImage imageNamed:imageName]];
    }];

    // set self as player view delegate
    [[self playersViewController] setDelegate:self];

    // add subivews
    [[self playersViewController] setSession:[self session]];
    [[self leftPaneView] addSubview:[[self playersViewController] view]];

    [[self resultsViewController] setSession:[self session]];
    [[self rightPaneView] addSubview:[[self resultsViewController] view]];
}

- (void)viewWillDisappear {
    [[self movementWindowController] close];
}

- (void)selectSeatForPlayerId:(NSString*)playerId {
    [[[self arrayController] arrangedObjects] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if([obj[@"player_id"] isEqualToString:playerId]) {
            [[self arrayController] setSelectionIndex:idx];
            [[self tableView] scrollRowToVisible:idx];
            *stop = YES;
        }
    }];
}

#pragma mark Menu and MenuItem utility

- (void)fundPlayerFromMenuItem:(NSMenuItem*)sender {
    NSDictionary* context = [sender representedObject];
    [[self session] fundPlayer:context[@"player_id"] withFunding:context[@"funding_id"]];
}

- (void)bustPlayerFromMenuItem:(NSMenuItem*)sender {
    NSNumber* playerId = [sender representedObject];
    [[self session] bustPlayer:playerId withBlock:^(NSArray* movements) {
        if([movements count] > 0) {
            // setup movement window if needed
            if([self movementWindowController] == nil) {
                [self setMovementWindowController:[[TBMovementWindowController alloc] initWithWindowNibName:@"TBMovementWindow"]];
            }

            for(NSDictionary* movement in movements) {
                // inject player
                NSMutableDictionary* newMovement = [[NSMutableDictionary alloc] initWithDictionary:movement];
                NSString* playerId = movement[@"player_id"];
                NSDictionary* player = [[self session] playersLookup][playerId];
                newMovement[@"player"] = player;
                [[[self movementWindowController] arrayController] addObject:newMovement];
            }

            // alert sound
            [[self rebalanceSound] play];

            // show window
            [[self movementWindowController] showWindow:self];
        }
    }];
}

- (void)unseatPlayerFromMenuItem:(NSMenuItem*)sender {
    NSNumber* playerId = [sender representedObject];
    [[self session] unseatPlayer:playerId withBlock:nil];
}

- (void)updateActionMenu:(NSMenu*)menu forTableCellView:(NSTableCellView*)cell {
    // use manual enabling
    [menu setAutoenablesItems:NO];

    // its object is the model object for this player
    id seatedPlayer = [cell objectValue];

    // current blind level
    NSNumber* current = [[self session] currentBlindLevel];

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
        [item setEnabled:(last == nil || !([last compare:current] == NSOrderedAscending))];
        [menu addItem:item];
    }];

    NSMenuItem* item = [NSMenuItem separatorItem];
    [menu addItem:item];

    // add bust function
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Bust Player", @"Bust the player out of the tournamnet") action:@selector(bustPlayerFromMenuItem:) keyEquivalent:@"b"];
    [item setRepresentedObject:seatedPlayer[@"player_id"]];
    [item setTarget:self];
    [item setEnabled:([current integerValue] > 0) && ([seatedPlayer[@"buyin"] boolValue])];
    [menu addItem:item];

    // add unseat function
    item = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"Unseat Player", @"Remove player without impacting results") action:@selector(unseatPlayerFromMenuItem:) keyEquivalent:@"u"];
    [item setTarget:self];
    [item setRepresentedObject:seatedPlayer[@"player_id"]];
    [item setEnabled:(![seatedPlayer[@"buyin"] boolValue])];
    [menu addItem:item];
}

#pragma mark NSMenuDelegate

- (void)menuNeedsUpdate:(NSMenu*)menu {
    // delete all previous objects from menu
    [menu removeAllItems];

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
    NSMenu* menu = [[NSMenu alloc] initWithTitle:NSLocalizedString(@"Manage", @"Manage user")];

    // delete all previous objects from menu
    [menu removeAllItems];

    // update menu content
    [self updateActionMenu:menu forTableCellView:cell];

    // show menu
    [NSMenu popUpContextMenu:menu withEvent:[NSApp currentEvent] forView:sender];
}

@end
