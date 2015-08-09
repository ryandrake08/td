//
//  TBSeatingViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSeatingViewController.h"
#import "TBConfigurationWindowController.h"
#import "TBPlayerWindowController.h"
#import "TBControlsViewController.h"
#import "TBResultsViewController.h"
#import "NSObject+FBKVOController.h"

@interface TBSeatingViewController () <NSTableViewDelegate, NSMenuDelegate>

// Configuration window
@property (strong) TBConfigurationWindowController* configurationWindowController;
@property (strong) TBPlayerWindowController* playerWindowController;

// View controllers
@property (strong) IBOutlet TBResultsViewController* resultsViewController;
@property (strong) IBOutlet TBControlsViewController* controlsViewController;

// Array controllers
@property (strong) IBOutlet NSArrayController* playersController;
@property (strong) IBOutlet NSArrayController* seatsController;

// Keep track of last seating plan size, to avoid setting again
@property NSInteger lastMaxPlayers;

// Image to use for buyin icon
@property (strong) NSImage* currencyImage;

// UI elements
@property (weak) IBOutlet NSView* rightPaneView;
@property (weak) IBOutlet NSView* controlsView;
@property (weak) IBOutlet NSTableView* playersTableView;
@property (weak) IBOutlet NSTableView* seatsTableView;

@end

@implementation TBSeatingViewController

- (void)viewDidLoad {
    if([NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [super viewDidLoad];
    }

    // filter predicate to not show empty seats
    [[self seatsController] setFilterPredicate:[NSPredicate predicateWithFormat: @"seat_number != nil"]];

    // setup sort descriptors
    NSSortDescriptor* playerNameSort = [[NSSortDescriptor alloc] initWithKey:@"player.name" ascending:YES];
    NSSortDescriptor* tableNumberSort = [[NSSortDescriptor alloc] initWithKey:@"table_number" ascending:YES];
    NSSortDescriptor* seatNumberSort = [[NSSortDescriptor alloc] initWithKey:@"seat_number" ascending:YES];

    // set sort descriptors for arrays
    [[self playersController] setSortDescriptors:@[playerNameSort]];
    [[self seatsController] setSortDescriptors:@[tableNumberSort, seatNumberSort]];

    // setup configuration window
    [self setConfigurationWindowController:[[TBConfigurationWindowController alloc] initWithWindowNibName:@"TBConfigurationWindow"]];
    [[self configurationWindowController] setSession:[self session]];
    [[self configurationWindowController] setConfiguration:[self configuration]];
    [[[self configurationWindowController] window] close];

    // setup player window
    [self setPlayerWindowController:[[TBPlayerWindowController alloc] initWithWindowNibName:@"TBPlayerWindow"]];
    [[self playerWindowController] setSession:[self session]];
    [[[self playerWindowController] window] close];

    // register for KVO
    [[self KVOController] observe:[self session] keyPath:@"costCurrency" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        NSDictionary* currencyImages = @{@"EUR":@"b_note_euro",@"INR":@"b_note_rupee",@"EGP":@"b_note_sterling",@"FKP":@"b_note_sterling",@"GIP":@"b_note_sterling",@"GGP":@"b_note_sterling",@"IMP":@"b_note_sterling",@"JEP":@"b_note_sterling",@"LBP":@"b_note_sterling",@"SHP":@"b_note_sterling",@"SYP":@"b_note_sterling",@"GBP":@"b_note_sterling",@"JPY":@"b_note_yen_yuan",@"CNY":@"b_note_yen_yuan"};
        NSString* imageName = currencyImages[[object costCurrency]];
        if(imageName == nil) {
            imageName = @"b_note_dollar";
        }
        [self setCurrencyImage:[NSImage imageNamed:imageName]];
    }];

    // add subivew
    [[self resultsViewController] setSession:[self session]];
    [[self rightPaneView] addSubview:[[self resultsViewController] view]];
    [[self controlsViewController] setSession:[self session]];
    [[self controlsView] addSubview:[[self controlsViewController] view]];
}

- (void)loadView {
    [super loadView];
    if(![NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [self viewDidLoad];
    }
}

- (void)dealloc {
    [[self configurationWindowController] close];
    [[self playerWindowController] close];
}

- (void)selectSeatForPlayerId:(NSString*)playerId {
    [[[self seatsController] arrangedObjects] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
        if([obj[@"player_id"] isEqualToString:playerId]) {
            [[self seatsController] setSelectionIndex:idx];
            [[self seatsTableView] scrollRowToVisible:idx];
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
        NSLog(@"Player Movements:");
        for(NSDictionary* movement in movements) {
            NSString* playerId = movement[@"player_id"];
            NSDictionary* player = [[self session] playersLookup][playerId];
            NSLog(@"%@ moves from table %d, seat %d to table %d, seat %d",
                  player[@"name"],
                  [movement[@"from_table_number"] intValue]+1,
                  [movement[@"from_seat_number"] intValue]+1,
                  [movement[@"to_table_number"] intValue]+1,
                  [movement[@"to_seat_number"] intValue]+1);
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
    NSTableView* tableView = [self seatsTableView];
    NSInteger row = [tableView clickedRow];
    NSInteger col = [tableView clickedColumn];
    if(row != -1 && col != -1) {
        NSTableCellView* cell = [tableView viewAtColumn:col row:row makeIfNecessary:YES];

        // update menu content
        [self updateActionMenu:menu forTableCellView:cell];
    }
}

#pragma mark Actions

- (IBAction)configureButtonDidChange:(id)sender {
    if([[[self configurationWindowController] window] isVisible]) {
        [[self configurationWindowController] close];
    } else {
        [[self configurationWindowController] showWindow:self];
    }
}

- (IBAction)displayButtonDidChange:(id)sender {
    if([[[self playerWindowController] window] isVisible]) {
        [[self playerWindowController] close];
    } else {
        [[self playerWindowController] showWindow:self];

        // move to second screen if possible
        NSArray* screens = [NSScreen screens];
        if([screens count] > 1) {
            NSScreen* screen = [[NSScreen screens] objectAtIndex:1];
            [[[self playerWindowController] window] setFrame: [screen frame] display:YES animate:NO];
            [[[self playerWindowController] window] makeKeyAndOrderFront:screen];
        }
    }
}

- (IBAction)maxPlayersTextDidChange:(id)sender {
    NSInteger maxPlayers = [sender integerValue];
    if(maxPlayers > 1 && maxPlayers != [self lastMaxPlayers]) {
        [[self session] planSeatingFor:@(maxPlayers)];
        [self setLastMaxPlayers:maxPlayers];
    }
}

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

- (IBAction)seatedButtonDidChange:(id)sender {
    NSTableCellView* cell = (NSTableCellView*)[sender superview];
    id ov = [cell objectValue];
    id playerId = ov[@"player"][@"player_id"];
    if([sender state] == NSOnState) {
        [[self session] seatPlayer:playerId withBlock:nil];
        // select that seat
        [self performSelector:@selector(selectSeatForPlayerId:) withObject:playerId afterDelay:0.0];
    } else {
        [[self session] unseatPlayer:playerId withBlock:nil];
    }
}

- (IBAction)playerChangeSelected:(id)sender {
    NSInteger selectedRow = [sender selectedRow];
    if (selectedRow != -1) {
        // get selected object
        NSArray* selectedObjects = [[self playersController] selectedObjects];
        NSString* playerId = selectedObjects[0][@"player_id"];
        // select that seat
        [self selectSeatForPlayerId:playerId];
    }
}
@end
