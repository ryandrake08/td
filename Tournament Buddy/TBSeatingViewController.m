//
//  TBSeatingViewController.m
//  Tournament Buddy
//
//  Created by Ryan Drake on 1/29/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBSeatingViewController.h"
#import "NSObject+FBKVOController.h"

// TBPlayersAvailableTableCellView to handle custom bindings
@interface TBPlayersAvailableTableCellView : NSTableCellView

@end

@implementation TBPlayersAvailableTableCellView

- (IBAction)seatedButtonDidChange:(id)sender {
    NSNumber* playerId = [self objectValue][@"player_id"];
    if([sender state] == NSOnState) {
        // TODO: get session and seat player
        //[[self session] seatPlayer:playerId withBlock:nil];
    } else {
        // TODO: get session and unseat player
        //[[self session] unseatPlayer:playerId];
    }
}

@end

@interface TBSeatingViewController ()

@property (strong) IBOutlet NSArrayController* playersArrayController;
@property (strong) IBOutlet NSDictionaryController* seatsDictionaryController;
@property (strong) IBOutlet NSArrayController* finishedArrayController;

// Derived game state
@property (strong) NSDictionary* seats;
@property (strong) NSArray* players;

@end

@implementation TBSeatingViewController

- (void)updatePlayers {
    // inject seated flag into our own player array
    NSMutableArray* newArray = [NSMutableArray array];
    for(id obj in [[self session] players]) {
        NSMutableDictionary* newObj = [NSMutableDictionary dictionaryWithDictionary:obj];
        NSNumber* playerId = obj[@"player_id"];
        id seat = [self seats][playerId];
        newObj[@"seated"] = [NSNumber numberWithBool:playerId != nil && seat != nil];
        [newArray addObject:newObj];
    }
    [self setPlayers:newArray];
}

- (void)updateSeats {
    // inject player names into our own seat assignment dictionary
    NSMutableDictionary* newDict = [NSMutableDictionary dictionary];
    for(id obj in [[self session] seats]) {
        NSNumber* playerId = obj[@"player_id"];
        if(playerId) {
            NSMutableDictionary* newObj = [NSMutableDictionary dictionaryWithDictionary:obj];
            newObj[@"player_name"] = [[self session] playersLookup][playerId];
            newDict[playerId] = newObj;
        }
    }
    [self setSeats:newDict];

    // also need to update seatef flag for players
    [self updatePlayers];
}

- (void)viewDidLoad {
    // register for KVO
    [[self KVOController] observe:[self session] keyPath:@"seats" options:NSKeyValueObservingOptionInitial action:@selector(updateSeats)];
    [[self KVOController] observe:[self session] keyPath:@"players" options:NSKeyValueObservingOptionInitial action:@selector(updatePlayers)];
}

#pragma mark Actions

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
