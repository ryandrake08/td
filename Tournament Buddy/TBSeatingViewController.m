//
//  TBSeatingViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSeatingViewController.h"
#import "NSObject+FBKVOController.h"

@interface TBSeatingViewController () <NSTableViewDelegate, NSTextFieldDelegate>

@property (strong) IBOutlet NSArrayController* playersArrayController;
@property (strong) IBOutlet NSDictionaryController* seatsDictionaryController;
@property (strong) IBOutlet NSArrayController* finishedArrayController;

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
        NSNumber* playerId = obj[@"player_id"];
        if(playerId) {
            NSMutableDictionary* newObj = [NSMutableDictionary dictionaryWithDictionary:obj];
            BOOL seated = [[[self session] seatedPlayers] containsObject:playerId];
            newObj[@"seated"] = [NSNumber numberWithBool:seated];
            [newArray addObject:newObj];
        }
    }
    [self setPlayers:newArray];
}

- (void)updateSeats {
    // inject players into our own seats array
    NSMutableArray* newArray = [NSMutableArray array];
    for(id obj in [[self session] seats]) {
        NSNumber* playerId = obj[@"player_id"];
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
    // register for KVO
    [[self KVOController] observe:[self session] keyPath:@"seats" options:NSKeyValueObservingOptionInitial action:@selector(updateSeats)];
    [[self KVOController] observe:[self session] keyPath:@"players" options:NSKeyValueObservingOptionInitial action:@selector(updatePlayers)];
}

#pragma mark NSTextFieldDelegate

- (void)controlTextDidEndEditing:(NSNotification*)notification {
    NSInteger maxPlayers = [[notification object] integerValue];
    if(maxPlayers > 1 && maxPlayers != [self lastMaxPlayers]) {
        [[self session] planSeatingFor:[NSNumber numberWithInteger:maxPlayers]];
        [self setLastMaxPlayers:maxPlayers];
    }
}

#pragma mark Actions

- (IBAction)seatedButtonDidChange:(id)sender {
    NSTableCellView* cell = (NSTableCellView*)[sender superview];
    NSNumber* playerId = [cell objectValue][@"player_id"];
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
            [[self session] setActionClock:[NSNumber numberWithUnsignedInteger:kActionClockRequestTime]];
        } else {
            [[self session] setActionClock:nil];
        }
    }
}

@end
