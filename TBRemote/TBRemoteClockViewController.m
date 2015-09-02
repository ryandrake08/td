//
//  TBRemoteClockViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteClockViewController.h"
#import "TournamentSession.h"
#import "TBActionClockView.h"
#import "TBAppDelegate.h"

#import "NSObject+FBKVOController.h"

@interface TBRemoteClockViewController () <TBActionClockDelegate>

@property (nonatomic, strong) TournamentSession* session;

@property (nonatomic, weak) IBOutlet UILabel* elapsedLabel;
@property (nonatomic, weak) IBOutlet UILabel* clockLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentGameLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* nextGameLabel;
@property (nonatomic, weak) IBOutlet UILabel* nextRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* playersLeftLabel;
@property (nonatomic, weak) IBOutlet UILabel* averageStackLabel;
@property (nonatomic, weak) IBOutlet UIButton* previousRoundButton;
@property (nonatomic, weak) IBOutlet UIButton* pauseResumeButton;
@property (nonatomic, weak) IBOutlet UIButton* nextRoundButton;
@property (nonatomic, weak) IBOutlet UIButton* callClockButton;
@property (nonatomic, weak) IBOutlet TBActionClockView* actionClockView;
@property (nonatomic, weak) IBOutlet UIScrollView* scrollView;

@end

@implementation TBRemoteClockViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // set scroll view content inset
    [[self scrollView] setContentInset:UIEdgeInsetsMake(0.0,0.0,49.0,0.0)];

    // register for KVO
    [[self KVOController] observe:[[self session] state] keyPaths:@[@"connected", @"authorized", @"current_blind_level"] options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        BOOL authorized = [object[@"connected"] boolValue] && [object[@"authorized"] boolValue];
        BOOL playing = [object[@"current_blind_level"] unsignedIntegerValue] != 0;
        [[observer previousRoundButton] setEnabled:authorized && playing];
        [[observer pauseResumeButton] setEnabled:authorized];
        [[observer nextRoundButton] setEnabled:authorized && playing];
        [[observer callClockButton] setEnabled:authorized && playing];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"elapsed_time_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer elapsedLabel] setText:object[@"elapsed_time_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"clock_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer clockLabel] setText:object[@"clock_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"current_game_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer currentGameLabel] setText:object[@"current_game_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"current_round_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer currentRoundLabel] setText:object[@"current_round_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"next_game_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer nextGameLabel] setText:object[@"next_game_text"]];
    }];
    
    [[self KVOController] observe:[[self session] state] keyPath:@"next_round_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer nextRoundLabel] setText:object[@"next_round_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"players_left_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer playersLeftLabel] setText:object[@"players_left_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"average_stack_text" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer averageStackLabel] setText:object[@"average_stack_text"]];
    }];

    [[self KVOController] observe:[[self session] state] keyPath:@"action_clock_time_remaining" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [observer updateActionClock:object[@"action_clock_time_remaining"]];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark Update

- (void)updateActionClock:(NSNumber*)timeRemaining {
    NSUInteger actionClockTimeRemaining = [timeRemaining unsignedIntegerValue];
    if(actionClockTimeRemaining == 0) {
        [[self actionClockView] setHidden:YES];
    } else {
        [[self actionClockView] setHidden:NO];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }
}

#pragma mark Actions

- (IBAction)previousRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGameAt:nil];
    }
}

- (IBAction)nextRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        NSUInteger remaining = [[[self session] state][@"action_clock_remaining"] unsignedIntegerValue];
        if(remaining == 0) {
            [[self session] setActionClock:@kActionClockRequestTime];
        } else {
            [[self session] setActionClock:nil];
        }
    }
}

# pragma mark TBActionClockViewDelegate

- (CGFloat)analogClock:(TBActionClockView*)clock graduationLengthForIndex:(NSInteger)index {
    return index % 5 == 0 ? 10.0 : 5.0;
}

@end
