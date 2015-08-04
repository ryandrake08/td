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

@interface TBSeatingViewController () <NSTableViewDelegate>

// Configuration window
@property (strong) TBConfigurationWindowController* configurationWindowController;
@property (strong) TBPlayerWindowController* playerWindowController;

// Derived game state
@property (strong) NSArray* seats;
@property (strong) NSArray* players;

// Keep track of last seating plan size, to avoid setting again
@property NSInteger lastMaxPlayers;

@end

@implementation TBSeatingViewController

- (void)updatePlayers {
    // inject seated flag into our own player array
    NSMutableArray* newArray = [NSMutableArray array];
    for(id obj in [[self session] players]) {
        id playerId = obj[@"player_id"];
        if(playerId) {
            NSMutableDictionary* newObj = [NSMutableDictionary dictionaryWithDictionary:obj];
            BOOL seated = [[[self session] seatedPlayers] containsObject:playerId];
            newObj[@"seated"] = @(seated);
            [newArray addObject:newObj];
        }
    }
    [self setPlayers:newArray];
}

- (void)updateSeats {
    // inject players into our own seats array
    NSMutableArray* newArray = [NSMutableArray array];
    for(id obj in [[self session] seats]) {
        id playerId = obj[@"player_id"];
        if(playerId) {
            NSDictionary* player = [[self session] playersLookup][playerId];
            if(player) {
                NSMutableDictionary* newObj = [NSMutableDictionary dictionaryWithDictionary:obj];
                newObj[@"player"] = player;
                [newArray addObject:newObj];
            }
        }
    }
    [self setSeats:newArray];

    // also need to update seated flag for players
    [self updatePlayers];
}

- (void)viewDidLoad {
    if([NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [super viewDidLoad];
    }

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
    [[self KVOController] observe:[self session] keyPath:@"seats" options:NSKeyValueObservingOptionInitial action:@selector(updateSeats)];
    [[self KVOController] observe:[self session] keyPath:@"players" options:NSKeyValueObservingOptionInitial action:@selector(updatePlayers)];
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
    id playerId = [cell objectValue][@"player_id"];
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
