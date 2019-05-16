//
//  TBRemoteWatchDelegate.m
//  td
//
//  Created by Ryan Drake on 9/8/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteWatchDelegate.h"
#import "TournamentSession.h"
#import "NSObject+FBKVOController.h"
#import <WatchConnectivity/WatchConnectivity.h>

@interface TBRemoteWatchDelegate() <WCSessionDelegate>

// the tournament session (model) object
@property (nonatomic, strong) TournamentSession* session;

// cache the state and send periodically
@property (nonatomic, strong) NSMutableDictionary* cachedState;

// timer
@property (nonatomic, strong) NSTimer* sendMessageTimer;

@end

@implementation TBRemoteWatchDelegate

- (instancetype)initWithSession:(TournamentSession *)session API_AVAILABLE(ios(9.0)) {
    if((self = [super init])) {
        if([WCSession isSupported]) {
            // set the tournament session
            _session = session;

            // set up cache
            _cachedState = [NSMutableDictionary dictionary];

            // set up the WatchConnectivity session
            WCSession* wcSession = [WCSession defaultSession];
            [wcSession setDelegate:self];
            [wcSession activateSession];

            // register for KVO
            NSArray* keyPaths = @[
                                  @"session.connected",
                                  @"session.authorized",
                                  @"session.state.running",
                                  @"session.state.current_blind_level",
                                  @"session.state.current_time",
                                  @"session.state.clock_remaining",
                                  @"session.state.current_round_blinds_text",
                                  @"session.state.current_round_ante_text",
                                  @"session.state.next_round_text",
                                  @"session.state.players_left_text",
                                  @"session.state.average_stack_text"
                                  ];
            [[self KVOController] observe:self keyPaths:keyPaths options:NSKeyValueObservingOptionNew block:^(id observer, id object, NSDictionary* change) {
                // get the state key that changed
                NSString* key = [[change[FBKVONotificationKeyPathKey] componentsSeparatedByString:@"."] lastObject];

                if([change[NSKeyValueChangeNewKey] isEqual:[NSNull null]]) {
                    // remove from cache if null
                    [[self cachedState] removeObjectForKey:key];
                } else {
                    // cache it for potential deferred send
                    [self cachedState][key] = change[NSKeyValueChangeNewKey];
                }
            }];

            // start a timer to send stored state (on iOS 9.3+, timer will be started in -activationDidCompleteWithState:error:
            if(@available(iOS 9.3, *)) {
            } else {
                _sendMessageTimer = [NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(sendCachedStateFromTimer:) userInfo:nil repeats:YES];
            }
        }
    }
    return self;
}

- (void)sendCachedStateFromTimer:(NSTimer*)timer API_AVAILABLE(ios(9.0)) {
    WCSession* wcSession = [WCSession defaultSession];
    if([wcSession isReachable] && [[self cachedState] count] > 0) {
        // send
        [wcSession sendMessage:@{@"state":[[self cachedState] copy]} replyHandler:nil errorHandler:nil];

        // clear cache
        [[self cachedState] removeAllObjects];
    }
}

- (void)handleCommand:(NSString*)command API_AVAILABLE(ios(9.0)) {
    if(command) {
        NSLog(@"Command received from watch: %@", command);

        if([command isEqualToString:@"fullSync"]) {
            // Checking for authorization will trigger auth message
            [[self session] checkAuthorizedWithBlock:nil];

            if([[WCSession defaultSession] isReachable]) {
                // Send full state as a message to watch
                [[WCSession defaultSession] sendMessage:@{@"state":[[self session] state]} replyHandler:nil errorHandler:nil];
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

- (void)session:(WCSession*)wcSession didReceiveMessage:(NSDictionary<NSString*,id>*)message API_AVAILABLE(ios(9.0)) {
    [self performSelectorOnMainThread:@selector(handleCommand:) withObject:message[@"command"] waitUntilDone:NO];
}

- (void)session:(nonnull WCSession*)session activationDidCompleteWithState:(WCSessionActivationState)activationState error:(nullable NSError*)error API_AVAILABLE(ios(9.3)) {
    // start a timer to send stored state
    if(activationState == WCSessionActivationStateActivated) {
        [self setSendMessageTimer:[NSTimer scheduledTimerWithTimeInterval:1.0 target:self selector:@selector(sendCachedStateFromTimer:) userInfo:nil repeats:YES]];
    }
}


- (void)sessionDidBecomeInactive:(nonnull WCSession*)session API_AVAILABLE(ios(9.3)) {
    // cancel the timer
    [[self sendMessageTimer] invalidate];
    [self setSendMessageTimer:nil];
}


- (void)sessionDidDeactivate:(nonnull WCSession*)session API_AVAILABLE(ios(9.3)) {
    // set up the WatchConnectivity session
    WCSession* wcSession = [WCSession defaultSession];
    [wcSession setDelegate:self];
    [wcSession activateSession];
}

@end
