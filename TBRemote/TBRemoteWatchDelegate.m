//
//  TBRemoteWatchDelegate.m
//  td
//
//  Created by Ryan Drake on 9/8/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteWatchDelegate.h"
@import WatchConnectivity;

#define kMinMessageSendInterval 1.0

@interface TBRemoteWatchDelegate() <WCSessionDelegate>

// the tournament session (model) object
@property (strong) TournamentSession* session;

@property (strong) NSDate* lastSendDate;

@end

@implementation TBRemoteWatchDelegate

- (instancetype)initWithSession:(TournamentSession *)session {
    if((self = [super init])) {
        if([WCSession isSupported]) {
            // Set the tournament session
            [self setSession:session];

            // Default last-send date
            [self setLastSendDate:[NSDate dateWithTimeIntervalSince1970:0.0]];

            // Set up the WatchConnectivity session
            WCSession* wcSession = [WCSession defaultSession];
            [wcSession setDelegate:self];
            [wcSession activateSession];

            // Notification center - Update WCSession when certain state changes happen
            [[NSNotificationCenter defaultCenter] addObserverForName:TournamentSessionUpdatedNotification object:nil queue:nil usingBlock:^(NSNotification* note) {
                if([[self lastSendDate] timeIntervalSinceNow] < -kMinMessageSendInterval) {
                    NSMutableDictionary* update = [NSMutableDictionary dictionaryWithDictionary:[note object]];
                    [update removeObjectsForKeys:@[@"elapsed_time", @"elapsed_time_text", @"time_remaining", @"break_time_remaining", @"action_clock_time_remaining"]];
                    if([update count] > 0) {
                        // Send state change as a message to watch
                        [[WCSession defaultSession] sendMessage:@{@"state":update} replyHandler:nil errorHandler:nil];
                        // Last sent = now
                        [self setLastSendDate:[NSDate date]];
                    }
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

        if([command isEqualToString:@"fullSync"]) {
            // Checking for authorization will trigger auth message
            [[self session] checkAuthorizedWithBlock:nil];
            if([[[self session] state] count] > 0) {
                // Send state change as a message to watch
                [[WCSession defaultSession] sendMessage:@{@"state":[[self session] state]} replyHandler:nil errorHandler:nil];
                // Last sent = now
                [self setLastSendDate:[NSDate date]];
            }
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
