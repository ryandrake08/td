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
    [[self KVOController] observe:[self session] keyPaths:@[@"connected", @"authorized", @"currentBlindLevel"] options:0 block:^(id observer, id object, NSDictionary *change) {
        if([[observer session] isConnected] && [[observer session] isAuthorized]) {
            if([[[observer session] currentBlindLevel] unsignedIntegerValue] == 0) {
                [[observer previousRoundButton] setEnabled:NO];
                [[observer pauseResumeButton] setEnabled:NO];
                [[observer nextRoundButton] setEnabled:NO];
                [[observer callClockButton] setEnabled:YES];
            } else {
                [[observer previousRoundButton] setEnabled:YES];
                [[observer pauseResumeButton] setEnabled:YES];
                [[observer nextRoundButton] setEnabled:YES];
                [[observer callClockButton] setEnabled:YES];
            }
        } else {
            [[observer previousRoundButton] setEnabled:NO];
            [[observer pauseResumeButton] setEnabled:NO];
            [[observer nextRoundButton] setEnabled:NO];
            [[observer callClockButton] setEnabled:NO];
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
