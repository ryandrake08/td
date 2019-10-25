//
//  InterfaceController.m
//  TBWatch Extension
//
//  Created by Ryan Drake on 9/2/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "InterfaceController.h"
#import "NSObject+FBKVOController.h"
#import "TBClockDateComponentsFormatter.h"
#import <WatchConnectivity/WatchConnectivity.h>

@interface InterfaceController () <WCSessionDelegate>

// ui
@property (nonatomic, weak) IBOutlet WKInterfaceLabel* clockLabel;
@property (nonatomic, weak) IBOutlet WKInterfaceLabel* currentRoundLabel;
@property (nonatomic, weak) IBOutlet WKInterfaceLabel* nextRoundLabel;
@property (nonatomic, weak) IBOutlet WKInterfaceLabel* playersLeftLabel;
@property (nonatomic, weak) IBOutlet WKInterfaceLabel* averageStackLabel;
@property (nonatomic, weak) IBOutlet WKInterfaceButton* previousRoundButton;
@property (nonatomic, weak) IBOutlet WKInterfaceButton* pauseResumeButton;
@property (nonatomic, weak) IBOutlet WKInterfaceButton* nextRoundButton;
@property (nonatomic, weak) IBOutlet WKInterfaceButton* callClockButton;

// tracked state
@property (nonatomic, assign) BOOL connected;
@property (nonatomic, assign) BOOL authorized;
@property (nonatomic, assign) BOOL playing;

// store last known times
@property (nonatomic, copy) NSNumber* currentTime;
@property (nonatomic, copy) NSNumber* clockRemaining;
@property (nonatomic, copy) NSNumber* running;

// timer to refresh clock label UI between updates from companion
@property (nonatomic, strong) NSTimer* refreshTimer;

@end


@implementation InterfaceController

- (void)awakeWithContext:(id)context {
    [super awakeWithContext:context];

    // WCSession
    if([WCSession isSupported]) {
        WCSession* session = [WCSession defaultSession];
        [session setDelegate:self];
        [session activateSession];

        // set timer
        [self setRefreshTimer:[NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(updateClockLabel) userInfo:nil repeats:YES]];
    }
}

- (void)willActivate {
    // This method is called when watch view controller is about to be visible to user
    [super willActivate];

    // Request a full sync from companion
    NSLog(@"Requesting a full sync from companion");
    [[WCSession defaultSession] sendMessage:@{@"command":@"fullSync"} replyHandler:nil errorHandler:nil];

    // re-set timer
    [self setRefreshTimer:[NSTimer scheduledTimerWithTimeInterval:0.05 target:self selector:@selector(updateClockLabel) userInfo:nil repeats:YES]];
}

- (void)didDeactivate {
    // This method is called when watch view controller is no longer visible
    [super didDeactivate];

    // remove timer
    [[self refreshTimer] invalidate];
}

- (void)updateClockLabel {
    TBClockDateComponentsFormatter* dateFormatter = [[TBClockDateComponentsFormatter alloc] init];
    if([[self running] boolValue]) {
        // format time for display
        [[self clockLabel] setText:[dateFormatter stringFromMillisecondsRemaining:[self clockRemaining]
                                                          atMillisecondsSince1970:[self currentTime]
                                                                     countingDown:YES]];
    } else {
        [[self clockLabel] setText:NSLocalizedString(@"PAUSED", nil)];
    }
}

- (void)session:(WCSession*)session didReceiveMessage:(NSDictionary<NSString*,id>*)message {
    NSDictionary* state = message[@"state"];
    if(state) {
        if(state[@"connected"] || state[@"authorized"] || state[@"current_blind_level"]) {
            if(state[@"connected"]) {
                [self setConnected:[state[@"connected"] boolValue]];
            }
            if(state[@"authorized"]) {
                [self setAuthorized:[state[@"authorized"] boolValue]];
            }
            if(state[@"current_blind_level"]) {
                [self setPlaying:[state[@"current_blind_level"] unsignedIntegerValue] != 0];
            }

            [[self previousRoundButton] setEnabled:[self authorized] && [self playing]];
            [[self pauseResumeButton] setEnabled:[self authorized]];
            [[self nextRoundButton] setEnabled:[self authorized] && [self playing]];
            [[self callClockButton] setEnabled:[self authorized] && [self playing]];
        }

        if(state[@"running"] != nil) {
            [self setRunning:state[@"running"]];
        }

        if(state[@"current_time"] != nil && state[@"clock_remaining"] != nil) {
            // store
            [self setClockRemaining:state[@"clock_remaining"]];
            [self setCurrentTime:state[@"current_time"]];
            [self updateClockLabel];
        }

        if(state[@"current_round_text"]) {
            [[self currentRoundLabel] setText:state[@"current_round_text"]];
        }

        if(state[@"next_round_text"]) {
            [[self nextRoundLabel] setText:state[@"next_round_text"]];
        }

        if(state[@"players_left_text"]) {
            [[self playersLeftLabel] setText:state[@"players_left_text"]];
        }

        if(state[@"average_stack_text"]) {
            [[self averageStackLabel] setText:state[@"average_stack_text"]];
        }
    }
}

- (void)session:(WCSession*)session activationDidCompleteWithState:(WCSessionActivationState)activationState error:(nullable NSError*)error API_AVAILABLE(watchos(2.2)) {
    // Do nothing but log here for now. Not sure why this is now required to implement
    NSLog(@"Activation completed");
}

#pragma mark Actions

- (IBAction)previousRoundTapped {
    [[WCSession defaultSession] sendMessage:@{@"command":@"previousRound"} replyHandler:nil errorHandler:nil];
}

- (IBAction)playPauseTapped {
    [[WCSession defaultSession] sendMessage:@{@"command":@"playPause"} replyHandler:nil errorHandler:nil];
}

- (IBAction)nextRoundTapped {
    [[WCSession defaultSession] sendMessage:@{@"command":@"nextRound"} replyHandler:nil errorHandler:nil];
}

- (IBAction)callClockTapped {
    [[WCSession defaultSession] sendMessage:@{@"command":@"callClock"} replyHandler:nil errorHandler:nil];
}

@end
