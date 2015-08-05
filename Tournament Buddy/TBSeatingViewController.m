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
#import "NSObject+FBKVOController.h"

@interface TBSeatingViewController () <NSTableViewDelegate, NSMenuDelegate>

// Configuration window
@property (strong) TBConfigurationWindowController* configurationWindowController;
@property (strong) TBPlayerWindowController* playerWindowController;

// Seats dictionary controller
@property (strong) IBOutlet NSArrayController* seatsController;

// Keep track of last seating plan size, to avoid setting again
@property NSInteger lastMaxPlayers;

@end

@implementation TBSeatingViewController

- (void)viewDidLoad {
    if([NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [super viewDidLoad];
    }

    // filter predicate to not show empty seats
    [[self seatsController] setFilterPredicate:[NSPredicate predicateWithFormat: @"seat_number != nil"]];

    // setup configuration window
    [self setConfigurationWindowController:[[TBConfigurationWindowController alloc] initWithWindowNibName:@"TBConfigurationWindow"]];
    [[self configurationWindowController] setSession:[self session]];
    [[self configurationWindowController] setConfiguration:[self configuration]];
    [[[self configurationWindowController] window] close];

    // setup player window
    [self setPlayerWindowController:[[TBPlayerWindowController alloc] initWithWindowNibName:@"TBPlayerWindow"]];
    [[self playerWindowController] setSession:[self session]];
    [[[self playerWindowController] window] close];
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

#pragma mark NSMenuDelegate

- (void)fundPlayerFromMenuItem:(NSMenuItem*)sender {
    NSDictionary* context = [sender representedObject];
    [[self session] fundPlayer:context[@"player_id"] withFunding:context[@"funding_id"]];
}

- (void)menuNeedsUpdate:(NSMenu*)menu {
    // delete all previous objects from menu
    [menu removeAllItems];

    // find the clicked tableView cell
    NSTableView* tableView = [self tableView];
    NSInteger row = [tableView clickedRow];
    NSInteger col = [tableView clickedColumn];
    NSTableCellView* cell = [tableView viewAtColumn:col row:row makeIfNecessary:YES];

    // its object is the model object for this player
    id seatedPlayer = [cell objectValue];

    // add funding sources
    [[[self session] fundingSources] enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL* stop) {
        id context = @{@"player_id":seatedPlayer[@"player_id"], @"funding_id":@(idx)};

        // create a menu item for this funding option
        NSMenuItem* item = [[NSMenuItem alloc] initWithTitle:obj[@"name"] action:@selector(fundPlayerFromMenuItem:) keyEquivalent:@""];
        [item setTarget:self];
        [item setRepresentedObject:context];
        [menu addItem:item];
    }];
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

- (IBAction)seatedButtonDidChange:(id)sender {
    NSTableCellView* cell = (NSTableCellView*)[sender superview];
    id ov = [cell objectValue];
    id playerId = ov[@"player"][@"player_id"];
    if([sender state] == NSOnState) {
        [[self session] seatPlayer:playerId withBlock:nil];
    } else {
        [[self session] unseatPlayer:playerId withBlock:nil];
    }
}

- (IBAction)previousRoundTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGameAt:nil];
    }
}

- (IBAction)nextRoundTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        NSUInteger remaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
        if(remaining == 0) {
            [[self session] setActionClock:@kActionClockRequestTime];
        } else {
            [[self session] setActionClock:nil];
        }
    }
}

@end
