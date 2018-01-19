//
//  InterfaceController.m
//  TBWatch Extension
//
//  Created by Ryan Drake on 9/2/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "InterfaceController.h"
#import "NSObject+FBKVOController.h"
#import <WatchConnectivity/WatchConnectivity.h>

@interface InterfaceController () <WCSessionDelegate>

// ui
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* clockLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* currentRoundLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* currentGameLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* nextRoundLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* nextGameLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* playersLeftLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* averageStackLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceButton* previousRoundButton;
@property (weak, nonatomic) IBOutlet WKInterfaceButton* pauseResumeButton;
@property (weak, nonatomic) IBOutlet WKInterfaceButton* nextRoundButton;
@property (weak, nonatomic) IBOutlet WKInterfaceButton* callClockButton;

// tracked state
@property (assign, nonatomic) BOOL connected;
@property (assign, nonatomic) BOOL authorized;
@property (assign, nonatomic) BOOL playing;

@end


@implementation InterfaceController

- (void)awakeWithContext:(id)context {
    [super awakeWithContext:context];

    // WCSession
    if([WCSession isSupported]) {
        WCSession* session = [WCSession defaultSession];
        [session setDelegate:self];
        [session activateSession];
    }
}

- (void)willActivate {
    // This method is called when watch view controller is about to be visible to user
    [super willActivate];

    // Request a full sync from companion
    NSLog(@"Requesting a full sync from companion");
    [[WCSession defaultSession] sendMessage:@{@"command":@"fullSync"} replyHandler:nil errorHandler:nil];
}

- (void)didDeactivate {
    // This method is called when watch view controller is no longer visible
    [super didDeactivate];
}

- (void)session:(WCSession *)session didReceiveMessage:(NSDictionary<NSString *,id> *)message {
    NSDictionary* state = message[@"state"];
    if(state) {
        NSLog(@"State received from companion: %ld entries", (unsigned long)[state count]);

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

        if(state[@"clock_text"]) {
            [[self clockLabel] setText:state[@"clock_text"]];
        }

        if(state[@"current_round_text"]) {
            [[self currentRoundLabel] setText:state[@"current_round_text"]];
        }

        if(state[@"current_game_text"]) {
            [[self currentGameLabel] setText:state[@"current_game_text"]];
        }

        if(state[@"next_round_text"]) {
            [[self nextRoundLabel] setText:state[@"next_round_text"]];
        }

        if(state[@"next_game_text"]) {
            [[self nextGameLabel] setText:state[@"next_game_text"]];
        }

        if(state[@"players_left_text"]) {
            [[self playersLeftLabel] setText:state[@"players_left_text"]];
        }

        if(state[@"average_stack_text"]) {
            [[self averageStackLabel] setText:state[@"average_stack_text"]];
        }
    }
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
