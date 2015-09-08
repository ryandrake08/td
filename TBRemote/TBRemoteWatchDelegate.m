//
//  TBRemoteWatchDelegate.m
//  td
//
//  Created by Ryan Drake on 9/8/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteWatchDelegate.h"

@import WatchConnectivity;

@interface TBRemoteWatchDelegate() <WCSessionDelegate>

// the tournament session (model) object
@property (strong) TournamentSession* session;

@end

@implementation TBRemoteWatchDelegate

- (instancetype)initWithSession:(TournamentSession *)session {
    if((self = [super init])) {
        if([WCSession isSupported]) {
            // Set the tournament session
            [self setSession:session];

            // Set up the WatchConnectivity session
            WCSession* wcSession = [WCSession defaultSession];
            [wcSession setDelegate:self];
            [wcSession activateSession];

            // Notification center - Update WCSession when certain state changes happen
            [[NSNotificationCenter defaultCenter] addObserverForName:TournamentSessionUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
                NSMutableDictionary* update = [NSMutableDictionary dictionaryWithDictionary:[note object]];
                [update removeObjectsForKeys:@[@"elapsed_time", @"elapsed_time_text", @"time_remaining", @"break_time_remaining", @"action_clock_time_remaining"]];
                if([update count] > 0) {
                    [[WCSession defaultSession] sendMessage:@{@"state":update} replyHandler:nil errorHandler:nil];
                }
            }];
        }
    }
    return self;
}

#pragma mark WCSessionDelegate

- (void)session:(WCSession*)wcSession didReceiveMessage:(NSDictionary<NSString*,id>*)message {
    NSString* command = message[@"command"];
    if(command) {
        NSLog(@"Command received from watch: %@", command);

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
                [[self session] startGameAt:nil];
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
                    [[self session] setActionClock:nil];
                }
            }
        }
    }
}

@end
