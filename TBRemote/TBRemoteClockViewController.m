//
//  TBRemoteClockViewController.m
//  TBRemote
//
//  Created by Ryan Drake on 1/2/15.
//  Copyright (c) 2015 HDna Studio. All rights reserved.
//

#import "TBRemoteClockViewController.h"
#import "TournamentKit/TournamentKit.h"
#import "TBActionClockView.h"
#import "TBAppDelegate.h"

#import "NSObject+FBKVOController.h"

@interface TBRemoteClockViewController () <TBActionClockDelegate>

@property (nonatomic) TournamentSession* session;

@property (nonatomic, weak) IBOutlet UILabel* elapsedLabel;
@property (nonatomic, weak) IBOutlet UILabel* clockLabel;
@property (nonatomic, weak) IBOutlet UILabel* currentRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* nextRoundLabel;
@property (nonatomic, weak) IBOutlet UILabel* playersLeftLabel;
@property (nonatomic, weak) IBOutlet UILabel* averageStackLabel;
@property (nonatomic, weak) IBOutlet UIButton* previousRoundButton;
@property (nonatomic, weak) IBOutlet UIButton* pauseResumeButton;
@property (nonatomic, weak) IBOutlet UIButton* nextRoundButton;
@property (nonatomic, weak) IBOutlet UIButton* callClockButton;
@property (nonatomic, weak) IBOutlet TBActionClockView* actionClockView;

- (IBAction)previousRoundTapped:(UIButton*)sender;
- (IBAction)pauseResumeTapped:(UIButton*)sender;
- (IBAction)nextRoundTapped:(UIButton*)sender;
- (IBAction)callClockTapped:(UIButton*)sender;

@end

@implementation TBRemoteClockViewController

- (void)viewDidLoad {
    [super viewDidLoad];

    // get model
    _session = [(TBAppDelegate*)[[UIApplication sharedApplication] delegate] session];

    // register for KVO
    [[self KVOController] observe:[self session] keyPaths:@[@"isConnected", @"authorized", @"currentBlindLevel"] options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        if([[observer session] isConnected] && [[observer session] isAuthorized]) {
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

        if([[[observer session] currentBlindLevel] unsignedIntegerValue] == 0) {
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

    [[self KVOController] observe:[self session] keyPath:@"clockText" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer clockLabel] setText:[object clockText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"currentRoundText" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer currentRoundLabel] setText:[object currentRoundText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"nextRoundText" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer nextRoundLabel] setText:[object nextRoundText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"playersLeftText" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer playersLeftLabel] setText:[object playersLeftText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"averageStackText" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [[observer averageStackLabel] setText:[object averageStackText]];
    }];

    [[self KVOController] observe:[self session] keyPath:@"actionClockTimeRemaining" options:NSKeyValueObservingOptionInitial block:^(id observer, id object, NSDictionary *change) {
        [observer updateActionClock];
    }];
}

- (void)didReceiveMemoryWarning {
    [super didReceiveMemoryWarning];
    // Dispose of any resources that can be recreated.
}

#pragma mark Update

- (void)updateActionClock {
    NSUInteger actionClockTimeRemaining = [[[self session] actionClockTimeRemaining] unsignedIntegerValue];
    if(actionClockTimeRemaining == 0) {
        [[self actionClockView] setHidden:YES];
    } else {
        [[self actionClockView] setHidden:NO];
        [[self actionClockView] setSeconds:actionClockTimeRemaining / 1000.0];
    }
}

#pragma mark Actions

- (IBAction)previousRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setPreviousLevelWithBlock:nil];
    }
}

- (IBAction)pauseResumeTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] togglePauseGame];
    } else {
        [[self session] startGameAt:nil];
    }
}

- (IBAction)nextRoundTapped:(UIButton*)sender {
    NSUInteger currentBlindLevel = [[[self session] currentBlindLevel] unsignedIntegerValue];
    if(currentBlindLevel != 0) {
        [[self session] setNextLevelWithBlock:nil];
    }
}

- (IBAction)callClockTapped:(UIButton*)sender {
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

# pragma mark TBActionClockViewDelegate

- (CGFloat)analogClock:(TBActionClockView*)clock graduationLengthForIndex:(NSInteger)index {
    return index % 5 == 0 ? 10.0 : 5.0;
}

@end
