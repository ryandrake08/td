//
//  InterfaceController.m
//  TBWatch Extension
//
//  Created by Ryan Drake on 9/2/15.
//  Copyright © 2015 HDna Studio. All rights reserved.
//

#import "InterfaceController.h"
#import "NSObject+FBKVOController.h"

@import WatchConnectivity;

@interface InterfaceController () <WCSessionDelegate>

// state
@property (nonatomic, strong) NSMutableDictionary* state;

// ui
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* clockLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* currentRoundLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* currentGameLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* nextRoundLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* nextGameLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceButton* previousRoundButton;
@property (weak, nonatomic) IBOutlet WKInterfaceButton* pauseResumeButton;
@property (weak, nonatomic) IBOutlet WKInterfaceButton* nextRoundButton;
@property (weak, nonatomic) IBOutlet WKInterfaceButton* callClockButton;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* playersLeftLabel;
@property (weak, nonatomic) IBOutlet WKInterfaceLabel* averageStackLabel;

@end


@implementation InterfaceController

- (void)awakeWithContext:(id)context {
    [super awakeWithContext:context];

    // Create state
    [self setState:[[NSMutableDictionary alloc] init]];

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
}

- (void)didDeactivate {
    // This method is called when watch view controller is no longer visible
    [super didDeactivate];
}

- (void)session:(WCSession *)session didReceiveMessage:(NSDictionary<NSString *,id> *)message {
    NSDictionary* state = message[@"state"];
    if(state) {
        NSLog(@"State received from companion: %ld entries", (unsigned long)[state count]);

        [[self state] addEntriesFromDictionary:state];

        if(state[@"connected"] || state[@"authorized"]) {
            BOOL authorized = [[self state][@"connected"] boolValue] && [[self state][@"authorized"] boolValue];
            BOOL playing = [[self state][@"current_blind_level"] unsignedIntegerValue] != 0;
            [[self previousRoundButton] setEnabled:authorized && playing];
            [[self pauseResumeButton] setEnabled:authorized];
            [[self nextRoundButton] setEnabled:authorized && playing];
            [[self callClockButton] setEnabled:authorized && playing];
        }

        if(state[@"clock_text"]) {
            [[self clockLabel] setText:[self state][@"clock_text"]];
        }

        if(state[@"current_round_text"]) {
            [[self currentRoundLabel] setText:[self state][@"current_round_text"]];
        }

        if(state[@"current_game_text"]) {
            [[self currentGameLabel] setText:[self state][@"current_game_text"]];
        }

        if(state[@"next_round_text"]) {
            [[self nextRoundLabel] setText:[self state][@"next_round_text"]];
        }

        if(state[@"next_game_text"]) {
            [[self nextGameLabel] setText:[self state][@"next_game_text"]];
        }

        if(state[@"players_left_text"]) {
            [[self playersLeftLabel] setText:[self state][@"players_left_text"]];
        }

        if(state[@"average_stack_text"]) {
            [[self averageStackLabel] setText:[self state][@"average_stack_text"]];
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
