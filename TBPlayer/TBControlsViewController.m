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

    // register for KVO
    [[self KVOController] observe:[self session] keyPaths:@[@"isConnected", @"authorized", @"currentBlindLevel"] options:0 block:^(id observer, id object, NSDictionary *change) {
        BOOL connected = [object isConnected];
        BOOL authorized = [object isAuthorized];
        if(connected && authorized) {
            [[observer previousRoundButton] setHidden:NO];
            [[observer pauseResumeButton] setHidden:NO];
            [[observer nextRoundButton] setHidden:NO];
            [[observer callClockButton] setHidden:NO];
        } else {
            [[observer previousRoundButton] setHidden:YES];
            [[observer pauseResumeButton] setHidden:YES];
            [[observer nextRoundButton] setHidden:YES];
            [[observer callClockButton] setHidden:YES];
        }

        NSUInteger currentBlindLevel = [[object currentBlindLevel] unsignedIntegerValue];
        if(currentBlindLevel == 0) {
            [[observer previousRoundButton] setEnabled:NO];
            [[observer pauseResumeButton] setEnabled:YES];
            [[observer nextRoundButton] setEnabled:NO];
            [[observer callClockButton] setEnabled:NO];
        } else {
            [[observer previousRoundButton] setEnabled:YES];
            [[observer pauseResumeButton] setEnabled:YES];
            [[observer nextRoundButton] setEnabled:YES];
            [[observer callClockButton] setEnabled:YES];
        }
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
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGameAt:nil];
    }
}

- (IBAction)nextRoundTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(NSButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        NSUInteger remaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
        if(remaining == 0) {
            [[self session] setActionClock:@kActionClockRequestTime];
        } else {
            [[self session] setActionClock:nil];
        }
    }
}

@end
