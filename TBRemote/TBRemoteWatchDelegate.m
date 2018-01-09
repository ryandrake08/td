//
//  TBRemoteWatchDelegate.m
//  td
//
//  Created by Ryan Drake on 9/8/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteWatchDelegate.h"
#import "NSDictionary+Changed.h"
@import WatchConnectivity;

@interface TBRemoteWatchDelegate() <WCSessionDelegate>

// the tournament session (model) object
@property (nonatomic, strong) TournamentSession* session;

// the full state when an update was last sent to watch
@property (nonatomic, strong) NSMutableDictionary* stateWhenLastSent;

@end

@implementation TBRemoteWatchDelegate

- (void)sendUpdateToWatch:(NSDictionary*)update {
    // If we still have an update, send it
    if([update count] > 0) {
        // Send state change as a message to watch
        [[WCSession defaultSession] sendMessage:@{@"state":update} replyHandler:nil errorHandler:nil];

        // Add changes from last state
        [[self stateWhenLastSent] addEntriesFromDictionary:update];
    }
}

- (instancetype)initWithSession:(TournamentSession *)session {
    if((self = [super init])) {
        if([WCSession isSupported]) {
            // Set the tournament session
            [self setSession:session];

            // Default last state
            [self setStateWhenLastSent:[[NSMutableDictionary alloc] init]];

            // Set up the WatchConnectivity session
            WCSession* wcSession = [WCSession defaultSession];
            [wcSession setDelegate:self];
            [wcSession activateSession];

            // Notification center - Update WCSession when certain state changes happen
            [[NSNotificationCenter defaultCenter] addObserverForName:kTournamentSessionUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
                if([wcSession isReachable]) {
                    // Changes from last sent state
                    NSMutableDictionary* update = [[note object] mutableDictionaryWithChangesFromDictionary:[self stateWhenLastSent]];

                    // Filter out keys we don't need to send to watch
                    [update removeObjectsForKeys:@[@"elapsed_time", @"elapsed_time_text", @"time_remaining", @"break_time_remaining", @"action_clock_time_remaining"]];

                    if([update count] != 0) {
                        // Send update
                        [self sendUpdateToWatch:update];
                    }
                }
            }];
        }
    }
    return self;
}

- (void)handleCommand:(NSString*)command {
    if(command) {
        NSLog(@"Command received from watch: %@", command);

        if([command isEqualToString:@"fullSync"]) {
            // Checking for authorization will trigger auth message
            [[self session] checkAuthorizedWithBlock:nil];

            // Send update
            [self sendUpdateToWatch:[[self session] state]];
        }

        if([command isEqualToString:@"previousRound"]) {
            NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
            if(currentBlindLevel != 0) {
                [[self session] setPreviousLevelWithBlock:nil];
            }
        }

        if([command isEqualToString:@"playPause"]) {
            NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
            if(currentBlindLevel != 0) {
                [[self session] togglePauseGame];
            } else {
                [[self session] startGame];
            }
        }

        if([command isEqualToString:@"nextRound"]) {
            NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
            if(currentBlindLevel != 0) {
                [[self session] setNextLevelWithBlock:nil];
            }
        }

        if([command isEqualToString:@"callClock"]) {
            NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
            if(currentBlindLevel != 0) {
                NSUInteger remaining = [[[self session] state][@"action_clock_time_remaining"] unsignedIntegerValue];
                if(remaining == 0) {
                    [[self session] setActionClock:@kActionClockRequestTime];
                } else {
                    [[self session] clearActionClock];
                }
            }
        }
    }
}

#pragma mark WCSessionDelegate

- (void)session:(WCSession*)wcSession didReceiveMessage:(NSDictionary<NSString*,id>*)message {
    [self performSelectorOnMainThread:@selector(handleCommand:) withObject:message[@"command"] waitUntilDone:NO];
}

@end
