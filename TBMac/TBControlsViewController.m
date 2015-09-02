//
//  TBControlsViewController.m
//  td
//
//  Created by Ryan Drake on 8/9/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBControlsViewController.h"
#import "NSObject+FBKVOController.h"

@interface TBControlsViewController ()

@property (weak) IBOutlet NSButton* previousRoundButton;
@property (weak) IBOutlet NSButton* pauseResumeButton;
@property (weak) IBOutlet NSButton* nextRoundButton;
@property (weak) IBOutlet NSButton* callClockButton;

@end

@implementation TBControlsViewController

- (void)viewDidLoad {
    if([NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [super viewDidLoad];
    }

    // TODO: Go back to logic that takes into account whether the game has started

    // register for KVO
    [[self KVOController] observe:[self session] keyPaths:@[@"isConnected", @"isAuthorized"] options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        BOOL enable = [object isConnected] && [object isAuthorized];
        [[observer previousRoundButton] setEnabled:enable];
        [[observer pauseResumeButton] setEnabled:enable];
        [[observer nextRoundButton] setEnabled:enable];
        [[observer callClockButton] setEnabled:enable];
    }];
}

- (void)loadView {
    [super loadView];
    if(![NSViewController instancesRespondToSelector:@selector(viewDidLoad)]) {
        [self viewDidLoad];
    }
}

#pragma mark Actions

- (IBAction)previousRoundTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGameAt:nil];
    }
}

- (IBAction)nextRoundTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] state][@"current_blind_level"] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(NSButton*)sender {
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

@end
