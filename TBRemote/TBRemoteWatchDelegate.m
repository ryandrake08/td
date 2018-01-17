//
//  TBRemoteWatchDelegate.m
//  td
//
//  Created by Ryan Drake on 9/8/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteWatchDelegate.h"
#import "NSObject+FBKVOController.h"
#import <WatchConnectivity/WatchConnectivity.h>

@interface TBRemoteWatchDelegate() <WCSessionDelegate>

// the tournament session (model) object
@property (nonatomic, strong) TournamentSession* session;

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

            // register for KVO
            NSArray* keyPaths = @[
                                  @"session.state.connected",
                                  @"session.state.authorized",
                                  @"session.state.current_blind_level",
                                  @"session.state.clock_text",
                                  @"session.state.current_round_text",
                                  @"session.state.current_game_text",
                                  @"session.state.next_round_text",
                                  @"session.state.next_game_text",
                                  @"session.state.players_left_text",
                                  @"session.state.average_stack_text"
                                  ];
            [[self KVOController] observe:self keyPaths:keyPaths options:NSKeyValueObservingOptionNew block:^(id observer, id object, NSDictionary* change) {
                NSLog(@"Watch observer called: %@ -> %@", change[FBKVONotificationKeyPathKey], change[NSKeyValueChangeNewKey]);

                // get the state key that changed
                NSString* key = [[change[FBKVONotificationKeyPathKey] componentsSeparatedByString:@"."] lastObject];

                // Send state change as a message to watch
                [wcSession sendMessage:@{@"state":@{ key:change[NSKeyValueChangeNewKey] }} replyHandler:nil errorHandler:nil];
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

            // Send full state as a message to watch
            [[WCSession defaultSession] sendMessage:@{@"state":[[self session] state]} replyHandler:nil errorHandler:nil];
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
